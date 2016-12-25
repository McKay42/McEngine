//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		core
//
// $NoKeywords: $engine $os
//===============================================================================//

#include "NetworkHandler.h" // must be up here
#include "Engine.h"

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#include "WinGLLegacyInterface.h"
#include "WinSWGraphicsInterface.h"
#include "WinContextMenu.h"
#include "WinEnvironment.h"

#elif defined __linux__

#include "stdarg.h"
#include "LinuxGLLegacyInterface.h"
#include "LinuxContextMenu.h"
#include "LinuxEnvironment.h"

#endif

#include "VulkanGraphicsInterface.h"

#include "OpenCLInterface.h"
#include "OpenVRInterface.h"
#include "VulkanInterface.h"
#include "SoundEngine.h"
#include "ResourceManager.h"
#include "AnimationHandler.h"
#include "XInputGamepad.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "Timer.h"
#include "ConVar.h"

#include "CBaseUIContainer.h"
#include "Console.h"
#include "ConsoleBox.h"



//********************//
//	Include App here  //
//********************//

//#include "Osu.h"
#include "FrameworkTest.h"

#include <cstdarg>



void _version(void);
void __host_timescale( UString oldValue, UString newValue );
ConVar host_timescale("host_timescale", 1.0f, "Scale by which the engine measures elapsed time, affects engine->getTime()", __host_timescale);
void __host_timescale( UString oldValue, UString newValue )
{
	if (newValue.toFloat() < 0.01f)
	{
		debugLog(0xffff4444, UString::format("Value must be >= 0.01!\n").toUtf8());
		host_timescale.setValue(1.0f);
	}
}
ConVar epilepsy("epilepsy", false);
ConVar debug_engine("debug_engine", false);



Engine *engine = NULL;
Environment *env = NULL;

Console *Engine::m_console = NULL;
ConsoleBox *Engine::m_consoleBox = NULL;



Engine::Engine(Environment *environment)
{
	engine = this;
	m_environment = environment;
	env = environment;
	m_guiContainer = NULL;
	m_app = NULL;

	// disable output buffering (else we get multithreading issues due to blocking)
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	debugLog("-= Engine Startup =-\n");
	_version();

	// check if we are able to start

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

	if (!checkGLHardwareAcceleration())
	{
		showMessageErrorFatal("Fatal Engine Error", "No OpenGL hardware acceleration available!\nThe engine will quit now.");
		exit(0);
	}

#elif defined __linux__
	// TODO: implement checkGLHardwareAcceleration() on linux
#endif

	// timing
	m_timer = new Timer();
	m_dTime = 0;
	m_dRunTime = 0;
	m_iFrameCount = 0;
	m_dFrameTime = 0.016f;

	// window
	m_bBlackout = false;
	m_bHasFocus = false;
	m_bIsMinimized = false;

	// screen
	m_bResolutionChange = false;
	m_vScreenSize = m_environment->getWindowSize();
	m_vNewScreenSize = m_vScreenSize;

	debugLog("Engine: ScreenSize = (%ix%i)\n", (int)m_vScreenSize.x, (int)m_vScreenSize.y);

	// engine
	m_bDrawing = false;

	// initialize all engine subsystems (the order does matter!)

	// input devices
	m_mouse = new Mouse();
	m_inputDevices.push_back(m_mouse);
	m_mice.push_back(m_mouse);

	m_keyboard = new Keyboard();
	m_inputDevices.push_back(m_keyboard);
	m_keyboards.push_back(m_keyboard);

	m_gamepad = new XInputGamepad();
	m_inputDevices.push_back(m_gamepad);
	m_gamepads.push_back(m_gamepad);

	m_vulkan = NULL;
	///m_vulkan = new VulkanInterface();

	// init platform specific interfaces

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

	m_graphics = new WinGLLegacyInterface(((WinEnvironment*)env)->getHwnd());
	///m_graphics = new WinSWGraphicsInterface(((WinEnvironment*)env)->getHwnd());
	///m_graphics = new VulkanGraphicsInterface();
	m_contextMenu = new WinContextMenu();

#elif defined __linux__

	m_graphics = new LinuxGLLegacyInterface(((LinuxEnvironment*)m_environment)->getDisplay(), ((LinuxEnvironment*)m_environment)->getWindow());
	m_contextMenu = new LinuxContextMenu();

#endif

	// and the rest
	m_resourceManager = new ResourceManager();
	m_sound = new SoundEngine();
	m_animationHandler = new AnimationHandler();
	m_openCL = new OpenCLInterface();
	m_openVR = new OpenVRInterface();
	m_networkHandler = new NetworkHandler();

	// default launch options/changes
	m_graphics->setVSync(false);

	// engine time starts now
	m_timer->start();

	debugLog("\n");
}

