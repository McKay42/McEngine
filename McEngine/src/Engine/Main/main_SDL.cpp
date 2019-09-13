//================ Copyright (c) 2018, PG, All rights reserved. =================//
//
// Purpose:		main entry point
//
// $NoKeywords: $main
//===============================================================================//

#include "cbase.h"

#ifdef MCENGINE_FEATURE_SDL

//#define MCENGINE_SDL_JOYSTICK
//#define MCENGINE_SDL_JOYSTICK_MOUSE

#if !defined(MCENGINE_FEATURE_OPENGL) && !defined(MCENGINE_FEATURE_OPENGLES)
#error OpenGL support is currently required for SDL
#endif

#include "SDL.h"

#include "Engine.h"
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

ConVar fps_max("fps_max", 60.0f);
ConVar fps_max_background("fps_max_background", 30.0f);
ConVar fps_unlimited("fps_unlimited", false);

ConVar sdl_joystick_mouse_sensitivity("sdl_joystick_mouse_sensitivity", 1.0f);

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

	// NOTE: hardcoded to OpenGL ES 2.0 currently
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

	g_engine = new Engine(environment, ""); // TODO: args
	g_engine->loadApp();

	frameTimer->update();
	deltaTimer->update();

	Vector2 mousePos;

	// main loop
	SDL_Event e;
	while (g_bRunning)
	{

		// HACKHACK: switch hack (mouse/keyboard)
#ifdef __SWITCH__

		HorizonSDLEnvironment *horizonSDLenv = dynamic_cast<HorizonSDLEnvironment*>(environment);
		if (horizonSDLenv != NULL)
			horizonSDLenv->update_before_winproc();

#endif

		// handle window message queue
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
						g_engine->onKeyboardChar(nullTerminatedTextInputString[i]);
					}
				}
				break;

			// mouse, also inject mouse 4 + 5 as keyboard keys
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
					g_engine->onKeyboardKeyDown(SDL_Scancode::SDL_NUM_SCANCODES + 1); // NOTE: abusing enum value
					break;
				case SDL_BUTTON_X2:
					g_engine->onKeyboardKeyDown(SDL_Scancode::SDL_NUM_SCANCODES + 2); // NOTE: abusing enum value
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
					g_engine->onKeyboardKeyUp(SDL_Scancode::SDL_NUM_SCANCODES + 1); // NOTE: abusing enum value
					break;
				case SDL_BUTTON_X2:
					g_engine->onKeyboardKeyUp(SDL_Scancode::SDL_NUM_SCANCODES + 2); // NOTE: abusing enum value
					break;
				}
				break;

			case SDL_MOUSEWHEEL:
				if (e.wheel.x != 0)
					g_engine->onMouseWheelHorizontal(e.wheel.x > 0 ? 120 : -120);
				if (e.wheel.y != 0)
					g_engine->onMouseWheelVertical(e.wheel.y > 0 ? 120 : -120);
				break;

			case SDL_MOUSEMOTION:
				if (SDL_GetRelativeMouseMode() == SDL_TRUE)
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
			// HACKHACK: currently hardcoded for nintendo switch (rewrite if joystick support for other platforms)
