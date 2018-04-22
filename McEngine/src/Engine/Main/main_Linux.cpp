//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		main entry point
//
// $NoKeywords: $main
//===============================================================================//

#ifdef __linux__

#include <stdio.h>
#include <stdlib.h>

#include "Engine.h"
#include "ConVar.h"
#include "Timer.h"

#include "LinuxGLLegacyInterface.h"
#include "LinuxEnvironment.h"

#define XLIB_ILLEGAL_ACCESS
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>

#include "OpenGLHeaders.h"

#include <unistd.h>

#define WINDOW_TITLE "McEngine"

// TODO: this is incorrect, the size here doesn't take the decorations into account
#define WINDOW_WIDTH (1280)
#define WINDOW_HEIGHT (720)

#define WINDOW_WIDTH_MIN 100
#define WINDOW_HEIGHT_MIN 100

Engine *g_engine = NULL;
LinuxEnvironment *g_environment = NULL;

bool g_bRunning = true;
bool g_bUpdate = true;
bool g_bDraw = true;
bool g_bDrawing = false;

bool g_bHasFocus = false; // for fps_max_background

ConVar fps_max("fps_max", 60);
ConVar fps_max_background("fps_max_background", 30);
ConVar fps_unlimited("fps_unlimited", false);

Display                 *dpy;
Window                  root;
//GLint                   att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
Colormap                cmap;
XSetWindowAttributes    swa;
Window                  win;
GLXContext              glc;
XWindowAttributes       gwa;
XEvent                  xev;

// input
XIM im;
XIC ic;
int xi2opcode;



//****************//
//	Message loop  //
//****************//

void WndProc(int type, int xcookieType, int xcookieExtension)
{
	switch (type)
	{
	case ConfigureNotify:
		if (g_engine != NULL)
			g_engine->requestResolutionChange(Vector2(xev.xconfigure.width, xev.xconfigure.height));
		break;

	case KeymapNotify:
		XRefreshKeyboardMapping(&xev.xmapping);
		break;

	case FocusIn:
		g_bHasFocus = true;
		if (g_bRunning && g_engine != NULL)
		{
			g_engine->onFocusGained();
		}
		break;

	case FocusOut:
		g_bHasFocus = false;
		if (g_bRunning && g_engine != NULL)
		{
			g_engine->onFocusLost();
		}
		break;

	// clipboard
	case SelectionRequest:
		if (g_environment != NULL)
			g_environment->handleSelectionRequest(xev.xselectionrequest);
		break;

	// keyboard
	case KeyPress:
		{
			XKeyEvent *ke = &xev.xkey;
			int key = XLookupKeysym(ke, /*(ke->state&ShiftMask) ? 1 : 0*/1); // WARNING: these two must be the same (see below)
			//printf("X: keyPress = %i\n", key);

			if (g_engine != NULL)
			{
				g_engine->onKeyboardKeyDown(key);
			}

			const int buffSize = 20;
			char buf[buffSize];
			Status status = 0;
			KeySym keysym = 0;
			int length = Xutf8LookupString(ic, (XKeyPressedEvent*)&xev.xkey, buf, buffSize, &keysym, &status);

			if (length > 0)
			{
				buf[buffSize-1] = 0;
				//printf("buff = %s\n", buf);
				if (g_engine != NULL)
					g_engine->onKeyboardChar(buf[0]);
			}
		}
		break;

	case KeyRelease:
		{
			XKeyEvent *ke = &xev.xkey;
			int key = XLookupKeysym(ke, /*(ke->state & ShiftMask) ? 1 : 0*/1); // WARNING: these two must be the same (see above)
			//printf("X: keyRelease = %i\n", key);

			if (g_engine != NULL)
			{
				// LINUX: fuck X11 key repeats with release events inbetween
				if (XEventsQueued(dpy, QueuedAfterReading))
				{
					XEvent nextEvent;
					XPeekEvent(dpy, &nextEvent);
					if (nextEvent.type == KeyPress && nextEvent.xkey.time == ke->time && nextEvent.xkey.keycode == ke->keycode)
						break; // throw the event away
				}

				g_engine->onKeyboardKeyUp(key);
			}
		}
		break;

	// mouse
	case ButtonPress:
		if (g_engine != NULL)
		{
			switch (xev.xbutton.button)
			{
			case Button1:
				g_engine->onMouseLeftChange(true);
				break;
			case Button2:
				g_engine->onMouseMiddleChange(true);
				break;
			case Button3:
				g_engine->onMouseRightChange(true);
				break;

			case Button4: // = mouse wheel up
				g_engine->onMouseWheelVertical(120);
				break;
			case Button5: // = mouse wheel down
				g_engine->onMouseWheelVertical(-120);
				break;
			}
		}
		break;

	case ButtonRelease:
		if (g_engine != NULL)
		{
			switch (xev.xbutton.button)
			{
			case Button1:
				g_engine->onMouseLeftChange(false);
				break;
			case Button2:
				g_engine->onMouseMiddleChange(false);
				break;
			case Button3:
				g_engine->onMouseRightChange(false);
				break;
			}
		}
		break;

	// destroy
	case ClientMessage:
		if (g_engine != NULL)
			g_engine->onShutdown();
		g_bRunning = false;
		break;
	}

	// XInput2
	if (xcookieType == GenericEvent && xcookieExtension == xi2opcode)
	{
		if (XGetEventData(dpy, &xev.xcookie))
		{
			switch(xev.xcookie.evtype)
			{
			case XI_RawMotion:
				{
					const XIRawEvent *rawev = (const XIRawEvent*)xev.xcookie.data;

					int rawX = 0;
					int rawY = 0;

					if (XIMaskIsSet(rawev->valuators.mask, 0))
						rawX = rawev->valuators.values[0];
					if (XIMaskIsSet(rawev->valuators.mask, 1))
						rawY = rawev->valuators.values[1];

					// TODO: apparently this is in 0-65536 units, convert properly
					// how to know whether the value is for an absolute input device, or a relative input device?
					// if even relative input devices generate 0-65536 values, then we have to "wait" 1 event to calculate a delta?
					//printf("x = %i, y = %i\n", rawX, rawY);
					//if (g_engine != NULL)
					//	g_engine->onMouseRawMove(rawX, rawY);
				}
				break;
			}

			XFreeEventData(dpy, &xev.xcookie);
		}
	}
}



