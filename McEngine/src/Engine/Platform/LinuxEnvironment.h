//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		linux
//
// $NoKeywords: $linuxenv
//===============================================================================//

#ifdef __linux__

#ifndef LINUXENVIRONMENT_H
#define LINUXENVIRONMENT_H

#include "Environment.h"

#include <X11/X.h>
#include <X11/Xlib.h>

class LinuxEnvironment : public Environment
{
public:
	LinuxEnvironment(Display *display, Window window);
	virtual ~LinuxEnvironment();

	void update();

	// engine/factory
	Graphics *createRenderer();
	ContextMenu *createContextMenu();

	// system
	OS getOS();
	void shutdown();
	void restart();
	UString getExecutablePath();
	void openURLInDefaultBrowser(UString url);

	// user
	UString getUsername();
	UString getUserDataPath();

	// file IO
	bool fileExists(UString filename);
	bool directoryExists(UString directoryName);
	bool createDirectory(UString directoryName);
	bool renameFile(UString oldFileName, UString newFileName);
	bool deleteFile(UString filePath);
	std::vector<UString> getFilesInFolder(UString folder);
	std::vector<UString> getFoldersInFolder(UString folder);
	std::vector<UString> getLogicalDrives();
	UString getFolderFromFilePath(UString filepath);
	UString getFileExtensionFromFilePath(UString filepath, bool includeDot = false);

	// clipboard
	UString getClipBoardText();
	void setClipBoardText(UString text);

	// dialogs & message boxes
	void showMessageInfo(UString title, UString message);
	void showMessageWarning(UString title, UString message);
	void showMessageError(UString title, UString message);
	void showMessageErrorFatal(UString title, UString message);
	UString openFileWindow(const char *filetypefilters, UString title, UString initialpath);
	UString openFolderWindow(UString title, UString initialpath);

	// window
	void focus();
	void center();
	void minimize();
	void maximize();
	void enableFullscreen();
	void disableFullscreen();
	void setWindowTitle(UString title);
	void setWindowPos(int x, int y);
	void setWindowSize(int width, int height);
	void setWindowResizable(bool resizable);
	void setWindowGhostCorporeal(bool corporeal);
	Vector2 getWindowPos();
	Vector2 getWindowSize();
	Vector2 getNativeScreenSize();
	Rect getVirtualScreenRect();
	Rect getDesktopRect();
	bool isFullscreen() {return m_bFullScreen;}
	bool isWindowResizable() {return m_bResizable;}

	// mouse
	bool isCursorInWindow();
	bool isCursorVisible();
	bool isCursorClipped();
	Vector2 getMousePos();
	Rect getCursorClip();
	void setCursor(CURSORTYPE cur);
	void setCursorVisible(bool visible);
	void setMousePos(int x, int y);
	void setCursorClip(bool clip, Rect rect);

	// ILLEGAL:
	inline Display *getDisplay() const {return m_display;}
	inline Window getWindow() const {return m_window;}

private:
	static int getFilesInFolderFilter(const struct dirent *entry);
	static int getFoldersInFolderFilter(const struct dirent *entry);
	Cursor makeBlankCursor();

	void toggleFullscreen();

	Display *m_display;
	Window m_window;

	static bool m_bResizable;
	bool m_bCursorClipped;
	Rect m_cursorClip;
	bool m_bCursorRequest;
	bool m_bCursorReset;
	bool m_bCursorVisible;
	bool m_bIsCursorInsideWindow;
	Cursor m_mouseCursor;
	Cursor m_invisibleCursor;

	bool m_bFullScreen;
};

#endif

#endif
