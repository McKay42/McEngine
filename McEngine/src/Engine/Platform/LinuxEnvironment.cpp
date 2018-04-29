//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		linux
//
// $NoKeywords: $linuxenv
//===============================================================================//

#ifdef __linux__

#include "LinuxEnvironment.h"
#include "Engine.h"

#include "LinuxGLLegacyInterface.h"
#include "LinuxContextMenu.h"

#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <unistd.h>
#include <pwd.h>

#include <string.h>
#include <stdio.h>

typedef struct
{
	unsigned long   flags;
	unsigned long   functions;
	unsigned long   decorations;
	long            inputMode;
	unsigned long   status;
} Hints;

bool LinuxEnvironment::m_bResizable = true;
std::vector<McRect> LinuxEnvironment::m_vMonitors;

LinuxEnvironment::LinuxEnvironment(Display *display, Window window) : Environment()
{
	m_display = display;
	m_window = window;

	m_bCursorClipped = false;
	m_bCursorRequest = false;
	m_bCursorReset = false;
	m_bCursorVisible = true;
	m_bIsCursorInsideWindow = false;
	m_mouseCursor = XCreateFontCursor(m_display, XC_left_ptr);
	m_invisibleCursor = makeBlankCursor();
	m_cursorType = CURSORTYPE::CURSOR_NORMAL;

	m_atom_UTF8_STRING = XInternAtom(m_display, "UTF8_STRING", False);
	m_atom_CLIPBOARD = XInternAtom(m_display, "CLIPBOARD", False);
	m_atom_TARGETS = XInternAtom(m_display, "TARGETS", False);

	m_bFullScreen = false;

	m_bIsRestartScheduled = false;
	m_bResizeDelayHack = false;
	m_bPrevCursorHack = false;
	m_bFullscreenWasResizable = true;

	// TODO: init monitors
	if (m_vMonitors.size() < 1)
	{
		///debugLog("WARNING: No monitors found! Adding default monitor ...\n");

		const Vector2 windowSize = getWindowSize();
		m_vMonitors.push_back(McRect(0, 0, windowSize.x, windowSize.y));
	}
}

LinuxEnvironment::~LinuxEnvironment()
{
	XFreeCursor(m_display, m_invisibleCursor);
}

void LinuxEnvironment::update()
{
	if (!m_bCursorRequest)
	{
		if (m_bCursorReset)
		{
			m_bCursorReset = false;
			setCursor(CURSOR_NORMAL);
		}
	}
	m_bCursorRequest = false;

	m_bIsCursorInsideWindow = McRect(0, 0, engine->getScreenWidth(), engine->getScreenHeight()).contains(getMousePos());
}

Graphics *LinuxEnvironment::createRenderer()
{
	return new LinuxGLLegacyInterface(m_display, m_window);
}

ContextMenu *LinuxEnvironment::createContextMenu()
{
	return new LinuxContextMenu();
}

Environment::OS LinuxEnvironment::getOS()
{
	return Environment::OS::OS_LINUX;
}

void LinuxEnvironment::shutdown()
{
	XEvent ev;
	memset(&ev, 0, sizeof (ev));

	ev.type = ClientMessage;
	ev.xclient.type = ClientMessage;
	ev.xclient.window = m_window;
	ev.xclient.message_type = XInternAtom(m_display, "WM_PROTOCOLS", True);
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = XInternAtom(m_display, "WM_DELETE_WINDOW", False);
	ev.xclient.data.l[1] = CurrentTime;

	XSendEvent(m_display, m_window, false, NoEventMask, &ev);
}

void LinuxEnvironment::restart()
{
	m_bIsRestartScheduled = true;
	shutdown();
}

void LinuxEnvironment::sleep(unsigned int us)
{
	usleep(us);
}

UString LinuxEnvironment::getExecutablePath()
{
	char buf[4096];
	memset(buf, '\0', 4096);
	if (readlink("/proc/self/exe", buf, 4095) != -1)
		return UString(buf);
	else
		return UString("");
}