//********************//
//	Main entry point  //
//********************//

int main(int argc, char *argv[])
{
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL)
	{
		printf("FATAL ERROR: XOpenDisplay() can't connect to X server!\n\n");
        exit(1);
	}


	// before we do anything, check if XInput is available (for raw mouse input, smooth horizontal & vertical mouse wheel etc.)
	int xi2firstEvent, xi2error;
	if (!XQueryExtension(dpy, "XInputExtension", &xi2opcode, &xi2firstEvent, &xi2error))
	{
		printf("FATAL ERROR: XQueryExtension() XInput extension not available!\n\n");
		exit(1);
	}

	// want version 2 at least
	int ximajor = 2, ximinor = 0;
	if (XIQueryVersion(dpy, &ximajor, &ximinor) == BadRequest)
	{
		printf("FATAL ERROR: XIQueryVersion() XInput2 not available, server supports only %d.%d!\n\n", ximajor, ximinor);
		exit(1);
	}


	root = DefaultRootWindow(dpy);

	XVisualInfo *vi = getVisualInfo(dpy);
	cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);

	// set window attributes
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | FocusChangeMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | KeymapStateMask;

	Screen *defaultScreen = DefaultScreenOfDisplay(dpy);
	win = XCreateWindow(dpy, root, defaultScreen->width/2 - WINDOW_WIDTH/2, defaultScreen->height/2 - WINDOW_HEIGHT/2, WINDOW_WIDTH, WINDOW_HEIGHT, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);

	// window resize limit
	XSizeHints* winSizeHints = XAllocSizeHints();
	if (!winSizeHints)
	{
	    printf("FATAL ERROR: XAllocSizeHints() out of memory!\n\n");
	    exit(1);
	}
	winSizeHints->flags = PMinSize;
	winSizeHints->min_width = WINDOW_WIDTH_MIN;
	winSizeHints->min_height = WINDOW_HEIGHT_MIN;
	XSetWMNormalHints(dpy, win, winSizeHints);
	XFree(winSizeHints);

	// register custom delete message for quitting the engine (received as ClientMessage in the main loop)
	Atom wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", false);
	XSetWMProtocols(dpy, win, &wm_delete_window, 1);

	// make window visible & set title
	XMapWindow(dpy, win);
	XStoreName(dpy, win, WINDOW_TITLE);

	// after the window is visible, center it again (if the window manager ignored the position of the window in XCreateWindow(), because fuck you)
	XMoveWindow(dpy, win, defaultScreen->width/2 - WINDOW_WIDTH/2, defaultScreen->height/2 - WINDOW_HEIGHT/2);

	// get input method
	im = XOpenIM(dpy, NULL, NULL, NULL);
	if (im == NULL)
	{
		printf("FATAL ERROR: XOpenIM() couldn't open input method!\n\n");
		exit(1);
	}

	// get input context
	ic = XCreateIC(im, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, win, NULL);
	if (ic == NULL)
	{
		printf("FATAL ERROR: XCreateIC() couldn't create input context!\n\n");
		exit(1);
	}


	// set XInput event masks
	XIEventMask masks[1];
	unsigned char mask[(XI_LASTEVENT + 7)/8];
	memset(mask, 0, sizeof(mask));

	XISetMask(mask, XI_RawMotion);

	// use client pointer for raw input, instead of XIAllMasterDevices.
	// otherwise, the implicit grab on clicks will not give any more events as long as the button is being pressed
	int pointerDevId;
	XIGetClientPointer(dpy, None, &pointerDevId);

	masks[0].mask_len = sizeof(mask);
	masks[0].mask = mask;
	masks[0].deviceid = /*XIAllMasterDevices*/ pointerDevId;

	// and select it on the window
	XISelectEvents(dpy, DefaultRootWindow(dpy), masks, 1);
	XFlush(dpy);


	// get keyboard focus
	XSetICFocus(ic);

    // initialize engine
	LinuxEnvironment *environment = new LinuxEnvironment(dpy, win);
	g_environment = environment;
    g_engine = new Engine(environment, argc > 1 ? argv[1] : "");
    g_engine->loadApp();

    // create timer
    Timer *frameTimer = new Timer();
    frameTimer->start();
    frameTimer->update();

    Timer *deltaTimer = new Timer();
    deltaTimer->start();
    deltaTimer->update();

    // main loop
	while (g_bRunning)
	{
		// handle window message queue
		while (XPending(dpy))
		{
			XNextEvent(dpy, &xev);

			if (XFilterEvent(&xev, win)) // for keyboard chars
				continue;

			WndProc(xev.type, xev.xcookie.type, xev.xcookie.extension);
		}

		// update
		if (g_bUpdate)
			g_engine->onUpdate();

		// draw
		if (g_bDraw)
		{
			g_bDrawing = true;
				g_engine->onPaint();
			g_bDrawing = false;
		}

		// delay the next frame
		frameTimer->update();
		deltaTimer->update();

		engine->setFrameTime(deltaTimer->getDelta());

		const bool inBackground = /*g_bMinimized ||*/ !g_bHasFocus;
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
					usleep(1000 * (unsigned int)((1.0f / fps_max_background.getFloat())*1000.0f));
				else // more or less "busy" waiting, but giving away the rest of the timeslice at least
					usleep(1);

				// decrease the delayTime by the time we spent in this loop
				// if the loop is executed more than once, note how delayStart now gets the value of the previous iteration from getElapsedTime()
				// this works because the elapsed time is only updated in update(). now we can easily calculate the time the Sleep() took and subtract it from the delayTime
				delayStart = frameTimer->getElapsedTime();
				frameTimer->update();
				delayTime -= (frameTimer->getElapsedTime() - delayStart);
			}
		}
    }

	bool isRestartScheduled = environment->isRestartScheduled();

	// release the engine
	SAFE_DELETE(g_engine);

	// destroy the window
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);

	// handle potential restart
	if (isRestartScheduled)
	{
		char buf[4096];
		memset(buf, '\0', 4096);
		if (readlink("/proc/self/exe", buf, 4095) != -1)
		{
			if (fork() == 0)
			{
				char *argv[] = {buf, 0};
				char *envp[] = {0};
				execve(buf, (char * const *)argv, (char * const *)envp);

				// TODO: not finished yet, not working
			}
		}
	}

	return 0;
}

#endif