#ifdef MCENGINE_SDL_JOYSTICK

			case SDL_JOYBUTTONDOWN:
				///debugLog("joybuttondown: %i button %i down\n", (int)e.jbutton.which, (int)e.jbutton.button);
				if (e.jbutton.button == 0) // KEY_A
				{
					g_engine->onMouseLeftChange(true);

					if (engine->getConsoleBox()->isActive())
					{
						g_engine->onKeyboardKeyDown(SDL_SCANCODE_RETURN);
						g_engine->onKeyboardKeyUp(SDL_SCANCODE_RETURN);
					}
				}
				else if (e.jbutton.button == 10 || e.jbutton.button == 1) // KEY_PLUS || KEY_B
					g_engine->onKeyboardKeyDown(SDL_SCANCODE_ESCAPE);
				else if (e.jbutton.button == 2) // KEY_X
				{
					g_engine->onKeyboardKeyDown(SDL_SCANCODE_X);
					xDown = true;
				}
				else if (e.jbutton.button == 3) // KEY_Y
					g_engine->onKeyboardKeyDown(SDL_SCANCODE_Y);
				else if (e.jbutton.button == 21 || e.jbutton.button == 13) // right stick up || dpad up
					g_engine->onKeyboardKeyDown(SDL_SCANCODE_UP);
				else if (e.jbutton.button == 23 || e.jbutton.button == 15) // right stick down || dpad down
					g_engine->onKeyboardKeyDown(SDL_SCANCODE_DOWN);
				else if (e.jbutton.button == 20 || e.jbutton.button == 12) // right stick left || dpad left
					g_engine->onKeyboardKeyDown(SDL_SCANCODE_LEFT);
				else if (e.jbutton.button == 22 || e.jbutton.button == 14) // right stick right || dpad right
					g_engine->onKeyboardKeyDown(SDL_SCANCODE_RIGHT);
				else if (e.jbutton.button == 6) // KEY_L
					g_engine->onKeyboardKeyDown(SDL_SCANCODE_L);
				else if (e.jbutton.button == 7) // KEY_R
					g_engine->onKeyboardKeyDown(SDL_SCANCODE_R);
				else if (e.jbutton.button == 8) // KEY_ZL
					g_engine->onKeyboardKeyDown(SDL_SCANCODE_Z);
				else if (e.jbutton.button == 9) // KEY_ZR
					g_engine->onKeyboardKeyDown(SDL_SCANCODE_V);
				else if (e.jbutton.button == 11) // KEY_MINUS
					g_engine->onKeyboardKeyDown(SDL_SCANCODE_F1);
				else if (e.jbutton.button == 4) // left stick press
				{
					// toggle options (CTRL + O)
					g_engine->onKeyboardKeyDown(SDL_SCANCODE_LCTRL);
					g_engine->onKeyboardKeyDown(SDL_SCANCODE_O);
					g_engine->onKeyboardKeyUp(SDL_SCANCODE_LCTRL);
					g_engine->onKeyboardKeyUp(SDL_SCANCODE_O);
				}
				else if (e.jbutton.button == 5) // right stick press
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
				else if (e.jbutton.button == 10 || e.jbutton.button == 1) // KEY_PLUS || KEY_B
					g_engine->onKeyboardKeyUp(SDL_SCANCODE_ESCAPE);
				else if (e.jbutton.button == 2) // KEY_X
				{
					g_engine->onKeyboardKeyUp(SDL_SCANCODE_X);
					xDown = false;
				}
				else if (e.jbutton.button == 3) // KEY_Y
					g_engine->onKeyboardKeyUp(SDL_SCANCODE_Y);
				else if (e.jbutton.button == 21 || e.jbutton.button == 13) // right stick up || dpad up
					g_engine->onKeyboardKeyUp(SDL_SCANCODE_UP);
				else if (e.jbutton.button == 23 || e.jbutton.button == 15) // right stick down || dpad down
					g_engine->onKeyboardKeyUp(SDL_SCANCODE_DOWN);
				else if (e.jbutton.button == 20 || e.jbutton.button == 12) // right stick left || dpad left
					g_engine->onKeyboardKeyUp(SDL_SCANCODE_LEFT);
				else if (e.jbutton.button == 22 || e.jbutton.button == 14) // right stick right || dpad right
					g_engine->onKeyboardKeyUp(SDL_SCANCODE_RIGHT);
				else if (e.jbutton.button == 6) // KEY_L
					g_engine->onKeyboardKeyUp(SDL_SCANCODE_L);
				else if (e.jbutton.button == 7) // KEY_R
					g_engine->onKeyboardKeyUp(SDL_SCANCODE_R);
				else if (e.jbutton.button == 8) // KEY_ZL
					g_engine->onKeyboardKeyUp(SDL_SCANCODE_Z);
				else if (e.jbutton.button == 9) // KEY_ZR
					g_engine->onKeyboardKeyUp(SDL_SCANCODE_V);
				else if (e.jbutton.button == 11) // KEY_MINUS
					g_engine->onKeyboardKeyUp(SDL_SCANCODE_F1);
				break;

			case SDL_JOYAXISMOTION:
				///debugLog("joyaxismotion: stick %i : axis = %i, value = %i\n", (int)e.jaxis.which, (int)e.jaxis.axis, (int)e.jaxis.value);
				// left stick
				if (e.jaxis.axis == 1 || e.jaxis.axis == 0)
				{
					if (e.jaxis.axis == 0)
						m_fJoystick0XPercent = (float)e.jaxis.value / 32767.0f;
					else
						m_fJoystick0YPercent = (float)e.jaxis.value / 32767.0f;
				}
				break;