void LinuxEnvironment::openURLInDefaultBrowser(UString url)
{
	if (fork() == 0)
		exit(execl("/usr/bin/xdg-open", "xdg-open", url.toUtf8(), (char*)0));
}

UString LinuxEnvironment::getUsername()
{
	passwd *pwd = getpwuid(getuid());
	if (pwd != NULL && pwd->pw_name != NULL)
		return UString(pwd->pw_name);
	else
		return UString("");
}

UString LinuxEnvironment::getUserDataPath()
{
	passwd *pwd = getpwuid(getuid());
	if (pwd != NULL && pwd->pw_dir != NULL)
		return UString(pwd->pw_dir);
	else
		return UString("");
}

bool LinuxEnvironment::fileExists(UString filename)
{
	return std::ifstream(filename.toUtf8()).good();
}

bool LinuxEnvironment::directoryExists(UString directoryName)
{
	DIR *dir = opendir(directoryName.toUtf8());
	if (dir)
	{
		closedir(dir);
		return true;
	}
	else if (ENOENT == errno) // not a directory
	{
	}
	else // something else broke
	{
	}
	return false;
}

bool LinuxEnvironment::createDirectory(UString directoryName)
{
	return mkdir(directoryName.toUtf8(), DEFFILEMODE) != -1;
}

bool LinuxEnvironment::renameFile(UString oldFileName, UString newFileName)
{
	return rename(oldFileName.toUtf8(), newFileName.toUtf8()) != -1;
}

bool LinuxEnvironment::deleteFile(UString filePath)
{
	return remove(filePath.toUtf8()) == 0;
}

std::vector<UString> LinuxEnvironment::getFilesInFolder(UString folder)
{
	std::vector<UString> files;

	struct dirent **namelist;
	int n = scandir(folder.toUtf8(), &namelist, getFilesInFolderFilter, alphasort);
	if (n < 0)
	{
		///debugLog("LinuxEnvironment::getFilesInFolder() error, scandir() returned %i!\n", n);
		return files;
	}

	while (n--)
	{
		const char *name = namelist[n]->d_name;
		UString uName = UString(name);
		UString fullName = folder;
		fullName.append(uName);
		free(namelist[n]);

		struct stat stDirInfo;
		int lstatret = lstat(fullName.toUtf8(), &stDirInfo);
		if (lstatret < 0)
		{
			//perror (name);
			//debugLog("LinuxEnvironment::getFilesInFolder() error, lstat() returned %i!\n", lstatret);
			continue;
		}

		if (!S_ISDIR(stDirInfo.st_mode))
			files.push_back(uName);
	}
	free(namelist);

	return files;
}

std::vector<UString> LinuxEnvironment::getFoldersInFolder(UString folder)
{
	std::vector<UString> folders;

	struct dirent **namelist;
	int n = scandir(folder.toUtf8(), &namelist, getFoldersInFolderFilter, alphasort);
	if (n < 0)
	{
		///debugLog("LinuxEnvironment::getFilesInFolder() error, scandir() returned %i!\n", n);
		return folders;
	}

	while (n--)
	{
		const char *name = namelist[n]->d_name;
		UString uName = UString(name);
		UString fullName = folder;
		fullName.append(uName);
		free(namelist[n]);

		struct stat stDirInfo;
		int lstatret = lstat(fullName.toUtf8(), &stDirInfo);
		if (lstatret < 0)
		{
			///perror (name);
			///debugLog("LinuxEnvironment::getFilesInFolder() error, lstat() returned %i!\n", lstatret);
			continue;
		}

		if (S_ISDIR(stDirInfo.st_mode))
			folders.push_back(uName);
	}
	free(namelist);

	return folders;
}

std::vector<UString> LinuxEnvironment::getLogicalDrives()
{
	std::vector<UString> drives;
	drives.push_back(UString("/"));
	return drives;
}

