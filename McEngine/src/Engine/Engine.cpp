//================ Copyright (c) 2012, PG, All rights reserved. =================//
//
// Purpose:		core
//
// $NoKeywords: $engine
//===============================================================================//

#include "Engine.h"

#include <stdio.h>

#ifdef MCENGINE_FEATURE_MULTITHREADING

#include <mutex>
#include "WinMinGW.Mutex.h"

#endif

#include "SteamworksInterface.h"
#include "AnimationHandler.h"
#include "DiscordInterface.h"
#include "OpenCLInterface.h"
#include "OpenVRInterface.h"
#include "VulkanInterface.h"
#include "ResourceManager.h"
#include "NetworkHandler.h"
#include "XInputGamepad.h"
#include "SoundEngine.h"
#include "ContextMenu.h"
#include "Keyboard.h"
#include "Profiler.h"
#include "ConVar.h"
#include "Mouse.h"
#include "Timer.h"

#include "CBaseUIContainer.h"

#include "Console.h"
#include "ConsoleBox.h"
#include "VisualProfiler.h"



//********************//
//	Include App here  //
//********************//

//#include "Osu.h"
//#include "GUICoherenceMode.h"
//#include "Asteroids.h"
#include "FrameworkTest.h"



class EngineLoadingScreenApp : public App
{
public:
	virtual ~EngineLoadingScreenApp() {;}

	virtual void draw(Graphics *g) override
	{
		McFont *consoleFont = engine->getResourceManager()->getFont("FONT_CONSOLE");
		if (consoleFont != NULL)
		{
			UString loadingText = "Loading ...";

			const float dpiScale = std::round(env->getDPIScale() + 0.255f);

			const float stringHeight = consoleFont->getHeight() * dpiScale;
			const float stringWidth = consoleFont->getStringWidth(loadingText) * dpiScale;

			const float margin = 5 * dpiScale;

			g->pushTransform();
			{
				g->scale(dpiScale, dpiScale);
				g->translate((int)(engine->getScreenWidth()/2 - stringWidth/2), (int)(engine->getScreenHeight()/2 + stringHeight/2));
				g->setColor(0xffffffff);
				g->drawString(consoleFont, loadingText);
			}
			g->popTransform();

			g->setColor(0xffffffff);
			g->drawRect(engine->getScreenWidth()/2 - stringWidth/2 - margin, engine->getScreenHeight()/2 - stringHeight/2 - margin, stringWidth + margin*2, stringHeight + margin*2);
		}
	}
};



void _version(void);
void _host_timescale_( UString oldValue, UString newValue );
ConVar host_timescale("host_timescale", 1.0f, "Scale by which the engine measures elapsed time, affects engine->getTime()", _host_timescale_);
void _host_timescale_( UString oldValue, UString newValue )
{
	if (newValue.toFloat() < 0.01f)
	{
		debugLog(0xffff4444, UString::format("Value must be >= 0.01!\n").toUtf8());
		host_timescale.setValue(1.0f);
	}
}
ConVar epilepsy("epilepsy", false);
ConVar debug_engine("debug_engine", false);
ConVar minimize_on_focus_lost_if_fullscreen("minimize_on_focus_lost_if_fullscreen", true);
ConVar minimize_on_focus_lost_if_borderless_windowed_fullscreen("minimize_on_focus_lost_if_borderless_windowed_fullscreen", false);
ConVar _win_realtimestylus("win_realtimestylus", false, "if compiled on Windows, enables native RealTimeStylus support for tablet clicks");
ConVar _win_processpriority("win_processpriority", 0, "if compiled on Windows, sets the main process priority (0 = normal, 1 = high)");
ConVar _win_disable_windows_key("win_disable_windows_key", false, "if compiled on Windows, set to 0/1 to disable/enable all windows keys via low level keyboard hook");

ConVar *win_realtimestylus = &_win_realtimestylus; // extern

Engine *engine = NULL;
Environment *env = NULL;

Console *Engine::m_console = NULL;
ConsoleBox *Engine::m_consoleBox = NULL;

