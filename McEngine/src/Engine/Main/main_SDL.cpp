//================ Copyright (c) 2018, PG, All rights reserved. =================//
//
// Purpose:		main entry point
//
// $NoKeywords: $main
//===============================================================================//

#include "cbase.h"

#ifdef MCENGINE_FEATURE_SDL

#ifndef MCENGINE_FEATURE_OPENGL
#error OpenGL support is currently required for SDL
#endif

#include "SDL.h"

#include "Engine.h"
#include "Timer.h"
#include "Mouse.h"
#include "ConVar.h"

#include "SDLEnvironment.h"



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



int mainSDL(int argc, char *argv[], SDLEnvironment *customSDLEnvironment)
{
	SDLEnvironment *environment = customSDLEnvironment;

	// initialize sdl
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	// pre window-creation settings
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	// create window
	g_window = SDL_CreateWindow(
			WINDOW_TITLE,
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			WINDOW_WIDTH, WINDOW_HEIGHT,
			SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_FOREIGN | SDL_WINDOW_OPENGL
	);
	if (g_window == NULL)
	{
		fprintf(stderr, "Couldn't create window: %s\n", SDL_GetError());
		return 1;
	}

	// post window-creation settings
	SDL_SetWindowMinimumSize(g_window, WINDOW_WIDTH_MIN, WINDOW_HEIGHT_MIN);

#ifdef MCENGINE_FEATURE_OPENGL

	// create OpenGL context
	SDL_GLContext context = SDL_GL_CreateContext(g_window);

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

	// main loop
	SDL_Event e;
	while (g_bRunning)
	{
		frameTimer->update();

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
			}
		}

		// update
		if (g_bUpdate)
			g_engine->onUpdate();

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
		deltaTimer->update();

		engine->setFrameTime(deltaTimer->getDelta());

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
					frameTimer->sleep((int)((1.0f / fps_max_background.getFloat())*1000.0f));
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

    // finally, destroy the window
	SDL_GL_DeleteContext(context);
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