Engine::~Engine()
{
	debugLog("\n-= Engine Shutdown =-\n");

	debugLog("Engine: Freeing app...\n");
	SAFE_DELETE(m_app);

	if (m_console != NULL)
		showMessageErrorFatal("Engine Error", "m_console not set to NULL before shutdown!");

	debugLog("Engine: Freeing console box...\n");
	SAFE_DELETE(m_consoleBox);

	debugLog("Engine: Freeing resource manager...\n");
	SAFE_DELETE(m_resourceManager);

	debugLog("Engine: Freeing OpenCL...\n");
	SAFE_DELETE(m_openCL);

	debugLog("Engine: Freeing OpenVR...\n");
	SAFE_DELETE(m_openVR);

	debugLog("Engine: Freeing Vulkan...\n");
	SAFE_DELETE(m_vulkan);

	debugLog("Engine: Freeing Sound...\n");
	SAFE_DELETE(m_sound);

	debugLog("Engine: Freeing context menu...\n");
	SAFE_DELETE(m_contextMenu);

	debugLog("Engine: Freeing animation handler...\n");
	SAFE_DELETE(m_animationHandler);

	debugLog("Engine: Freeing network handler...\n");
	SAFE_DELETE(m_networkHandler);

	debugLog("Engine: Freeing input devices...\n");
	for (int i=0; i<m_inputDevices.size(); i++)
	{
		delete m_inputDevices[i];
	}
	m_inputDevices.clear();

	debugLog("Engine: Freeing timer...\n");
	SAFE_DELETE(m_timer);

	debugLog("Engine: Freeing graphics...\n");
	SAFE_DELETE(m_graphics);

	debugLog("Engine: Freeing environment...\n");
	SAFE_DELETE(m_environment);

	debugLog("Engine: Goodbye.");
	engine = NULL;
}

void Engine::loadApp()
{
	debugLog("Engine: Loading default resources ...\n");

	// load default resources
	engine->getResourceManager()->loadFont("weblysleekuisb.ttf", "FONT_DEFAULT", 15.0f);
	engine->getResourceManager()->loadFont("tahoma.ttf", "FONT_CONSOLE", 8.0f, false);
	Image *missingTexture = engine->getResourceManager()->createImage(512, 512);
	missingTexture->setName("MISSING_TEXTURE");
	for (int x=0; x<512; x++)
	{
		for (int y=0; y<512; y++)
		{
			int rowCounter = (x / 64);
			int columnCounter = (y / 64);
			Color color = (((rowCounter+columnCounter) % 2 == 0) ? COLOR(255, 255, 0, 221) : COLOR(255, 0, 0, 0));
			missingTexture->setPixel(x, y, color);
		}
	}
	missingTexture->load();

	// create engine gui
	m_guiContainer = new CBaseUIContainer(0, 0, engine->getScreenWidth(), engine->getScreenHeight(), "engine");
	m_consoleBox = new ConsoleBox();
	m_guiContainer->addBaseUIElement(m_consoleBox);

	debugLog("\nEngine: Loading app ...\n");

	//*****************//
	//	Load App here  //
	//*****************//

	//m_app = new Osu();

	m_app = new FrameworkTest();




	// start listening to the default keyboard input (engine gui comes first)
	m_keyboard->addListener(m_guiContainer, true);
	m_keyboard->addListener(m_app);
}