UString LinuxEnvironment::getFolderFromFilePath(UString filepath)
{
	if (directoryExists(filepath)) // indirect check if this is already a valid directory (and not a file)
		return filepath;
	else
		return UString(dirname((char*)filepath.toUtf8()));
}

UString LinuxEnvironment::getFileExtensionFromFilePath(UString filepath, bool includeDot)
{
	const int idx = filepath.findLast(".");
	if (idx != -1)
		return filepath.substr(idx+1);
	else
		return UString("");
}

UString LinuxEnvironment::getClipBoardText()
{
	return getClipboardTextInt();
}

void LinuxEnvironment::setClipBoardText(UString text)
{
	setClipBoardTextInt(text);
}

void LinuxEnvironment::showMessageInfo(UString title, UString message)
{
	// TODO:
}

void LinuxEnvironment::showMessageWarning(UString title, UString message)
{
	// TODO:
}

void LinuxEnvironment::showMessageError(UString title, UString message)
{
	// TODO:
}

void LinuxEnvironment::showMessageErrorFatal(UString title, UString message)
{
	// TODO:
}

UString LinuxEnvironment::openFileWindow(const char *filetypefilters, UString title, UString initialpath)
{
	// TODO:
	return UString("");
}

UString LinuxEnvironment::openFolderWindow(UString title, UString initialpath)
{
	// TODO:
	return UString("");
}

void LinuxEnvironment::focus()
{
	XRaiseWindow(m_display, m_window);
	XMapRaised(m_display, m_window);
}

void LinuxEnvironment::center()
{
	Vector2 windowSize = getWindowSize();
	if (m_bResizeDelayHack)
		windowSize = m_vResizeHackSize;
	m_bResizeDelayHack = false;

	Screen *defaultScreen = XDefaultScreenOfDisplay(m_display);
	XMoveResizeWindow(m_display, m_window, WidthOfScreen(defaultScreen)/2 - (unsigned int)(windowSize.x/2), HeightOfScreen(defaultScreen)/2 - (unsigned int)(windowSize.y/2), (unsigned int)windowSize.x, (unsigned int)windowSize.y);
}

void LinuxEnvironment::minimize()
{
	XIconifyWindow(m_display, m_window, 0);
}

void LinuxEnvironment::maximize()
{
	XMapWindow(m_display, m_window);

	// set size to fill entire screen (also fill borders)
	// the "x" and "y" members of "attributes" are the window's coordinates relative to its parent, i.e. to the decoration window
	XWindowAttributes attributes;
	XGetWindowAttributes(m_display, m_window, &attributes);
	XMoveResizeWindow(m_display,
			m_window,
			-attributes.x,
			-attributes.y,
			(unsigned int)getNativeScreenSize().x,
			(unsigned int)getNativeScreenSize().y);
}

