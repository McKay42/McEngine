//================ Copyright (c) 2018, PG, All rights reserved. =================//
//
// Purpose:		main entry point
//
// $NoKeywords: $main
//===============================================================================//

#include "EngineFeatures.h"

#ifdef MCENGINE_FEATURE_SDL

//#define MCENGINE_SDL_JOYSTICK
//#define MCENGINE_SDL_JOYSTICK_MOUSE

#if !defined(MCENGINE_FEATURE_OPENGL) && !defined(MCENGINE_FEATURE_OPENGLES)
#error OpenGL support is currently required for SDL
#endif

#include "SDL.h"

#include "Engine.h"
#include "Profiler.h"
#include "Timer.h"
#include "Mouse.h"
#include "ConVar.h"
#include "ConsoleBox.h"

#include "SDLEnvironment.h"
#include "HorizonSDLEnvironment.h"
#include "WinSDLEnvironment.h"



#define WINDOW_TITLE "McEngine"

#define WINDOW_WIDTH (1280)
#define WINDOW_HEIGHT (720)

#define WINDOW_WIDTH_MIN 100
#define WINDOW_HEIGHT_MIN 100



Engine *g_engine = NULL;

bool g_bRunning = true;
bool g_bUpdate = true;
bool g_bDraw = true;
bool g_bDrawing = false;

bool g_bMinimized = false; // for fps_max_background
bool g_bHasFocus = true; // for fps_max_background

SDL_Window *g_window = NULL;

ConVar fps_max("fps_max", 60.0f, FCVAR_NONE, "framerate limiter, foreground");
ConVar fps_max_yield("fps_max_yield", true, FCVAR_NONE, "always release rest of timeslice once per frame (call scheduler via sleep(0))");
ConVar fps_max_background("fps_max_background", 30.0f, FCVAR_NONE, "framerate limiter, background");
ConVar fps_unlimited("fps_unlimited", false, FCVAR_NONE);
ConVar fps_unlimited_yield("fps_unlimited_yield", true, FCVAR_NONE, "always release rest of timeslice once per frame (call scheduler via sleep(0)), even if unlimited fps are enabled");

ConVar sdl_joystick_mouse_sensitivity("sdl_joystick_mouse_sensitivity", 1.0f, FCVAR_NONE);
ConVar sdl_joystick0_deadzone("sdl_joystick0_deadzone", 0.3f, FCVAR_NONE);
ConVar sdl_joystick_zl_threshold("sdl_joystick_zl_threshold", -0.5f, FCVAR_NONE);
ConVar sdl_joystick_zr_threshold("sdl_joystick_zr_threshold", -0.5f, FCVAR_NONE);