#endif
			}
		}

		// update
		{
			deltaTimer->update();
			g_engine->setFrameTime(deltaTimer->getDelta());

#if defined(MCENGINE_SDL_JOYSTICK) && defined(MCENGINE_SDL_JOYSTICK_MOUSE)

			// joystick mouse
			if (m_fJoystick0XPercent != 0.0f || m_fJoystick0YPercent != 0.0f)
			{
				const float hardcodedMultiplier = 1000.0f;
				const Vector2 hardcodedResolution = Vector2(1280, 720);
				Vector2 joystickDelta = Vector2(m_fJoystick0XPercent * sdl_joystick_mouse_sensitivity.getFloat(), m_fJoystick0YPercent * sdl_joystick_mouse_sensitivity.getFloat()) * g_engine->getFrameTime() * hardcodedMultiplier;
				joystickDelta *= g_engine->getScreenSize().x/hardcodedResolution.x > g_engine->getScreenSize().y/hardcodedResolution.y ?
								 g_engine->getScreenSize().y/hardcodedResolution.y : g_engine->getScreenSize().x/hardcodedResolution.x; // normalize

				mousePos += joystickDelta;
				mousePos.x = clamp<float>(mousePos.x, 0.0f, g_engine->getScreenSize().x);
				mousePos.y = clamp<float>(mousePos.y, 0.0f, g_engine->getScreenSize().y);

				environment->setMousePos(mousePos.x, mousePos.y);
				g_engine->getMouse()->onPosChange(mousePos);
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
		frameTimer->update();
		const bool inBackground = g_bMinimized || !g_bHasFocus;
		if (!fps_unlimited.getBool() || inBackground)
		{
			double delayStart = frameTimer->getElapsedTime();
			double delayTime;
			if (inBackground)
				delayTime = (1.0 / (double)fps_max_background.getFloat()) - frameTimer->getDelta();
			else
				delayTime = (1.0 / (double)fps_max.getFloat()) - frameTimer->getDelta();

			while (delayTime > 0.0)
			{
				if (inBackground) // real waiting (very inaccurate, but very good for little background cpu utilization)
					frameTimer->sleep((int)((1.0f / fps_max_background.getFloat())*1000.0f*1000.0f));
				else // more or less "busy" waiting, but giving away the rest of the timeslice at least
					frameTimer->sleep(0); // yes, there is a zero in there

				// decrease the delayTime by the time we spent in this loop
				// if the loop is executed more than once, note how delayStart now gets the value of the previous iteration from getElapsedTime()
				// this works because the elapsed time is only updated in update(). now we can easily calculate the time the Sleep() took and subtract it from the delayTime
				delayStart = frameTimer->getElapsedTime();
				frameTimer->update();
				delayTime -= (frameTimer->getElapsedTime() - delayStart);
			}
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
