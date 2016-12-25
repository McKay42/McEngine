//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		linux
//
// $NoKeywords: $linuxenv
//===============================================================================//

#ifdef __linux__

#include "LinuxEnvironment.h"
#include "Engine.h"

#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <unistd.h>

#include <string.h>
#include <stdio.h>

LinuxEnvironment::LinuxEnvironment(Display *display, Window window)
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

	m_bFullScreen = false;
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

	m_bIsCursorInsideWindow = Rect(0, 0, engine->getScreenWidth(), engine->getScreenHeight()).contains(getMousePos());
}

void LinuxEnvironment::shutdown()
{
	XEvent ev;
	memset(&ev, 0, sizeof (ev));

	ev.type = ClientMessage;
	ev.xclient.type = ClientMessage;
	ev.xclient.window = m_window;
	ev.xclient.message_type = XInternAtom(m_display, "WM_PROTOCOLS", true);
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = XInternAtom(m_display, "WM_DELETE_WINDOW", false);
	ev.xclient.data.l[1] = CurrentTime;

	XSendEvent(m_display, m_window, false, NoEventMask, &ev);
}

UString LinuxEnvironment::getUsername()
{
	// TODO:
	return "";
}

UString LinuxEnvironment::getUserDataPath()
{
	// TODO:
	return "";
}

bool LinuxEnvironment::fileExists(UString filename)
{
	return (bool)(std::ifstream(filename.toUtf8()));
}

UString LinuxEnvironment::getClipBoardText()
{
	// TODO:
	return "";
}

void LinuxEnvironment::setClipBoardText(UString text)
{
	// TODO:
}

void LinuxEnvironment::openURLInDefaultBrowser(UString url)
{
	if (fork() == 0)
		exit(execl("/usr/bin/xdg-open", "xdg-open", url.toUtf8(), (char*)0));
}

UString LinuxEnvironment::openFileWindow(const char *filetypefilters, UString title, UString initialpath)
{
	// TODO:
	return "";
}

UString LinuxEnvironment::openFolderWindow(UString title, UString initialpath)
{
	// TODO:
	return "";
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
	return drives;
}

UString LinuxEnvironment::getFolderFromFilePath(UString filepath)
{
	// TODO:
	return "";
}

UString LinuxEnvironment::getFileExtensionFromFilePath(UString filepath, bool includeDot)
{
	int idx = filepath.findLast(".");
	if (idx != -1)
		return filepath.substr(idx+1);
	else
		return UString("");
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

void LinuxEnvironment::focus()
{
	XRaiseWindow(m_display, m_window);
}

void LinuxEnvironment::center()
{
	Vector2 windowSize = getWindowSize();
	Screen *defaultScreen = XDefaultScreenOfDisplay(m_display);
	XMoveResizeWindow(m_display, m_window, WidthOfScreen(defaultScreen)/2 - windowSize.x/2, HeightOfScreen(defaultScreen)/2 - windowSize.y/2, windowSize.x, windowSize.y);
}

void LinuxEnvironment::minimize()
{
	XIconifyWindow(m_display, m_window, 0);
}

void LinuxEnvironment::maximize()
{
	XMapWindow(m_display, m_window);
	// TODO: maybe resize?
}

#define _NET_WM_STATE_TOGGLE    2

void LinuxEnvironment::toggleFullscreen()
{
	debugLog("toggleFullscreen()");

	/*
    XEvent xev;
    long evmask = SubstructureRedirectMask | SubstructureNotifyMask;
	xev.type = ClientMessage;
	xev.xclient.window = m_window;
	xev.xclient.message_type = XInternAtom(m_display, "_NET_WM_STATE_FULLSCREEN", true);
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = _NET_WM_STATE_TOGGLE;
	xev.xclient.data.l[1] = true; // STATE_FULLSCREEN?
	xev.xclient.data.l[2] = 0;  // no second property to toggle
	xev.xclient.data.l[3] = 1;  // source indication: application
	xev.xclient.data.l[4] = 0;  // unused

	if (!XSendEvent(m_display, m_window, 0, evmask, &xev))
		debugLog("LinuxEnvironment: Couldn't XSendEvent() fullscreen!");
	*/

	/*
	Atom wm_state   = XInternAtom (m_display, "_NET_WM_STATE", true );
	Atom wm_fullscreen = XInternAtom (m_display, "_NET_WM_STATE_FULLSCREEN", true );

	XChangeProperty(m_display, m_window, wm_state, XA_ATOM, 32,
	                PropModeReplace, (unsigned char *)&wm_fullscreen, 1);
	*/
    // TODO:
}

void LinuxEnvironment::enableFullscreen()
{
	// TODO:
}

void LinuxEnvironment::disableFullscreen()
{
	// TODO:
}

void LinuxEnvironment::setWindowTitle(UString title)
{
	XStoreName(m_display, m_window, title.toUtf8());
}

void LinuxEnvironment::setWindowPos(int x, int y)
{
	XMoveWindow(m_display, m_window, x, y);
}

void LinuxEnvironment::setWindowSize(int width, int height)
{
	XResizeWindow(m_display, m_window, width, height);
}

void LinuxEnvironment::setWindowGhostCorporeal(bool corporeal)
{
	// TODO:
}

Vector2 LinuxEnvironment::getWindowPos()
{
	Window rootRet;
	int x = 0;
	int y = 0;
	unsigned int borderWidth = 1;
	unsigned int depth = 0;
	unsigned int width = 1;
	unsigned int height = 1;

	XGetGeometry(m_display, m_window, &rootRet, &x, &y, &width, &height, &borderWidth, &depth);

	return Vector2(x, y);
}

Vector2 LinuxEnvironment::getWindowSize()
{
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

Vector2 LinuxEnvironment::getNativeScreenSize()
{
	return Vector2(WidthOfScreen(DefaultScreenOfDisplay(m_display)), HeightOfScreen(DefaultScreenOfDisplay(m_display)));
}

Rect LinuxEnvironment::getVirtualScreenRect()
{
	// TODO:
	return Rect(0,0,1,1);
}

Rect LinuxEnvironment::getDesktopRect()
{
	// TODO:
	Vector2 screen = getNativeScreenSize();
	return Rect(0, 0, screen.x, screen.y);
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

Rect LinuxEnvironment::getCursorClip()
{
	return m_cursorClip;
}

void LinuxEnvironment::setCursor(CURSORTYPE cur)
{
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

	XDefineCursor(m_display, m_window, m_mouseCursor);

	m_bCursorReset = true;
	m_bCursorRequest = true;
}

void LinuxEnvironment::setCursorVisible(bool visible)
{
	m_bCursorVisible = visible;
	XDefineCursor(m_display, m_window, visible ? m_mouseCursor : m_invisibleCursor);
}

void LinuxEnvironment::setMousePos(int x, int y)
{
	XWarpPointer(m_display, None, m_window, 0, 0, 0, 0, x, y);
	XFlush(m_display);
}

void LinuxEnvironment::setCursorClip(bool clip, Rect rect)
{
	if (clip)
	{
		// TODO: is a custom rectangle even possible?
		/*
		if (XGrabPointer(m_display, m_window, True, 0, GrabModeAsync, GrabModeAsync, m_window, None, CurrentTime) == GrabSuccess)
			m_bCursorClipped = true;
		else
			m_bCursorClipped = false;
		*/

		// TODO: this is fucked
	}
	else
	{
		XUngrabPointer(m_display, CurrentTime);
		m_bCursorClipped = false;
	}
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

#endif