void LinuxEnvironment::enableFullscreen()
{
	if (m_bFullScreen) return;

	// backup
	if (m_vPrevDisableFullscreenWindowSize != getWindowSize())
	{
		m_vLastWindowPos = getWindowPos();
		m_vLastWindowSize = getWindowSize();
	}

	// handle resizability (force enable while fullscreen)
	m_bFullscreenWasResizable = m_bResizable;
	setWindowResizable(true);

	// disable window decorations
    Hints hints;
    Atom property;
    hints.flags = 2; // specify that we're changing the window decorations
    hints.decorations = 0; // 0 (false) = disable decorations
    property = XInternAtom(m_display, "_MOTIF_WM_HINTS", True);
    XChangeProperty(m_display, m_window, property, property, 32, PropModeReplace, (unsigned char *)&hints, 5);

	// set size to fill entire screen (also fill borders)
	// the "x" and "y" members of "attributes" are the window's coordinates relative to its parent, i.e. to the decoration window
	XWindowAttributes attributes;
	XGetWindowAttributes(m_display, m_window, &attributes);
	XMoveResizeWindow(m_display,
			m_window,
			-attributes.x,
			-attributes.y,
			(unsigned int)getNativeScreenSize().x,
			(unsigned int)getNativeScreenSize().y);

    // suggest fullscreen mode
	Atom atom = XInternAtom(m_display, "_NET_WM_STATE_FULLSCREEN", True);
	XChangeProperty(
		m_display, m_window,
		XInternAtom(m_display, "_NET_WM_STATE", True),
		XA_ATOM, 32, PropModeReplace,
		(unsigned char*)&atom, 1);

	// get identifiers for the provided atom name strings
	Atom wm_state = XInternAtom(m_display, "_NET_WM_STATE", False);
	Atom fullscreen = XInternAtom(m_display, "_NET_WM_STATE_FULLSCREEN", False);

	XEvent xev;
	memset(&xev, 0, sizeof(xev));

	xev.type                 = ClientMessage;
	xev.xclient.window       = m_window;
	xev.xclient.message_type = wm_state;
	xev.xclient.format       = 32;
	xev.xclient.data.l[0]    = 1; // enable fullscreen (1 == true)
	xev.xclient.data.l[1]    = fullscreen;

	// send an event mask to the X-server
	XSendEvent(
		m_display,
		DefaultRootWindow(m_display),
		False,
		SubstructureNotifyMask,
		&xev);

	// force top window
	focus();

	m_bFullScreen = true;
}

void LinuxEnvironment::disableFullscreen()
{
	if (!m_bFullScreen) return;

    // unsuggest fullscreen mode
	Atom atom = XInternAtom(m_display, "_NET_WM_STATE_FULLSCREEN", True);
	XChangeProperty(
		m_display, m_window,
		XInternAtom(m_display, "_NET_WM_STATE", True),
		XA_ATOM, 32, PropModeReplace,
		(unsigned char*)&atom, 1);

	// get identifiers for the provided atom name strings
	Atom wm_state = XInternAtom(m_display, "_NET_WM_STATE", False);
	Atom fullscreen = XInternAtom(m_display, "_NET_WM_STATE_FULLSCREEN", False);

	XEvent xev;
	memset(&xev, 0, sizeof(xev));

	xev.type                 = ClientMessage;
	xev.xclient.window       = m_window;
	xev.xclient.message_type = wm_state;
	xev.xclient.format       = 32;
	xev.xclient.data.l[0]    = 0; // disable fullscreen (0 == false)
	xev.xclient.data.l[1]    = fullscreen;

	// send an event mask to the X-server
	XSendEvent(
		m_display,
		DefaultRootWindow(m_display),
		False,
		SubstructureNotifyMask,
		&xev);

	// enable window decorations
    Hints hints;
    Atom property;
    hints.flags = 2;
    hints.decorations = 1;
    property = XInternAtom(m_display, "_MOTIF_WM_HINTS", True);
    XChangeProperty(m_display, m_window, property, property, 32, PropModeReplace, (unsigned char *)&hints, 5);

	// restore previous size and position
    // NOTE: the y-position is not consistent, the window keeps going down when toggling fullscreen (probably due to decorations), force center() workaround
	XMoveResizeWindow(m_display,
			m_window,
			(int)m_vLastWindowPos.x,
			(int)m_vLastWindowPos.y,
			(unsigned int)m_vLastWindowSize.x,
			(unsigned int)m_vLastWindowSize.y);
	m_vResizeHackSize = m_vLastWindowSize;
	m_bResizeDelayHack = true;

	// update resizability with new resolution
	setWindowResizableInt(m_bFullscreenWasResizable, m_vLastWindowSize);

	center();

	m_vPrevDisableFullscreenWindowSize = getWindowSize();
	m_bFullScreen = false;
}

void LinuxEnvironment::setWindowTitle(UString title)
{
	XStoreName(m_display, m_window, title.toUtf8());
}

void LinuxEnvironment::setWindowPos(int x, int y)
{
	XMapWindow(m_display, m_window);
	XMoveWindow(m_display, m_window, x, y);
}

