//================ Copyright (c) 2018, PG, All rights reserved. =================//
//
// Purpose:		sdl implementation of main_OSX
//
// $NoKeywords: $macsdl
//===============================================================================//

// TODO: use new SDLEnvironment here, subclass it with implementations for file IO etc.

// building notes:
// copy SDL2.framework into /Library/Frameworks/ to be able to build.
// modify SDL2[.dylib], extracted from SDL2.framework, copied as SDL2.dylib into engine working directory:
// > install_name_tool -id "@loader_path/SDL2.dylib" SDL2.dylib
// modify McEngine executable:
// > install_name_tool -change @rpath/SDL2.framework/Versions/A/SDL2 @loader_path/SDL2.dylib McEngine

#ifdef __APPLE__

#include "main_OSX_cpp.h"
#include "Engine.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "ConVar.h"

#include "SDL.h"



#define WINDOW_TITLE "McEngine"

#define WINDOW_WIDTH (1280)
#define WINDOW_HEIGHT (720)

#define WINDOW_WIDTH_MIN 100
#define WINDOW_HEIGHT_MIN 100



extern bool g_bRunning;
extern Engine *g_engine;

SDL_Window* window = NULL;



// reverse wrapper

const char *_prevClipboardTextSDL = NULL;
const char *MacOSWrapper::getClipboardText()
{
	if (_prevClipboardTextSDL != NULL)
		SDL_free((void*)_prevClipboardTextSDL);

	_prevClipboardTextSDL = SDL_GetClipboardText();
	return (_prevClipboardTextSDL != NULL ? _prevClipboardTextSDL : "");
}

void MacOSWrapper::setClipboardText(const char *text)
{
	SDL_SetClipboardText(text);
}

void MacOSWrapper::showMessageInfo(const char *title, const char *message)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title, message, window);
}

void MacOSWrapper::showMessageWarning(const char *title, const char *message)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, title, message, window);
}

void MacOSWrapper::showMessageError(const char *title, const char *message)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, message, window);
}

void MacOSWrapper::showMessageErrorFatal(const char *title, const char *message)
{
	showMessageError(title, message);
}

void MacOSWrapper::focus()
{
	SDL_RaiseWindow(window);
}

void MacOSWrapper::center()
{
	// TODO: use nearest monitor
	VECTOR2 screenSize = getNativeScreenSize();
	VECTOR2 windowSize = getWindowSize();
	setWindowPos(screenSize.x/2 - windowSize.x/2, screenSize.y/2 - windowSize.y/2);
}

void MacOSWrapper::minimize()
{
	SDL_MinimizeWindow(window);
}

void MacOSWrapper::maximize()
{
	SDL_MaximizeWindow(window);
}