void Engine::onPaint()
{
	if (m_bBlackout || m_bIsMinimized)
		return;

	m_bDrawing = true;

	m_graphics->beginScene();

		if (m_app != NULL)
			m_app->draw(m_graphics);

		if (m_guiContainer != NULL)
			m_guiContainer->draw(m_graphics);

		// debug input devices
		for (int i=0; i<m_inputDevices.size(); i++)
		{
			m_inputDevices[i]->draw(m_graphics);
		}

		if (epilepsy.getBool())
		{
			m_graphics->setColor(COLOR(255, rand()%256, rand()%256, rand()%256));
			m_graphics->fillRect(0, 0, engine->getScreenWidth(), engine->getScreenHeight());
		}

	m_graphics->endScene();

	m_bDrawing = false;

	m_iFrameCount++;
}

void Engine::onUpdate()
{
	if (m_bBlackout || m_bIsMinimized)
		return;

	// update time
	m_timer->update();
	m_dRunTime = m_timer->getElapsedTime();
	m_dFrameTime *= (double)host_timescale.getFloat();
	m_dTime += m_dFrameTime;

	// handle resolution changes
	if (m_bResolutionChange)
	{
		m_bResolutionChange = false;
		onResolutionChange(m_vNewScreenSize);
	}

	// update input devices
	for (int i=0; i<m_inputDevices.size(); i++)
	{
		m_inputDevices[i]->update();
	}

	// update openvr (this also includes input devices)
	m_openVR->update();

	// update animations
	anim->update();

	// update resource manager
	m_resourceManager->update();

	// update gui
	if (m_guiContainer != NULL)
		m_guiContainer->update();

	// execute queued commands
	// TODO: this is shit
	if (Console::g_commandQueue.size() > 0)
	{
		for (int i=0; i<Console::g_commandQueue.size(); i++)
		{
			Console::processCommand(Console::g_commandQueue[i]);
		}
		Console::g_commandQueue = std::vector<UString>(); // reset
	}

	// update networking
	m_networkHandler->update();

	// update app
	if (m_app != NULL)
		m_app->update();

	// update environment
	m_environment->update();
}

void Engine::onFocusGained()
{
	m_bHasFocus = true;

	if (debug_engine.getBool())
		debugLog("Engine: got focus\n");

	if (m_app != NULL)
		m_app->onFocusGained();
}

void Engine::onFocusLost()
{
	m_bHasFocus = false;

	if (debug_engine.getBool())
		debugLog("Engine: lost focus\n");

	for (int i=0; i<m_keyboards.size(); i++)
	{
		m_keyboards[i]->reset();
	}

	if (m_app != NULL)
		m_app->onFocusLost();
}

void Engine::onMinimized()
{
	m_bIsMinimized = true;
	m_bHasFocus = false;

	if (debug_engine.getBool())
		debugLog("Engine: window minimized\n");

	if (m_app != NULL)
		m_app->onMinimized();
}

void Engine::onMaximized()
{
	m_bIsMinimized = false;

	if (debug_engine.getBool())
		debugLog("Engine: window maximized\n");
}

void Engine::onRestored()
{
	m_bIsMinimized = false;

	if (debug_engine.getBool())
		debugLog("Engine: window restored\n");

	if (m_app != NULL)
		m_app->onRestored();
}

void Engine::onResolutionChange(Vector2 newResolution)
{
	debugLog(0xff00ff00,"Engine: onResolutionChange() (%i, %i) -> (%i, %i)\n", (int)m_vScreenSize.x, (int)m_vScreenSize.y, (int)newResolution.x, (int)newResolution.y);

	// NOTE: Windows [Show Desktop] button in the superbar causes (0,0)
	if (newResolution.x < 2 || newResolution.y < 2)
	{
		m_bIsMinimized = true;
		newResolution = Vector2(2,2);
	}

	// to avoid double resolutionChange
	m_bResolutionChange = false;
	m_vNewScreenSize = newResolution;

	if (m_guiContainer != NULL)
		m_guiContainer->setSize(newResolution.x, newResolution.y);
	if (m_consoleBox != NULL)
		m_consoleBox->onResolutionChange(newResolution);

	// update everything
	m_vScreenSize = newResolution;
	if (m_graphics != NULL)
		m_graphics->onResolutionChange(newResolution);
	if (m_openVR != NULL)
		m_openVR->onResolutionChange(newResolution);
	if (m_app != NULL)
		m_app->onResolutionChanged(newResolution);
}