void LinuxEnvironment::setWindowSize(int width, int height)
{
	// due to the way resizability works, we have to temporarily disable it to be able to resize the window (because min/max is fixed)
	const Vector2 windowPos = getWindowPos();
	const bool wasWindowResizable = m_bResizable;
	if (!wasWindowResizable)
		setWindowResizableInt(true, Vector2(width, height));

	m_vResizeHackSize = Vector2(width, height);
	m_bResizeDelayHack = true;

	// hack to force update the XSizeHints state
	XResizeWindow(m_display, m_window, (unsigned int)width, (unsigned int)height);
	XMoveWindow(m_display, m_window, (int)windowPos.x, (int)windowPos.y);
	XRaiseWindow(m_display, m_window);

	if (!wasWindowResizable)
		setWindowResizableInt(false, Vector2(width, height));

	XFlush(m_display);
}

void LinuxEnvironment::setWindowResizable(bool resizable)
{
	setWindowResizableInt(resizable, getWindowSize());
}

void LinuxEnvironment::setWindowResizableInt(bool resizable, Vector2 windowSize)
{
	m_bResizable = resizable;

	const Vector2 windowPos = getWindowPos();

	// window managers may ignore this completely, there is no way to force it
	XSizeHints wmsize;
	memset(&wmsize, 0, sizeof(XSizeHints));

	wmsize.flags = PMinSize | PMaxSize;
	wmsize.min_width = m_bResizable ? 100 : (int)windowSize.x;
	wmsize.min_height = m_bResizable ? 100 : (int)windowSize.y;
	wmsize.max_width = m_bResizable ? (std::numeric_limits<int>::max() - 1) : (int)windowSize.x;
	wmsize.max_height = m_bResizable ? (std::numeric_limits<int>::max() - 1) : (int)windowSize.y;

	XSetWMNormalHints(m_display, m_window, &wmsize);

	// hack to force update the XSizeHints state
	XResizeWindow(m_display, m_window, (unsigned int)windowSize.x, (unsigned int)windowSize.y);
	XMoveWindow(m_display, m_window, (int)windowPos.x, (int)windowPos.y);
	XRaiseWindow(m_display, m_window);

	XFlush(m_display);
}

void LinuxEnvironment::setWindowGhostCorporeal(bool corporeal)
{
	// TODO:
}

void LinuxEnvironment::setMonitor(int monitor)
{
	// TODO:
	center();
}

Vector2 LinuxEnvironment::getWindowPos()
{
	// client coordinates
	Window rootRet;
	int x = 0;
	int y = 0;
	unsigned int borderWidth = 1;
	unsigned int depth = 0;
	unsigned int width = 1;
	unsigned int height = 1;

	XGetGeometry(m_display, m_window, &rootRet, &x, &y, &width, &height, &borderWidth, &depth);

	return Vector2(x, y);

	// server coordinates
	/*
	int x = 0;
	int y = 0;
	Window child;
	XWindowAttributes xwa;
	XTranslateCoordinates(m_display, m_window, DefaultRootWindow(m_display), 0, 0, &x, &y, &child );
	XGetWindowAttributes(m_display, m_window, &xwa);

	return Vector2(x - xwa.x, y - xwa.y);
	*/
}

Vector2 LinuxEnvironment::getWindowSize()
{
	// client size (engine coordinates)
	Window rootRet;
	int x = 0;
	int y = 0;
	unsigned int borderWidth = 1;
	unsigned int depth = 0;
	unsigned int width = 1;
	unsigned int height = 1;

	XGetGeometry(m_display, m_window, &rootRet, &x, &y, &width, &height, &borderWidth, &depth);

	return Vector2(width, height);
}

Vector2 LinuxEnvironment::getWindowSizeServer()
{
	// server size
	XWindowAttributes xwa;
	XGetWindowAttributes(m_display, m_window, &xwa);

	return Vector2(xwa.width, xwa.height);
}

int LinuxEnvironment::getMonitor()
{
	// TODO:
	return 0;
}