Engine::Engine(Environment *environment, const char *args)
{
	engine = this;
	m_environment = environment;
	env = environment;
	m_sArgs = UString(args);

	m_graphics = NULL;
	m_guiContainer = NULL;
	m_visualProfiler = NULL;
	m_app = NULL;

	// disable output buffering (else we get multithreading issues due to blocking)
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	// print debug information
	debugLog("-= Engine Startup =-\n");
	_version();
	debugLog("Engine: args = %s\n", m_sArgs.toUtf8());

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

	// custom
	m_bDrawing = false;
	m_iLoadingScreenDelay = 0; // 0 == enabled, -2 == disabled (-1 is reserved)

	// initialize all engine subsystems (the order does matter!)
	debugLog("\nEngine: Initializing subsystems ...\n");
	{
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

		// init platform specific interfaces
		m_vulkan = new VulkanInterface(); // needs to be created before Graphics
		m_graphics = m_environment->createRenderer();
		{
			m_graphics->init(); // needs init() separation due to potential engine->getGraphics() access
		}
		m_contextMenu = m_environment->createContextMenu();

		// and the rest
		m_resourceManager = new ResourceManager();
		m_sound = new SoundEngine();
		m_animationHandler = new AnimationHandler();
		m_openCL = new OpenCLInterface();
		m_openVR = new OpenVRInterface();
		m_networkHandler = new NetworkHandler();
		m_steam = new SteamworksInterface();
		m_discord = new DiscordInterface();

		// default launch overrides
		m_graphics->setVSync(false);

		// engine time starts now
		m_timer->start();
	}
	debugLog("Engine: Initializing subsystems done.\n\n");
}

Engine::~Engine()
{
	debugLog("\n-= Engine Shutdown =-\n");

	debugLog("Engine: Freeing app...\n");
	SAFE_DELETE(m_app);

	if (m_console != NULL)
		showMessageErrorFatal("Engine Error", "m_console not set to NULL before shutdown!");

	debugLog("Engine: Freeing engine GUI...\n");
	{
		m_console = NULL;
		m_consoleBox = NULL;
	}
	SAFE_DELETE(m_guiContainer);

	debugLog("Engine: Freeing resource manager...\n");
	SAFE_DELETE(m_resourceManager);

	debugLog("Engine: Freeing OpenCL...\n");
	SAFE_DELETE(m_openCL);

	debugLog("Engine: Freeing OpenVR...\n");
	SAFE_DELETE(m_openVR);

	debugLog("Engine: Freeing Sound...\n");
	SAFE_DELETE(m_sound);

	debugLog("Engine: Freeing context menu...\n");
	SAFE_DELETE(m_contextMenu);

	debugLog("Engine: Freeing animation handler...\n");
	SAFE_DELETE(m_animationHandler);

	debugLog("Engine: Freeing network handler...\n");
	SAFE_DELETE(m_networkHandler);

	debugLog("Engine: Freeing Steam...\n");
	SAFE_DELETE(m_steam);

	debugLog("Engine: Freeing Discord...\n");
	SAFE_DELETE(m_discord);

	debugLog("Engine: Freeing input devices...\n");
	for (size_t i=0; i<m_inputDevices.size(); i++)
	{
		delete m_inputDevices[i];
	}
	m_inputDevices.clear();

	debugLog("Engine: Freeing timer...\n");
	SAFE_DELETE(m_timer);

	debugLog("Engine: Freeing graphics...\n");
	SAFE_DELETE(m_graphics);

	debugLog("Engine: Freeing Vulkan...\n");
	SAFE_DELETE(m_vulkan);

	debugLog("Engine: Freeing environment...\n");
	SAFE_DELETE(m_environment);

	debugLog("Engine: Goodbye.");

	engine = NULL;
}

void Engine::loadApp()
{
	// load core default resources (these are required to be able to draw the loading screen)
	if (m_iLoadingScreenDelay == 0 || m_iLoadingScreenDelay == -2)
	{
		debugLog("Engine: Loading default resources ...\n");
		{
			engine->getResourceManager()->loadFont("weblysleekuisb.ttf", "FONT_DEFAULT", 15, true, m_environment->getDPI());
			engine->getResourceManager()->loadFont("tahoma.ttf", "FONT_CONSOLE", 8, false, 96);
		}
		debugLog("Engine: Loading default resources done.\n");
	}

	// allow the engine to draw an initial loading screen before the real app is loaded
	// this schedules another delayed loadApp() call in update()
	if (m_iLoadingScreenDelay == 0)
	{
		m_app = new EngineLoadingScreenApp();
		m_iLoadingScreenDelay = 1; // allow drawing 1 single frame
		return; // NOTE: early return
	}
	else
		SAFE_DELETE(m_app);

	// load other default resources and things which are not strictly necessary
	{
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
		m_guiContainer = new CBaseUIContainer(0, 0, engine->getScreenWidth(), engine->getScreenHeight(), "");
		m_consoleBox = new ConsoleBox();
		m_visualProfiler = new VisualProfiler();
		m_guiContainer->addBaseUIElement(m_visualProfiler);
		m_guiContainer->addBaseUIElement(m_consoleBox);

		// (engine gui comes first)
		m_keyboard->addListener(m_guiContainer, true);
	}

	debugLog("\nEngine: Loading app ...\n");
	{
		//*****************//
		//	Load App here  //
		//*****************//

		//m_app = new Osu();

		//m_app = new GUICoherenceMode();

		//m_app = new Asteroids();

		m_app = new FrameworkTest();



		// start listening to the default keyboard input
		if (m_app != NULL)
			m_keyboard->addListener(m_app);
	}
	debugLog("Engine: Loading app done.\n\n");
}

