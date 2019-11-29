#ifdef __linux__

#include <X11/X.h>
#include <X11/Xlib.h>

int mainX11(int argc, char *argv[], Display *display);

int main(int argc, char *argv[])
{
	Display *dpy = XOpenDisplay(NULL);
	mainX11(argc, argv, dpy);
}

#endif