int mainSDL(int argc, char *argv[], SDLEnvironment *customSDLEnvironment)
{
	SDLEnvironment *environment = customSDLEnvironment;

	uint32_t flags = SDL_INIT_VIDEO;

#ifdef MCENGINE_SDL_JOYSTICK

	flags |= SDL_INIT_JOYSTICK;

#endif

#ifdef MCENGINE_FEATURE_SDL_MIXER

	flags |= SDL_INIT_AUDIO;

#endif

	// initialize sdl
	if (SDL_Init(flags) < 0)
	{
		fprintf(stderr, "Couldn't SDL_Init(): %s\n", SDL_GetError());
		return 1;
	}

	// pre window-creation settings
#ifdef MCENGINE_FEATURE_OPENGL

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

#endif

#ifdef MCENGINE_FEATURE_OPENGLES

	// NOTE: hardcoded to OpenGL ES 2.0 currently (for nintendo switch builds)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

#endif

	uint32_t windowFlags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_FOREIGN;

#if defined(MCENGINE_FEATURE_OPENGL) || defined(MCENGINE_FEATURE_OPENGLES)

	windowFlags |= SDL_WINDOW_OPENGL;

#endif

	// create window
	g_window = SDL_CreateWindow(
			WINDOW_TITLE,
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			WINDOW_WIDTH, WINDOW_HEIGHT,
			windowFlags);

	if (g_window == NULL)
	{
		fprintf(stderr, "Couldn't SDL_CreateWindow(): %s\n", SDL_GetError());
		return 1;
	}

	// get the screen refresh rate, and set fps_max to that as default
	{
		SDL_DisplayMode currentDisplayMode;
		currentDisplayMode.refresh_rate = fps_max.getInt();

		SDL_GetCurrentDisplayMode(SDL_GetWindowDisplayIndex(g_window), &currentDisplayMode);

		if (currentDisplayMode.refresh_rate > 0)
		{
			///printf("Display Refresh Rate is %i Hz, setting fps_max to %i.\n\n", currentDisplayMode.refresh_rate, currentDisplayMode.refresh_rate);
			fps_max.setValue(currentDisplayMode.refresh_rate);
			fps_max.setDefaultFloat(currentDisplayMode.refresh_rate);
		}
	}

	// post window-creation settings
	SDL_SetWindowMinimumSize(g_window, WINDOW_WIDTH_MIN, WINDOW_HEIGHT_MIN);

	// create OpenGL context
#if defined(MCENGINE_FEATURE_OPENGL) || defined(MCENGINE_FEATURE_OPENGLES)

	SDL_GLContext context = SDL_GL_CreateContext(g_window);

#endif

#ifdef MCENGINE_SDL_JOYSTICK

	SDL_JoystickOpen(0);
	SDL_JoystickOpen(1);

	float m_fJoystick0XPercent = 0.0f;
	float m_fJoystick0YPercent = 0.0f;

	bool xDown = false;

	bool zlDown = false;
	bool zrDown = false;

	bool hatUpDown = false;
	bool hatDownDown = false;
	bool hatLeftDown = false;
	bool hatRightDown = false;

#endif

    // create timers
    Timer *frameTimer = new Timer();
    frameTimer->start();
    frameTimer->update();

    Timer *deltaTimer = new Timer();
    deltaTimer->start();
    deltaTimer->update();

    // make the window visible
    SDL_ShowWindow(g_window);
	SDL_RaiseWindow(g_window);

	// initialize engine
	if (environment == NULL)
		environment = new SDLEnvironment(g_window);
	else
		environment->setWindow(g_window);

	g_engine = new Engine(environment, argc > 1 ? argv[1] : ""); // TODO: proper arg support
	g_engine->loadApp();

	frameTimer->update();
	deltaTimer->update();

	// custom
	Vector2 mousePos;
	ConVar *mouse_raw_input_ref = convar->getConVarByName("mouse_raw_input");

	// main loop
	SDL_Event e;
	while (g_bRunning)
	{
		VPROF_MAIN();

		// HACKHACK: switch hack (usb mouse/keyboard support)
#ifdef __SWITCH__

		HorizonSDLEnvironment *horizonSDLenv = dynamic_cast<HorizonSDLEnvironment*>(environment);
		if (horizonSDLenv != NULL)
			horizonSDLenv->update_before_winproc();

#endif

		// handle window message queue
		{
			VPROF_BUDGET("SDL", VPROF_BUDGETGROUP_WNDPROC);

			// handle automatic raw input toggling
			{
				const bool isRawInputActuallyEnabled = (SDL_GetRelativeMouseMode() == SDL_TRUE);

				const bool shouldRawInputBeEnabled = (mouse_raw_input_ref->getBool() && !environment->isCursorVisible());

				if (shouldRawInputBeEnabled != isRawInputActuallyEnabled)
					SDL_SetRelativeMouseMode(shouldRawInputBeEnabled ? SDL_TRUE : SDL_FALSE);
			}

			const bool isRawInputEnabled = (SDL_GetRelativeMouseMode() == SDL_TRUE);

			while (SDL_PollEvent(&e) != 0)
			{
				switch (e.type)
				{
				case SDL_QUIT:
					if (g_bRunning)
					{
						g_bRunning = false;
						g_engine->onShutdown();
					}
					break;

				// window
				case SDL_WINDOWEVENT:
					switch (e.window.event)
					{
					case SDL_WINDOWEVENT_CLOSE:
						if (g_bRunning)
						{
							g_bRunning = false;
							g_engine->onShutdown();
						}
						break;
					case SDL_WINDOWEVENT_FOCUS_GAINED:
						g_bHasFocus = true;
						g_engine->onFocusGained();
						break;
					case SDL_WINDOWEVENT_FOCUS_LOST:
						g_bHasFocus = false;
						g_engine->onFocusLost();
						break;
					case SDL_WINDOWEVENT_MAXIMIZED:
						g_bMinimized = false;
						g_engine->onMaximized();
						break;
					case SDL_WINDOWEVENT_MINIMIZED:
						g_bMinimized = true;
						g_engine->onMinimized();
						break;
					case SDL_WINDOWEVENT_RESTORED:
						g_bMinimized = false;
						g_engine->onRestored();
						break;
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						g_engine->requestResolutionChange(Vector2(e.window.data1, e.window.data2));
						break;
					}
					break;

				// keyboard
				case SDL_KEYDOWN:
					g_engine->onKeyboardKeyDown(e.key.keysym.scancode);
					break;

				case SDL_KEYUP:
					g_engine->onKeyboardKeyUp(e.key.keysym.scancode);
					break;

				case SDL_TEXTINPUT:
					{
						UString nullTerminatedTextInputString(e.text.text);
						for (int i=0; i<nullTerminatedTextInputString.length(); i++)
						{
							g_engine->onKeyboardChar((KEYCODE)nullTerminatedTextInputString[i]); // NOTE: this splits into UTF-16 wchar_t atm
						}
					}
					break;

				// mouse
				case SDL_MOUSEBUTTONDOWN:
					switch (e.button.button)
					{
					case SDL_BUTTON_LEFT:
						g_engine->onMouseLeftChange(true);
						break;
					case SDL_BUTTON_MIDDLE:
						g_engine->onMouseMiddleChange(true);
						break;
					case SDL_BUTTON_RIGHT:
						g_engine->onMouseRightChange(true);
						break;

					case SDL_BUTTON_X1:
						g_engine->onMouseButton4Change(true);
						break;
					case SDL_BUTTON_X2:
						g_engine->onMouseButton5Change(true);
						break;
					}
					break;

				case SDL_MOUSEBUTTONUP:
					switch (e.button.button)
					{
					case SDL_BUTTON_LEFT:
						g_engine->onMouseLeftChange(false);
						break;
					case SDL_BUTTON_MIDDLE:
						g_engine->onMouseMiddleChange(false);
						break;
					case SDL_BUTTON_RIGHT:
						g_engine->onMouseRightChange(false);
						break;

					case SDL_BUTTON_X1:
						g_engine->onMouseButton4Change(false);
						break;
					case SDL_BUTTON_X2:
						g_engine->onMouseButton5Change(false);
						break;
					}
					break;

				case SDL_MOUSEWHEEL:
					if (e.wheel.x != 0)
						g_engine->onMouseWheelHorizontal(e.wheel.x > 0 ? 120*std::abs(e.wheel.x) : -120*std::abs(e.wheel.x)); // NOTE: convert to Windows units
					if (e.wheel.y != 0)
						g_engine->onMouseWheelVertical(e.wheel.y > 0 ? 120*std::abs(e.wheel.y) : -120*std::abs(e.wheel.y)); // NOTE: convert to Windows units
					break;

				case SDL_MOUSEMOTION:
					if (isRawInputEnabled)
						g_engine->onMouseRawMove(e.motion.xrel, e.motion.yrel);
					break;

				// touch mouse
				// NOTE: sometimes when quickly tapping with two fingers, events will get lost (due to the touchscreen believing that it was one finger which moved very quickly, instead of 2 tapping fingers)
				case SDL_FINGERDOWN:
					if (e.tfinger.fingerId == 0)
					{
						mousePos = Vector2(e.tfinger.x, e.tfinger.y) * g_engine->getScreenSize();
						environment->setMousePos(mousePos.x, mousePos.y);
						g_engine->getMouse()->onPosChange(mousePos);

						if (g_engine->getMouse()->isLeftDown())
							g_engine->onMouseLeftChange(false);

						g_engine->onMouseLeftChange(true);
					}
					else if (e.tfinger.fingerId == 1)
					{
						if (g_engine->getMouse()->isLeftDown())
							g_engine->onMouseLeftChange(false);

						g_engine->onMouseLeftChange(true);
					}
					else if (e.tfinger.fingerId == 2)
					{
						if (g_engine->getMouse()->isLeftDown())
							g_engine->onMouseLeftChange(false);

						g_engine->onMouseLeftChange(true);
					}
					break;

				case SDL_FINGERUP:
					if (e.tfinger.fingerId == 0)
						g_engine->onMouseLeftChange(false);
					//else if (e.tfinger.fingerId == 1)
					//	g_engine->onMouseRightChange(false);
					//else if (e.tfinger.fingerId == 2)
					//	g_engine->onMouseLeftChange(false);
					break;

				case SDL_FINGERMOTION:
					if (e.tfinger.fingerId == 0)
					{
						mousePos = Vector2(e.tfinger.x, e.tfinger.y) * g_engine->getScreenSize();
						environment->setMousePos(mousePos.x, mousePos.y);
						g_engine->getMouse()->onPosChange(mousePos);
					}
					break;

				// joystick keyboard
				// NOTE: defaults to xbox 360 controller layout on all non-horizon environments
#ifdef MCENGINE_SDL_JOYSTICK

				case SDL_JOYBUTTONDOWN:
					//debugLog("joybuttondown: %i button %i down\n", (int)e.jbutton.which, (int)e.jbutton.button);
					if (e.jbutton.button == 0) // KEY_A
					{
						g_engine->onMouseLeftChange(true);

						if (engine->getConsoleBox()->isActive())
						{
							g_engine->onKeyboardKeyDown(SDL_SCANCODE_RETURN);
							g_engine->onKeyboardKeyUp(SDL_SCANCODE_RETURN);
						}
					}
					else if ((env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 10 : e.jbutton.button == 7) || e.jbutton.button == 1) // KEY_PLUS/KEY_START || KEY_B
						g_engine->onKeyboardKeyDown(SDL_SCANCODE_ESCAPE);
					else if (e.jbutton.button == 2) // KEY_X
					{
						g_engine->onKeyboardKeyDown(SDL_SCANCODE_X);
						xDown = true;
					}
					else if (e.jbutton.button == 3) // KEY_Y
						g_engine->onKeyboardKeyDown(SDL_SCANCODE_Y);
					else if ((env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 21 : false) || (env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 13 : false)) // right stick up || dpad up
						g_engine->onKeyboardKeyDown(SDL_SCANCODE_UP);
					else if ((env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 23 : false) || (env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 15 : false)) // right stick down || dpad down
						g_engine->onKeyboardKeyDown(SDL_SCANCODE_DOWN);
					else if ((env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 20 : false) || (env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 12 : false)) // right stick left || dpad left
						g_engine->onKeyboardKeyDown(SDL_SCANCODE_LEFT);
					else if ((env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 22 : false) || (env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 14 : false)) // right stick right || dpad right
						g_engine->onKeyboardKeyDown(SDL_SCANCODE_RIGHT);
					else if ((env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 6 : e.jbutton.button == 4)) // KEY_L
						g_engine->onKeyboardKeyDown((env->getOS() == Environment::OS::OS_HORIZON ? SDL_SCANCODE_L : SDL_SCANCODE_BACKSPACE));
					else if ((env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 7 : e.jbutton.button == 5)) // KEY_R
						g_engine->onKeyboardKeyDown((env->getOS() == Environment::OS::OS_HORIZON ? SDL_SCANCODE_R : SDL_SCANCODE_LSHIFT));
					else if ((env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 8 : false)) // KEY_ZL
						g_engine->onKeyboardKeyDown(SDL_SCANCODE_Z);
					else if ((env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 9 : false)) // KEY_ZR
						g_engine->onKeyboardKeyDown(SDL_SCANCODE_V);
					else if ((env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 11 : e.jbutton.button == 6)) // KEY_MINUS/KEY_SELECT
						g_engine->onKeyboardKeyDown(SDL_SCANCODE_F1);
					else if ((env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 4 : e.jbutton.button == 8)) // left stick press
					{
						// toggle options (CTRL + O)
						g_engine->onKeyboardKeyDown(SDL_SCANCODE_LCTRL);
						g_engine->onKeyboardKeyDown(SDL_SCANCODE_O);
						g_engine->onKeyboardKeyUp(SDL_SCANCODE_LCTRL);
						g_engine->onKeyboardKeyUp(SDL_SCANCODE_O);
					}
					else if ((env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 5 : e.jbutton.button == 9)) // right stick press
					{
						if (xDown)
						{
							// toggle console
							g_engine->onKeyboardKeyDown(SDL_SCANCODE_LSHIFT);
							g_engine->onKeyboardKeyDown(SDL_SCANCODE_F1);
							g_engine->onKeyboardKeyUp(SDL_SCANCODE_LSHIFT);
							g_engine->onKeyboardKeyUp(SDL_SCANCODE_F1);
						}
						else
						{
#ifdef __SWITCH__

							((HorizonSDLEnvironment*)environment)->showKeyboard();

#endif
						}
					}
					break;

				case SDL_JOYBUTTONUP:
					if (e.jbutton.button == 0) // KEY_A
						g_engine->onMouseLeftChange(false);
					else if ((env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 10 : e.jbutton.button == 7) || e.jbutton.button == 1) // KEY_PLUS/KEY_START || KEY_B
						g_engine->onKeyboardKeyUp(SDL_SCANCODE_ESCAPE);
					else if (e.jbutton.button == 2) // KEY_X
					{
						g_engine->onKeyboardKeyUp(SDL_SCANCODE_X);
						xDown = false;
					}
					else if (e.jbutton.button == 3) // KEY_Y
						g_engine->onKeyboardKeyUp(SDL_SCANCODE_Y);
					else if ((env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 21 : false) || (env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 13 : false)) // right stick up || dpad up
						g_engine->onKeyboardKeyUp(SDL_SCANCODE_UP);
					else if ((env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 23 : false) || (env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 15 : false)) // right stick down || dpad down
						g_engine->onKeyboardKeyUp(SDL_SCANCODE_DOWN);
					else if ((env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 20 : false) || (env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 12 : false)) // right stick left || dpad left
						g_engine->onKeyboardKeyUp(SDL_SCANCODE_LEFT);
					else if ((env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 22 : false) || (env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 14 : false)) // right stick right || dpad right
						g_engine->onKeyboardKeyUp(SDL_SCANCODE_RIGHT);
					else if ((env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 6 : e.jbutton.button == 4)) // KEY_L
						g_engine->onKeyboardKeyUp((env->getOS() == Environment::OS::OS_HORIZON ? SDL_SCANCODE_L : SDL_SCANCODE_BACKSPACE));
					else if ((env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 7 : e.jbutton.button == 5)) // KEY_R
						g_engine->onKeyboardKeyUp((env->getOS() == Environment::OS::OS_HORIZON ? SDL_SCANCODE_R : SDL_SCANCODE_LSHIFT));
					else if ((env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 8 : false)) // KEY_ZL
						g_engine->onKeyboardKeyUp(SDL_SCANCODE_Z);
					else if ((env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 9 : false)) // KEY_ZR
						g_engine->onKeyboardKeyUp(SDL_SCANCODE_V);
					else if ((env->getOS() == Environment::OS::OS_HORIZON ? e.jbutton.button == 11 : e.jbutton.button == 6)) // KEY_MINUS/KEY_SELECT
						g_engine->onKeyboardKeyUp(SDL_SCANCODE_F1);
					break;

				case SDL_JOYAXISMOTION:
					//debugLog("joyaxismotion: stick %i : axis = %i, value = %i\n", (int)e.jaxis.which, (int)e.jaxis.axis, (int)e.jaxis.value);
					// left stick
					if (e.jaxis.axis == 1 || e.jaxis.axis == 0)
					{
						if (e.jaxis.axis == 0)
							m_fJoystick0XPercent = clamp<float>((float)e.jaxis.value / 32767.0f, -1.0f, 1.0f);
						else
							m_fJoystick0YPercent = clamp<float>((float)e.jaxis.value / 32767.0f, -1.0f, 1.0f);
					}
					if (env->getOS() != Environment::OS::OS_HORIZON)
					{
						// ZL/ZR
						if (e.jaxis.axis == 2 || e.jaxis.axis == 5)
						{
							if (e.jaxis.axis == 2)
							{
								const float threshold = sdl_joystick_zl_threshold.getFloat();
								const float percent = clamp<float>((float)e.jaxis.value / 32767.0f, -1.0f, 1.0f);
								const bool wasZlDown = zlDown;
								zlDown = !(threshold <= 0.0f ? percent <= threshold : percent >= threshold);
								if (zlDown != wasZlDown)
								{
									if (zlDown)
										g_engine->onKeyboardKeyDown(SDL_SCANCODE_KP_MINUS);
									else
										g_engine->onKeyboardKeyUp(SDL_SCANCODE_KP_MINUS);
								}
							}
							else
							{
								const float threshold = sdl_joystick_zr_threshold.getFloat();
								const float percent = clamp<float>((float)e.jaxis.value / 32767.0f, -1.0f, 1.0f);
								const bool wasZrDown = zrDown;
								zrDown = !(threshold <= 0.0f ? percent <= threshold : percent >= threshold);
								if (zrDown != wasZrDown)
								{
									if (zrDown)
										g_engine->onKeyboardKeyDown(SDL_SCANCODE_KP_PLUS);
									else
										g_engine->onKeyboardKeyUp(SDL_SCANCODE_KP_PLUS);
								}
							}
						}
					}
					break;

				case SDL_JOYHATMOTION:
					//debugLog("joyhatmotion: hat %i : value = %i\n", (int)e.jhat.hat, (int)e.jhat.value);
					if (env->getOS() != Environment::OS::OS_HORIZON)
					{
						const bool wasHatUpDown = hatUpDown;
						const bool wasHatDownDown = hatDownDown;
						const bool wasHatLeftDown = hatLeftDown;
						const bool wasHatRightDown = hatRightDown;

						hatUpDown = (e.jhat.value == SDL_HAT_UP);
						hatDownDown = (e.jhat.value == SDL_HAT_DOWN);
						hatLeftDown = (e.jhat.value == SDL_HAT_LEFT);
						hatRightDown = (e.jhat.value == SDL_HAT_RIGHT);

						if (hatUpDown != wasHatUpDown)
						{
							if (hatUpDown)
								g_engine->onKeyboardKeyDown(SDL_SCANCODE_UP);
							else
								g_engine->onKeyboardKeyUp(SDL_SCANCODE_UP);
						}

						if (hatDownDown != wasHatDownDown)
						{
							if (hatDownDown)
								g_engine->onKeyboardKeyDown(SDL_SCANCODE_DOWN);
							else
								g_engine->onKeyboardKeyUp(SDL_SCANCODE_DOWN);
						}

						if (hatLeftDown != wasHatLeftDown)
						{
							if (hatLeftDown)
								g_engine->onKeyboardKeyDown(SDL_SCANCODE_LEFT);
							else
								g_engine->onKeyboardKeyUp(SDL_SCANCODE_LEFT);
						}

						if (hatRightDown != wasHatRightDown)
						{
							if (hatRightDown)
								g_engine->onKeyboardKeyDown(SDL_SCANCODE_RIGHT);
							else
								g_engine->onKeyboardKeyUp(SDL_SCANCODE_RIGHT);
						}
					}
					break;

#endif
				}
			}
		}

		// update
		{
			deltaTimer->update();
			g_engine->setFrameTime(deltaTimer->getDelta());

#if defined(MCENGINE_SDL_JOYSTICK) && defined(MCENGINE_SDL_JOYSTICK_MOUSE)

			// joystick mouse
			{
				// apply deadzone
				float joystick0XPercent = m_fJoystick0XPercent;
				float joystick0YPercent = m_fJoystick0YPercent;
				{
					const float joystick0DeadzoneX = sdl_joystick0_deadzone.getFloat();
					const float joystick0DeadzoneY = sdl_joystick0_deadzone.getFloat();

					if (joystick0DeadzoneX > 0.0f && joystick0DeadzoneX < 1.0f)
					{
						const float deltaAbs = (std::abs(m_fJoystick0XPercent) - joystick0DeadzoneX);
						joystick0XPercent = (deltaAbs > 0.0f ? (deltaAbs / (1.0f - joystick0DeadzoneX)) * (float)sign<float>(m_fJoystick0XPercent) : 0.0f);
					}

					if (joystick0DeadzoneY > 0.0f && joystick0DeadzoneY < 1.0f)
					{
						const float deltaAbs = (std::abs(m_fJoystick0YPercent) - joystick0DeadzoneY);
						joystick0YPercent = (deltaAbs > 0.0f ? (deltaAbs / (1.0f - joystick0DeadzoneY)) * (float)sign<float>(m_fJoystick0YPercent) : 0.0f);
					}
				}

				if (g_bHasFocus && !g_bMinimized && (joystick0XPercent != 0.0f || joystick0YPercent != 0.0f))
				{
					const float hardcodedMultiplier = 1000.0f;
					const Vector2 hardcodedResolution = Vector2(1280, 720);
					Vector2 joystickDelta = Vector2(joystick0XPercent * sdl_joystick_mouse_sensitivity.getFloat(), joystick0YPercent * sdl_joystick_mouse_sensitivity.getFloat()) * g_engine->getFrameTime() * hardcodedMultiplier;
					joystickDelta *= g_engine->getScreenSize().x/hardcodedResolution.x > g_engine->getScreenSize().y/hardcodedResolution.y ?
									 g_engine->getScreenSize().y/hardcodedResolution.y : g_engine->getScreenSize().x/hardcodedResolution.x; // normalize

					mousePos += joystickDelta;
					mousePos.x = clamp<float>(mousePos.x, 0.0f, g_engine->getScreenSize().x - 1);
					mousePos.y = clamp<float>(mousePos.y, 0.0f, g_engine->getScreenSize().y - 1);

					environment->setMousePos(mousePos.x, mousePos.y);
					g_engine->getMouse()->onPosChange(mousePos);
				}
			}
#endif

			if (g_bUpdate)
				g_engine->onUpdate();
		}

		// draw
		if (g_bDraw)
		{
			g_bDrawing = true;
			{
				g_engine->onPaint();
			}
			g_bDrawing = false;
		}

		// delay the next frame
		{
			VPROF_BUDGET("FPSLimiter", VPROF_BUDGETGROUP_SLEEP);

			frameTimer->update();
			const bool inBackground = g_bMinimized || !g_bHasFocus;
			if ((!fps_unlimited.getBool() && fps_max.getInt() > 0) || inBackground)
			{
				double delayStart = frameTimer->getElapsedTime();
				double delayTime;
				if (inBackground)
					delayTime = (1.0 / (double)fps_max_background.getFloat()) - frameTimer->getDelta();
				else
					delayTime = (1.0 / (double)fps_max.getFloat()) - frameTimer->getDelta();

				const bool didSleep = delayTime > 0.0;
				while (delayTime > 0.0)
				{
					if (inBackground) // real waiting (very inaccurate, but very good for little background cpu utilization)
						env->sleep((int)((1.0f / fps_max_background.getFloat())*1000.0f*1000.0f));
					else // more or less "busy" waiting, but giving away the rest of the timeslice at least
						env->sleep(0); // yes, there is a zero in there

					// decrease the delayTime by the time we spent in this loop
					// if the loop is executed more than once, note how delayStart now gets the value of the previous iteration from getElapsedTime()
					// this works because the elapsed time is only updated in update(). now we can easily calculate the time the Sleep() took and subtract it from the delayTime
					delayStart = frameTimer->getElapsedTime();
					frameTimer->update();
					delayTime -= (frameTimer->getElapsedTime() - delayStart);
				}

				if (!didSleep && fps_max_yield.getBool())
					env->sleep(0); // yes, there is a zero in there
			}
			else if (fps_unlimited_yield.getBool())
				env->sleep(0); // yes, there is a zero in there
		}
	}

	// release the timers
	SAFE_DELETE(frameTimer);
	SAFE_DELETE(deltaTimer);

    // release engine
    SAFE_DELETE(g_engine);

    // and the opengl context
#if defined(MCENGINE_FEATURE_OPENGL) || defined(MCENGINE_FEATURE_OPENGLES)

	SDL_GL_DeleteContext(context);

#endif

	// finally, destroy the window
	SDL_DestroyWindow(g_window);
	SDL_Quit();

	// TODO: handle potential restart

	return 0;
}



int mainSDL(int argc, char *argv[])
{
	return mainSDL(argc, argv, NULL);
}

#endif