std::vector<McRect> LinuxEnvironment::getMonitors()
{
	return m_vMonitors;
}

Vector2 LinuxEnvironment::getNativeScreenSize()
{
	return Vector2(WidthOfScreen(DefaultScreenOfDisplay(m_display)), HeightOfScreen(DefaultScreenOfDisplay(m_display)));
}

McRect LinuxEnvironment::getVirtualScreenRect()
{
	// TODO:
	return McRect(0,0,1,1);
}

McRect LinuxEnvironment::getDesktopRect()
{
	// TODO:
	Vector2 screen = getNativeScreenSize();
	return McRect(0, 0, screen.x, screen.y);
}

bool LinuxEnvironment::isCursorInWindow()
{
	return m_bIsCursorInsideWindow;
}

bool LinuxEnvironment::isCursorVisible()
{
	return m_bCursorVisible;
}

bool LinuxEnvironment::isCursorClipped()
{
	return m_bCursorClipped;
}

Vector2 LinuxEnvironment::getMousePos()
{
	Window rootRet, childRet;
	unsigned int mask;
	int childX, childY;
	int rootX = 0;
	int rootY = 0;

	XQueryPointer(m_display, m_window, &rootRet, &childRet, &rootX, &rootY, &childX, &childY, &mask);

	return Vector2(childX, childY);
}

McRect LinuxEnvironment::getCursorClip()
{
	return m_cursorClip;
}

CURSORTYPE LinuxEnvironment::getCursor()
{
	return m_cursorType;
}

void LinuxEnvironment::setCursor(CURSORTYPE cur)
{
	m_cursorType = cur;

	if (!m_bCursorVisible) return;

	switch (cur)
	{
	case CURSOR_NORMAL:
		m_mouseCursor = XCreateFontCursor(m_display, XC_left_ptr);
		break;
	case CURSOR_WAIT:
		m_mouseCursor = XCreateFontCursor(m_display, XC_circle);
		break;
	case CURSOR_SIZE_H:
		m_mouseCursor = XCreateFontCursor(m_display, XC_sb_h_double_arrow);
		break;
	case CURSOR_SIZE_V:
		m_mouseCursor = XCreateFontCursor(m_display, XC_sb_v_double_arrow);
		break;
	case CURSOR_SIZE_HV:
		m_mouseCursor = XCreateFontCursor(m_display, XC_bottom_left_corner);
		break;
	case CURSOR_SIZE_VH:
		m_mouseCursor = XCreateFontCursor(m_display, XC_bottom_right_corner);
		break;
	case CURSOR_TEXT:
		m_mouseCursor = XCreateFontCursor(m_display, XC_xterm);
		break;
	default:
		m_mouseCursor = XCreateFontCursor(m_display, XC_left_ptr);
		break;
	}

	setCursorInt(m_mouseCursor);

	m_bCursorReset = true;
	m_bCursorRequest = true;
}

void LinuxEnvironment::setCursorVisible(bool visible)
{
	m_bCursorVisible = visible;
	setCursorInt(visible ? m_mouseCursor : m_invisibleCursor);
}

void LinuxEnvironment::setMousePos(int x, int y)
{
	XWarpPointer(m_display, None, m_window, 0, 0, 0, 0, x, y);
	XSync(m_display, False);
}

void LinuxEnvironment::setCursorClip(bool clip, McRect rect)
{
	if (clip)
	{
		const unsigned int eventMask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask | FocusChangeMask;
		m_bCursorClipped = (XGrabPointer(m_display, m_window, True, eventMask, GrabModeAsync, GrabModeAsync, m_window, None, CurrentTime) == GrabSuccess);
	}
	else
	{
		m_bCursorClipped = false;

		XUngrabPointer(m_display, CurrentTime);
		XSync(m_display, False);
	}
}

UString LinuxEnvironment::keyCodeToString(KEYCODE keyCode)
{
	const char *name = XKeysymToString(keyCode);
	return name != NULL ? UString(name) : UString("");
}



// helper functions