void Engine::onPaint()
{
	VPROF_BUDGET("Engine::onPaint", VPROF_BUDGETGROUP_DRAW);
	if (m_bBlackout || m_bIsMinimized) return;

	m_bDrawing = true;
	{
		// begin
		{
			VPROF_BUDGET("Graphics::beginScene", VPROF_BUDGETGROUP_DRAW);
			m_graphics->beginScene();
		}

		// middle
		{
			if (m_app != NULL)
			{
				VPROF_BUDGET("App::draw", VPROF_BUDGETGROUP_DRAW);
				m_app->draw(m_graphics);
			}

			if (m_guiContainer != NULL)
				m_guiContainer->draw(m_graphics);

			// debug input devices
			for (size_t i=0; i<m_inputDevices.size(); i++)
			{
				m_inputDevices[i]->draw(m_graphics);
			}

			if (epilepsy.getBool())
			{
				m_graphics->setColor(COLOR(255, rand()%256, rand()%256, rand()%256));
				m_graphics->fillRect(0, 0, engine->getScreenWidth(), engine->getScreenHeight());
			}
		}

		// end
		{
			VPROF_BUDGET("Graphics::endScene", VPROF_BUDGETGROUP_DRAW_SWAPBUFFERS);
			m_graphics->endScene();
		}
	}
	m_bDrawing = false;

	m_iFrameCount++;
}

