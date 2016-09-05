//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		windows
//
// $NoKeywords: $winenv
//===============================================================================//

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#ifndef WINENVIRONMENT_H
#define WINENVIRONMENT_H

#include "Environment.h"

#include <windows.h>

class WinEnvironment : public Environment
{
public:
	static long getWindowStyleWindowed();
	static long getWindowStyleFullscreen();

public:
	WinEnvironment(HWND hwnd, HINSTANCE hinstance);
	virtual ~WinEnvironment() {;}

	void update();

	// os calls
	void shutdown();
	bool fileExists(UString filename);
	UString getClipBoardText();
	void setClipBoardText(UString text);
	void openURLInDefaultBrowser(UString url);
	UString openFileWindow(const char *filetypefilters, UString title, UString initialpath);
	UString openFolderWindow(UString title, UString initialpath);
	std::vector<UString> getFilesInFolder(UString folder);
	std::vector<UString> getFoldersInFolder(UString folder);
	std::vector<UString> getLogicalDrives();
	UString getFolderFromFilePath(UString filepath);
	UString getFileExtensionFromFilePath(UString filepath, bool includeDot = false);

	// message boxes
	void showMessageInfo(UString title, UString message);
	void showMessageWarning(UString title, UString message);
	void showMessageError(UString title, UString message);
	void showMessageErrorFatal(UString title, UString message);

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
	void setWindowGhostCorporeal(bool corporeal);
	Vector2 getWindowPos();
	Vector2 getWindowSize();
	Vector2 getNativeScreenSize();
	Rect getVirtualScreenRect();
	Rect getDesktopRect();
	bool isFullscreen() {return m_bFullScreen;}

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
	inline HWND getHwnd() const {return m_hwnd;}
	inline HINSTANCE getHInstance() const {return m_hInstance;}

private:
	void path_strip_filename(TCHAR *Path);
	void handleShowMessageFullscreen();

	HWND m_hwnd;
	HINSTANCE m_hInstance;

	// window
	bool m_bFullScreen;
	Vector2 m_vWindowSize;
	Vector2 m_vLastWindowPos;
	Vector2 m_vLastWindowSize;

	// mouse
	bool m_bCursorClipped;
	Rect m_cursorClip;
	bool m_bIsCursorInsideWindow;
	HCURSOR m_mouseCursor;
	HCURSOR m_mouseCursorArrow;
};

#endif

#endif