int LinuxEnvironment::getFilesInFolderFilter(const struct dirent *entry)
{
	return 1;
}

int LinuxEnvironment::getFoldersInFolderFilter(const struct dirent *entry)
{
	return 1;
}

Cursor LinuxEnvironment::makeBlankCursor()
{
	static char data[1] = {0};
	Cursor cursor;
	Pixmap blank;
	XColor dummy;

	blank = XCreateBitmapFromData(m_display, m_window, data, 1, 1);
	if (blank == None)
	{
		debugLog("LinuxEnvironment::makeBlankCursor() fatal error, XCreateBitmapFromData() out of memory!\n");
		return 0;
	}
	cursor = XCreatePixmapCursor(m_display, blank, blank, &dummy, &dummy, 0, 0);
	XFreePixmap(m_display, blank);

	return cursor;
}

void LinuxEnvironment::setCursorInt(Cursor cursor)
{
	if (m_bPrevCursorHack)
		XUndefineCursor(m_display, m_window);
	m_bPrevCursorHack = true;

	XDefineCursor(m_display, m_window, cursor);
}

UString LinuxEnvironment::readWindowProperty(Window window, Atom prop, Atom fmt /* XA_STRING or UTF8_STRING */, bool deleteAfterReading)
{
	UString returnData = UString("");
	unsigned char *clipData;
	Atom actualType;
	int actualFormat;
	unsigned long nitems, bytesLeft;
	if (XGetWindowProperty(m_display, m_window, prop, 0L /* offset */,
			1000000 /* length (max) */, False,
			AnyPropertyType /* format */, &actualType, &actualFormat, &nitems,
			&bytesLeft, &clipData) == Success)
	{
		if (actualType == m_atom_UTF8_STRING && actualFormat == 8)
		{
			// very inefficient, but whatever
			std::string temp;
			for (int i=0; i<nitems; i++)
			{
				temp += clipData[i];
			}
			returnData = UString(temp.c_str());
		}
		else if (actualType == XA_STRING && actualFormat == 8)
		{
			// very inefficient, but whatever
			std::string temp;
			for (int i=0; i<nitems; i++)
			{
				temp += clipData[i];
			}
			returnData = UString(temp.c_str());
		}

		if (clipData != 0)
			XFree(clipData);
	}

	if (deleteAfterReading)
		XDeleteProperty(m_display, window, prop);

	return returnData;
}

bool LinuxEnvironment::requestSelectionContent(UString &selection_content, Atom selection, Atom requested_format)
{
	// send a SelectionRequest to the window owning the selection and waits for its answer (with a timeout)
	// the selection owner will be asked to set the MCENGINE_SEL property on m_window with the selection content
	Atom property_name = XInternAtom(m_display, "MCENGINE_SEL", false);
	XConvertSelection(m_display, selection, requested_format, property_name, m_window, CurrentTime);
	bool gotReply = false;
	int timeoutMs = 200; // will wait at most for 200 ms
	do
	{
		XEvent event;
		gotReply = XCheckTypedWindowEvent(m_display, m_window, SelectionNotify, &event);
		if (gotReply)
		{
			if (event.xselection.property == property_name)
			{
				selection_content = readWindowProperty(event.xselection.requestor, event.xselection.property, requested_format, true);
				return true;
			}
			else // the format we asked for was denied.. (event.xselection.property == None)
				return false;
		}

		// not very elegant.. we could do a select() or something like that... however clipboard content requesting
		// is inherently slow on x11, it often takes 50ms or more so...
		usleep(4000);
		timeoutMs -= 4;
	}
	while (timeoutMs > 0);

	debugLog("LinuxEnvironment::requestSelectionContent() : Timeout!\n");
	return false;
}