void Engine::onUpdate()
{
	VPROF_BUDGET("Engine::onUpdate", VPROF_BUDGETGROUP_UPDATE);

	if (m_iLoadingScreenDelay > 0 && m_iFrameCount >= m_iLoadingScreenDelay)
	{
		m_iLoadingScreenDelay = -1;
		loadApp();
	}

	if (m_bBlackout || (m_bIsMinimized && !(m_networkHandler->isClient() || m_networkHandler->isServer()))) return;

	// update time
	{
		m_timer->update();
		m_dRunTime = m_timer->getElapsedTime();
		m_dFrameTime *= (double)host_timescale.getFloat();
		m_dTime += m_dFrameTime;
	}

	// handle pending queued resolution changes
	if (m_bResolutionChange)
	{
		m_bResolutionChange = false;

		if (debug_engine.getBool())
			debugLog("Engine: executing pending queued resolution change to (%i, %i)\n", (int)m_vNewScreenSize.x, (int)m_vNewScreenSize.y);

		onResolutionChange(m_vNewScreenSize);
	}

	// update miscellaneous engine subsystems
	{
		for (size_t i=0; i<m_inputDevices.size(); i++)
		{
			m_inputDevices[i]->update();
		}

		m_openVR->update(); // (this also handles its input devices)

		{
			VPROF_BUDGET("AnimationHandler::update", VPROF_BUDGETGROUP_UPDATE);
			m_animationHandler->update();
		}

		{
			VPROF_BUDGET("SoundEngine::update", VPROF_BUDGETGROUP_UPDATE);
			m_sound->update();
		}

		{
			VPROF_BUDGET("ResourceManager::update", VPROF_BUDGETGROUP_UPDATE);
			m_resourceManager->update();
		}

		// update gui
		if (m_guiContainer != NULL)
			m_guiContainer->update();

		// execute queued commands
		// TODO: this is shit
		if (Console::g_commandQueue.size() > 0)
		{
			for (size_t i=0; i<Console::g_commandQueue.size(); i++)
			{
				Console::processCommand(Console::g_commandQueue[i]);
			}
			Console::g_commandQueue = std::vector<UString>(); // reset
		}

		// update networking
		{
			VPROF_BUDGET("NetworkHandler::update", VPROF_BUDGETGROUP_UPDATE);
			m_networkHandler->update();
		}
	}

	// update app
	if (m_app != NULL)
	{
		VPROF_BUDGET("App::update", VPROF_BUDGETGROUP_UPDATE);
		m_app->update();
	}

	// update environment (after app, at the end here)
	{
		VPROF_BUDGET("Environment::update", VPROF_BUDGETGROUP_UPDATE);
		m_environment->update();
	}
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

	for (size_t i=0; i<m_keyboards.size(); i++)
	{
		m_keyboards[i]->reset();
	}

	if (m_app != NULL)
		m_app->onFocusLost();

	// auto minimize on certain conditions
	if (m_environment->isFullscreen() || m_environment->isFullscreenWindowedBorderless())
	{
		if ((!m_environment->isFullscreenWindowedBorderless() && minimize_on_focus_lost_if_fullscreen.getBool())
		  || (m_environment->isFullscreenWindowedBorderless() && minimize_on_focus_lost_if_borderless_windowed_fullscreen.getBool()))
		{
			m_environment->minimize();
		}
	}
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
		newResolution = Vector2(2, 2);
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

void Engine::onDPIChange()
{
	debugLog(0xff00ff00, "Engine: DPI changed to %i\n", m_environment->getDPI());

	if (m_app != NULL)
		m_app->onDPIChanged();
}

void Engine::onShutdown()
{
	if (m_bBlackout || (m_app != NULL && !m_app->onShutdown())) return;

	m_bBlackout = true;
	m_environment->shutdown();
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

#ifdef MCENGINE_FEATURE_MULTITHREADING

std::mutex g_engineMouseLeftClickMutex;

#endif

void Engine::onMouseLeftChange(bool mouseLeftDown)
{
#ifdef MCENGINE_FEATURE_MULTITHREADING

	std::lock_guard<std::mutex> lk(g_engineMouseLeftClickMutex); // async calls from WinRealTimeStylus must be protected

#endif

	if (m_mouse->isLeftDown() != mouseLeftDown) // necessary due to WinRealTimeStylus and Touch, would cause double clicks otherwise
		m_mouse->onLeftChange(mouseLeftDown);
}

void Engine::onMouseMiddleChange(bool mouseMiddleDown)
{
	m_mouse->onMiddleChange(mouseMiddleDown);
}

void Engine::onMouseRightChange(bool mouseRightDown)
{
	if (m_mouse->isRightDown() != mouseRightDown) // necessary due to Touch, would cause double clicks otherwise
		m_mouse->onRightChange(mouseRightDown);
}

void Engine::onMouseButton4Change(bool mouse4down)
{
	m_mouse->onButton4Change(mouse4down);
}

void Engine::onMouseButton5Change(bool mouse5down)
{
	m_mouse->onButton5Change(mouse5down);
}

void Engine::onKeyboardKeyDown(KEYCODE keyCode)
{
	// hardcoded engine hotkeys
	{
		// handle ALT+F4 quit
		if (m_keyboard->isAltDown() && keyCode == KEY_F4)
		{
			shutdown();
			return;
		}

		// handle ALT+ENTER fullscreen toggle
		if (m_keyboard->isAltDown() && keyCode == KEY_ENTER)
		{
			toggleFullscreen();
			return;
		}

		// handle CTRL+F11 profiler toggle
		if (m_keyboard->isControlDown() && keyCode == KEY_F11)
		{
			ConVar *vprof = convar->getConVarByName("vprof");
			vprof->setValue(vprof->getBool() ? 0.0f : 1.0f);
			return;
		}

		// handle profiler display mode change
		if (m_keyboard->isControlDown() && keyCode == KEY_TAB)
		{
			const ConVar *vprof = convar->getConVarByName("vprof");
			if (vprof->getBool())
			{
				if (m_keyboard->isShiftDown())
					m_visualProfiler->decrementInfoBladeDisplayMode();
				else
					m_visualProfiler->incrementInfoBladeDisplayMode();
				return;
			}
		}
	}

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
	onShutdown();
}

void Engine::restart()
{
	onShutdown();
	m_environment->restart();
}

void Engine::sleep(unsigned int us)
{
	m_environment->sleep(us);
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

	for (size_t i=0; i<m_gamepads.size(); i++)
	{
		if (m_gamepads[i] == gamepad)
		{
			m_gamepads.erase(m_gamepads.begin()+i);
			break;
		}
	}
}

void Engine::requestResolutionChange(Vector2 newResolution)
{
	if (newResolution == m_vNewScreenSize) return;

	m_vNewScreenSize = newResolution;
	m_bResolutionChange = true;
}

void Engine::setFrameTime(double delta)
{
	// NOTE: clamp to between 10000 fps and 1 fps, very small/big timesteps could cause problems
	// NOTE: special case for loading screen and first app frame
	if (m_iFrameCount < 3)
		m_dFrameTime = delta;
	else
		m_dFrameTime = clamp<double>(delta, 0.0001, 1.0);
}

double const Engine::getTimeReal()
{
	m_timer->update();
	return m_timer->getElapsedTime();
}

void Engine::debugLog(const char *fmt, va_list args)
{
	if (fmt == NULL) return;

	va_list ap2;
	va_copy(ap2, args);

	// write to console
	int numChars = vprintf(fmt, args);

	if (numChars < 1 || numChars > 65534)
		goto cleanup;

	// write to engine console
	{
		char *buffer = new char[numChars + 1]; // +1 for null termination later
		vsnprintf(buffer, numChars + 1, fmt, ap2); // "The generated string has a length of at most n-1, leaving space for the additional terminating null character."
		buffer[numChars] = '\0'; // null terminate

		UString actualBuffer = UString(buffer);
		delete[] buffer;

		// WARNING: these calls here are not threadsafe by default
		if (m_consoleBox != NULL)
			m_consoleBox->log(actualBuffer);
		if (m_console != NULL)
			m_console->log(actualBuffer);
	}

cleanup:
	va_end(ap2);
}

void Engine::debugLog(Color color, const char *fmt, va_list args)
{
	if (fmt == NULL) return;

	va_list ap2;
	va_copy(ap2, args);

	// write to console
	int numChars = vprintf(fmt, args);

	if (numChars < 1 || numChars > 65534)
		goto cleanup;

	// write to engine console
	{
		char *buffer = new char[numChars + 1]; // +1 for null termination later
		vsnprintf(buffer, numChars + 1, fmt, ap2); // "The generated string has a length of at most n-1, leaving space for the additional terminating null character."
		buffer[numChars] = '\0'; // null terminate

		UString actualBuffer = UString(buffer);
		delete[] buffer;

		// WARNING: these calls here are not threadsafe by default
		if (m_consoleBox != NULL)
			m_consoleBox->log(actualBuffer, color);
		if (m_console != NULL)
			m_console->log(actualBuffer, color);
	}

cleanup:
	va_end(ap2);
}

void Engine::debugLog(const char *fmt, ...)
{
	if (fmt == NULL) return;

	va_list ap;
	va_start(ap, fmt);

	debugLog(fmt, ap);

	va_end(ap);
}

void Engine::debugLog(Color color, const char *fmt, ...)
{
	if (fmt == NULL) return;

	va_list ap;
	va_start(ap, fmt);

	debugLog(color, fmt, ap);

	va_end(ap);
}



//**********************//
//	Engine ConCommands	//
//**********************//

void _exit(void)
{
	engine->shutdown();
}

void _restart(void)
{
	engine->restart();
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

void _borderless(void)
{
	ConVar *fullscreen_windowed_borderless_ref = convar->getConVarByName("fullscreen_windowed_borderless");
	if (fullscreen_windowed_borderless_ref != NULL)
	{
		if (fullscreen_windowed_borderless_ref->getBool())
		{
			fullscreen_windowed_borderless_ref->setValue(0.0f);
			if (env->isFullscreen())
				env->disableFullscreen();
		}
		else
		{
			fullscreen_windowed_borderless_ref->setValue(1.0f);
			if (!env->isFullscreen())
				env->enableFullscreen();
		}
	}
}

void _windowed(UString args)
{
	env->disableFullscreen();

	if (args.length() < 7) return;

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

void _maximize(void)
{
	env->maximize();
}

void _toggleresizable(void)
{
	env->setWindowResizable(!env->isWindowResizable());
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

void _dpiinfo(void)
{
	debugLog("env->getDPI() = %i, env->getDPIScale() = %f\n", env->getDPI(), env->getDPIScale());
}

ConVar _exit_("exit", _exit);
ConVar _shutdown_("shutdown", _exit);
ConVar _restart_("restart", _restart);
ConVar _printsize_("printsize", _printsize);
ConVar _fullscreen_("fullscreen", _fullscreen);
ConVar _borderless_("borderless", _borderless);
ConVar _windowed_("windowed", _windowed);
ConVar _minimize_("minimize", _minimize);
ConVar _maximize_("maximize", _maximize);
ConVar _resizable_toggle_("resizable_toggle", _toggleresizable);
ConVar _focus_("focus", _focus);
ConVar _center_("center", _center);
ConVar _version_("version", _version);
ConVar _corporeal_("debug_ghost", false, _debugCorporeal);
ConVar _errortest_("errortest", _errortest);
ConVar _crash_("crash", _crash);
ConVar _dpiinfo_("dpiinfo", _dpiinfo);