void MacOSWrapper::enableFullscreen()
{
	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void MacOSWrapper::disableFullscreen()
{
	SDL_SetWindowFullscreen(window, 0);
}

void MacOSWrapper::setWindowTitle(const char *title)
{
	SDL_SetWindowTitle(window, title);
}

void MacOSWrapper::setWindowPos(int x, int y)
{
	SDL_SetWindowPosition(window, x, y);
}

void MacOSWrapper::setWindowSize(int width, int height)
{
	SDL_SetWindowSize(window, width, height);
}

void MacOSWrapper::setWindowResizable(bool resizable)
{
	SDL_SetWindowResizable(window, resizable ? SDL_TRUE : SDL_FALSE);
}

MacOSWrapper::VECTOR2 MacOSWrapper::getWindowPos()
{
	int x = 0;
	int y = 0;
	SDL_GetWindowPosition(window, &x, &y);
	return {(float)x, (float)y};
}

MacOSWrapper::VECTOR2 MacOSWrapper::getWindowSize()
{
	int width = WINDOW_WIDTH_MIN;
	int height = WINDOW_HEIGHT_MIN;
	SDL_GetWindowSize(window, &width, &height);
	return {(float)width, (float)height};
}

int MacOSWrapper::getMonitor()
{
	// TODO
	return 0;
}

MacOSWrapper::VECTOR2 MacOSWrapper::getNativeScreenSize()
{
	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(MacOSWrapper::getMonitor(), &dm);
	return {(float)dm.w, (float)dm.h};
}

MacOSWrapper::VECTOR2 MacOSWrapper::getMousePos()
{
	int x = 0;
	int y = 0;
	SDL_GetMouseState(&x, &y);
	if (g_engine != NULL)
		return {(float)x, ((float)g_engine->getScreenHeight() - (float)y + 1)}; // see MacOSEnvironment::getMousePos()
	else
		return {(float)x, (float)y};
}

void MacOSWrapper::setCursor(int cursor)
{
	// TODO:
}

void MacOSWrapper::setCursorVisible(bool visible)
{
	SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
}

void MacOSWrapper::setMousePos(int x, int y)
{
	SDL_WarpMouseInWindow(window, x, y);
}

void MacOSWrapper::setCursorClip(bool clip)
{
	SDL_SetWindowGrab(window, clip ? SDL_TRUE : SDL_FALSE);
}

void MacOSWrapper::endScene()
{
	SDL_GL_SwapWindow(window);
}

void MacOSWrapper::setVSync(bool vsync)
{
	if (SDL_GL_SetSwapInterval(vsync ? 1 : -1) < 0 && !vsync)
		SDL_GL_SetSwapInterval(0);
}



int main(int argc, char* argv[])
{
	// initialize sdl
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, "Couldn't initialize SDL2: %s\n", SDL_GetError());
		return 1;
	}

	// create window
	window = SDL_CreateWindow(
			WINDOW_TITLE,
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			WINDOW_WIDTH, WINDOW_HEIGHT,
			SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_GRABBED | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_FOREIGN | SDL_WINDOW_OPENGL
	);
	if (window == NULL)
	{
		fprintf(stderr, "Couldn't create window: %s\n", SDL_GetError());
		return 1;
	}

	// settings
	SDL_SetWindowMinimumSize(window, WINDOW_WIDTH_MIN, WINDOW_HEIGHT_MIN);

	// create OpenGL context
	SDL_GLContext context = SDL_GL_CreateContext(window);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);

	SDL_RaiseWindow(window);

	MacOSWrapper *wrapper = new MacOSWrapper();
	{
		// framework overrides
		convar->getConVarByName("minimize_on_focus_lost_if_fullscreen")->setValue(0.0f);

	}
	wrapper->loadApp();

	SDL_Event e;
	while (g_bRunning)
	{
		wrapper->main_objc_before_winproc();
		{
			// TODO: clear mixed usage of g_engine and wrapper here
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
						wrapper->onFocusGained();
						break;
					case SDL_WINDOWEVENT_FOCUS_LOST:
						wrapper->onFocusLost();
						break;
					case SDL_WINDOWEVENT_MAXIMIZED:
						g_engine->onMaximized();
						break;
					case SDL_WINDOWEVENT_MINIMIZED:
						g_engine->onMinimized();
						break;
					case SDL_WINDOWEVENT_RESTORED:
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
						g_engine->getMouse()->onLeftChange(true);
						break;
					case SDL_BUTTON_MIDDLE:
						g_engine->getMouse()->onMiddleChange(true);
						break;
					case SDL_BUTTON_RIGHT:
						g_engine->getMouse()->onRightChange(true);
						break;
					}
					break;

				case SDL_MOUSEBUTTONUP:
					switch (e.button.button)
					{
					case SDL_BUTTON_LEFT:
						g_engine->getMouse()->onLeftChange(false);
						break;
					case SDL_BUTTON_MIDDLE:
						g_engine->getMouse()->onMiddleChange(false);
						break;
					case SDL_BUTTON_RIGHT:
						g_engine->getMouse()->onRightChange(false);
						break;
					}
					break;

				case SDL_MOUSEWHEEL:
					if (e.wheel.x != 0)
						g_engine->getMouse()->onWheelHorizontal(e.wheel.x > 0 ? 120 : -120);
					if (e.wheel.y != 0)
						g_engine->getMouse()->onWheelVertical(e.wheel.y > 0 ? 120 : -120);
					break;
				}
			}
		}
		wrapper->main_objc_after_winproc();
	}

	delete wrapper;

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

#endif