void LinuxEnvironment::handleSelectionRequest(XSelectionRequestEvent &evt)
{
	// called from the event loop in response to SelectionRequest events
	// the selection content is sent to the target window as a window property
	XSelectionEvent reply;
	reply.type = SelectionNotify;
	reply.display = evt.display;
	reply.requestor = evt.requestor;
	reply.selection = evt.selection;
	reply.target = evt.target;
	reply.property = None; // == "fail"
	reply.time = evt.time;

	char *data = 0;
	int property_format = 0, data_nitems = 0;
	if (evt.selection == XA_PRIMARY || evt.selection == m_atom_CLIPBOARD)
	{
		if (evt.target == XA_STRING)
		{
			// format data according to system locale
			data = strdup((const char*)m_sLocalClipboardContent.toUtf8());
			data_nitems = strlen(data);
			property_format = 8; // bits/item
		}
		else if (evt.target == m_atom_UTF8_STRING)
		{
			// translate to utf8
			data = strdup((const char*)m_sLocalClipboardContent.toUtf8());
			data_nitems = strlen(data);
			property_format = 8; // bits/item
		}
		else if (evt.target == m_atom_TARGETS)
		{
			// another application wants to know what we are able to send
			data_nitems = 2;
			property_format = 32; // atoms are 32-bit
			data = (char*) malloc(data_nitems * 4);
			((Atom*)data)[0] = m_atom_UTF8_STRING;
			((Atom*)data)[1] = XA_STRING;
		}
	}
	else
		debugLog("LinuxEnvironment::handleSelectionRequest() : Requested unsupported clipboard!\n");

	if (data)
	{
		const size_t MAX_REASONABLE_SELECTION_SIZE = 1000000;

		// for very big chunks of data, we should use the "INCR" protocol, which is a pain in the asshole
		if (evt.property != None && strlen(data) < MAX_REASONABLE_SELECTION_SIZE)
		{
			XChangeProperty(evt.display, evt.requestor, evt.property,
					evt.target, property_format /* 8 or 32 */, PropModeReplace,
					(const unsigned char*) data, data_nitems);
			reply.property = evt.property; // " == success"
		}
		free(data);
	}

	XSendEvent(evt.display, evt.requestor, 0, NoEventMask, (XEvent *) &reply);
}

void LinuxEnvironment::setClipBoardTextInt(UString clipText)
{
	m_sLocalClipboardContent = clipText;
	XSetSelectionOwner(m_display, XA_PRIMARY, m_window, CurrentTime);
	XSetSelectionOwner(m_display, m_atom_CLIPBOARD, m_window, CurrentTime);
}

UString LinuxEnvironment::getClipboardTextInt()
{
	UString content;

	// clipboard code is modified from https://forum.juce.com/t/clipboard-support-in-linux/3894
	// thanks to jpo!

	/*
	 1) try to read from the "CLIPBOARD" selection first (the "high
	 level" clipboard that is supposed to be filled by ctrl-C
	 etc). When a clipboard manager is running, the content of this
	 selection is preserved even when the original selection owner
	 exits.

	 2) and then try to read from "PRIMARY" selection (the "legacy" selection
	 filled by good old x11 apps such as xterm)

	 3) a third fallback could be CUT_BUFFER0 but they are obsolete since X10 !
	 ( http://www.jwz.org/doc/x-cut-and-paste.html )

	 There is not content negotiation here -- we just try to retrieve the selection first
	 as utf8 and then as a locale-dependent string
	*/

	Atom selection = XA_PRIMARY;
	Window selection_owner = None;
	if ((selection_owner = XGetSelectionOwner(m_display, selection)) == None)
	{
		selection = m_atom_CLIPBOARD;
		selection_owner = XGetSelectionOwner(m_display, selection);
	}

	if (selection_owner != None)
	{
		if (selection_owner == m_window) // ourself
		    content = m_sLocalClipboardContent; // just return the local clipboard
		else
		{
			// first try: we want an utf8 string
			bool ok = requestSelectionContent(content, selection, m_atom_UTF8_STRING);
			if (!ok) // second chance, ask for a good old locale-dependent string
				ok = requestSelectionContent(content, selection, XA_STRING);
		}
	}

	return content;
}

#endif
