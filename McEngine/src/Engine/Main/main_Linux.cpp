#ifdef __linux__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <wayland-client.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include "Engine.h"
#include "LinuxEnvironment.h"
#include "ConVar.h"

Engine *g_engine = NULL;
LinuxEnvironment *g_environment = NULL;

ConVar fps_max("fps_max", 60);
ConVar fps_max_background("fps_max_background", 30);
ConVar fps_unlimited("fps_unlimited", false);

bool g_bRunning = true;

int mainX11(int argc, char *argv[], Display *display);
int mainWayland(int argc, char *argv[], struct wl_display *display);

int main(int argc, char *argv[])
{
	if (getenv("MCENGINE_X11") == NULL)
	{
		struct wl_display *wlDisplay = wl_display_connect(NULL);
		if (wlDisplay != NULL)
			return mainWayland(argc, argv, wlDisplay);
		else
			printf("wl_display_connect() failed, falling back to X11\n");
	}

	Display *x11Display = XOpenDisplay(NULL);
	if (x11Display != NULL)
		return mainX11(argc, argv, x11Display);
	else
		printf("FATAL ERROR: XOpenDisplay() can't connect to X server!\n\n");

	return 1;
}

#endif