void Engine::onMouseRawMove(int xDelta, int yDelta, bool absolute, bool virtualDesktop)
{
	m_mouse->onRawMove(xDelta, yDelta, absolute, virtualDesktop);
}

void Engine::onMouseWheelVertical(int delta)
{
	m_mouse->onWheelVertical(delta);
}

void Engine::onMouseWheelHorizontal(int delta)
{
	m_mouse->onWheelHorizontal(delta);
}

void Engine::onMouseLeftChange(bool mouseLeftDown)
{
	m_mouse->onLeftChange(mouseLeftDown);
}

void Engine::onMouseMiddleChange(bool mouseMiddleDown)
{
	m_mouse->onMiddleChange(mouseMiddleDown);
}

void Engine::onMouseRightChange(bool mouseRightDown)
{
	m_mouse->onRightChange(mouseRightDown);
}

void Engine::onKeyboardKeyDown(KEYCODE keyCode)
{
	if (m_keyboard->isAltDown() && keyCode == KEY_F4)
		shutdown();

	m_keyboard->onKeyDown(keyCode);
}

void Engine::onKeyboardKeyUp(KEYCODE keyCode)
{
	m_keyboard->onKeyUp(keyCode);
}

void Engine::onKeyboardChar(KEYCODE charCode)
{
	m_keyboard->onChar(charCode);
}

void Engine::shutdown()
{
	if (m_app != NULL && !m_app->onShutdown())
		return;

	m_bBlackout = true;
	m_environment->shutdown();
}

void Engine::focus()
{
	m_environment->focus();
}

void Engine::center()
{
	m_environment->center();
}

void Engine::toggleFullscreen()
{
	if (m_environment->isFullscreen())
		m_environment->disableFullscreen();
	else
		m_environment->enableFullscreen();
}

void Engine::disableFullscreen()
{
	m_environment->disableFullscreen();
}

void Engine::showMessageInfo(UString title, UString message)
{
	debugLog("INFO: [%s] | %s\n", title.toUtf8(), message.toUtf8());
	m_environment->showMessageInfo(title, message);
}

void Engine::showMessageWarning(UString title, UString message)
{
	debugLog("WARNING: [%s] | %s\n", title.toUtf8(), message.toUtf8());
	m_environment->showMessageWarning(title, message);
}

void Engine::showMessageError(UString title, UString message)
{
	debugLog("ERROR: [%s] | %s\n", title.toUtf8(), message.toUtf8());
	m_environment->showMessageError(title, message);
}

void Engine::showMessageErrorFatal(UString title, UString message)
{
	debugLog("FATAL ERROR: [%s] | %s\n", title.toUtf8(), message.toUtf8());
	m_environment->showMessageErrorFatal(title, message);
}

void Engine::addGamepad(Gamepad *gamepad)
{
	if (gamepad == NULL)
	{
		showMessageError("Engine Error", "addGamepad(NULL)!");
		return;
	}

	m_gamepads.push_back(gamepad);
}

void Engine::removeGamepad(Gamepad *gamepad)
{
	if (gamepad == NULL)
	{
		showMessageError("Engine Error", "removeGamepad(NULL)!");
		return;
	}

	for (int i=0; i<m_gamepads.size(); i++)
	{
		if (m_gamepads[i] == gamepad)
		{
			m_gamepads.erase(m_gamepads.begin()+i);
			break;
		}
	}
}

double const Engine::getTimeReal()
{
	m_timer->update();
	return m_timer->getElapsedTime();
}

void Engine::debugLog(const char *fmt, ...)
{
	if (fmt == NULL)
		return;

	va_list ap,ap2;
	va_start(ap, fmt);
	va_copy(ap2, ap);

	// write to console
	int numChars = vprintf(fmt, ap);

	if (numChars < 1 || numChars > 65534)
		goto cleanup;

	{
	// write to engine console
	// numChars = vsnprintf(NULL, 0, fmt, ap2);
	char *buffer = new char[numChars+1];
	buffer[numChars-1] = '\0';
	vsnprintf(buffer, numChars+1, fmt, ap2);

	UString actualBuffer = UString(buffer);
	delete[] buffer;

	if (m_consoleBox != NULL)
		m_consoleBox->log(actualBuffer);
	if (m_console != NULL)
		m_console->log(actualBuffer);
	}

cleanup:
	va_end(ap2);
	va_end(ap);
}

void Engine::debugLog(Color color, const char *fmt, ...)
{
	if (fmt == NULL)
		return;

	va_list ap,ap2;
	va_start(ap, fmt);
	va_copy(ap2, ap);

	// write to console
	int numChars = vprintf(fmt, ap);

	if (numChars < 1 || numChars > 65534)
		goto cleanup;

	{
	// write to engine console
	// numChars = vsnprintf(NULL, 0, fmt, ap2);
	char *buffer = new char[numChars+1];
	buffer[numChars-1] = '\0';
	vsnprintf(buffer, numChars+1, fmt, ap2);

	UString actualBuffer = UString(buffer);
	delete[] buffer;

	if (m_consoleBox != NULL)
		m_consoleBox->log(actualBuffer);
	if (m_console != NULL)
		m_console->log(actualBuffer, color);
	}

cleanup:
	va_end(ap2);
	va_end(ap);
}



//**********************//
//	Engine ConCommands	//
//**********************//

void _exit(void)
{
	engine->shutdown();
}

void _printsize(void)
{
	Vector2 s = engine->getScreenSize();
	debugLog("Engine: screenSize = (%f, %f)\n", s.x, s.y);
}

void _fullscreen(void)
{
	engine->toggleFullscreen();
}

void _windowed(UString args)
{
	env->disableFullscreen();

	if (args.length() < 7)
		return;

	std::vector<UString> resolution = args.split("x");
	if (resolution.size() != 2)
		debugLog("Error: Invalid parameter count for command 'windowed'! (Usage: e.g. \"windowed 1280x720\")");
	else
	{
		int width = resolution[0].toFloat();
		int height = resolution[1].toFloat();

		if (width < 300 || height < 240)
			debugLog("Error: Invalid values for resolution for command 'windowed'!");
		else
		{
			env->setWindowSize(width, height);
			env->center();
		}
	}
}

void _minimize(void)
{
	env->minimize();
}

void _focus(void)
{
	engine->focus();
}

void _center(void)
{
	engine->center();
}

void _version(void)
{
	debugLog("McEngine v4 - Build Date: %s, %s\n", __DATE__, __TIME__);
}

void _debugCorporeal(UString oldValue, UString args)
{
	bool corporeal = !(args.toFloat() > 0.0f);
	debugLog("setting it to %i\n", (int)corporeal);
	env->setWindowGhostCorporeal(corporeal);
}

void _errortest(void)
{
	engine->showMessageError("Error Test", "This is an error message, fullscreen mode should be disabled and you should be able to read this");
}

void _crash(void)
{
	ConVar *nullPointer = NULL;
	nullPointer->setValue(false);
}

ConVar __exit("exit", _exit);
ConVar __printsize("printsize", _printsize);
ConVar __fullscreen("fullscreen", _fullscreen);
ConVar __windowed("windowed", _windowed);
ConVar __minimize("minimize", _minimize);
ConVar __focus("focus", _focus);
ConVar __center("center", _center);
ConVar __version("version", _version);
ConVar __corporeal("debug_ghost", false, _debugCorporeal);
ConVar __errortest("errortest", _errortest);
ConVar __crash("crash", _crash);
