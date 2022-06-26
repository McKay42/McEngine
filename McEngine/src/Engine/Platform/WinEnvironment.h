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

	static void bluescreen();

public:
	WinEnvironment(HWND hwnd, HINSTANCE hinstance);
	virtual ~WinEnvironment();

	void update();

	// engine/factory
	Graphics *createRenderer();
	ContextMenu *createContextMenu();

	// system
	OS getOS();
	void shutdown();
	void restart();
	void sleep(unsigned int us);
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
	UString getFileNameFromFilePath(UString filePath);

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
	void setMonitor(int monitor);
	Vector2 getWindowPos();
	Vector2 getWindowSize();
	int getMonitor();
	std::vector<McRect> getMonitors();
	Vector2 getNativeScreenSize();
	McRect getVirtualScreenRect();
	McRect getDesktopRect();
	int getDPI();
	bool isFullscreen() {return m_bFullScreen;}
	bool isWindowResizable() {return m_bResizable;}

	// mouse
	bool isCursorInWindow();
	bool isCursorVisible();
	bool isCursorClipped();
	Vector2 getMousePos();
	McRect getCursorClip();
	CURSORTYPE getCursor();
	void setCursor(CURSORTYPE cur);
	void setCursorVisible(bool visible);
	void setMousePos(int x, int y);
	void setCursorClip(bool clip, McRect rect);

	// keyboard
	UString keyCodeToString(KEYCODE keyCode);

	// ILLEGAL:
	// (also custom)
	void setDPIOverride(int newForcedDPI) {m_iDPIOverride = newForcedDPI;}
	bool setProcessPriority(int priority); // 0 = normal, 1 = high
	bool setProcessAffinity(int affinity); // -1 = reset (all cores), 0 = first core, 1 = last core
	void disableWindowsKey();
	void enableWindowsKey();
	inline HWND getHwnd() const {return m_hwnd;}
	inline HINSTANCE getHInstance() const {return m_hInstance;}
	inline bool isRestartScheduled() const {return m_bIsRestartScheduled;}

private:
	static BOOL CALLBACK monitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
	static LRESULT CALLBACK lowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

	void path_strip_filename(TCHAR *Path);
	void handleShowMessageFullscreen();
	void enumerateMonitors();
	void onProcessPriorityChange(UString oldValue, UString newValue);
	void onDisableWindowsKeyChange(UString oldValue, UString newValue);

	HWND m_hwnd;
	HINSTANCE m_hInstance;

	// monitors
	static std::vector<McRect> m_vMonitors;

	// window
	static bool m_bResizable;
	bool m_bFullScreen;
	Vector2 m_vWindowSize;
	Vector2 m_vLastWindowPos;
	Vector2 m_vLastWindowSize;

	// mouse
	bool m_bCursorClipped;
	McRect m_cursorClip;
	bool m_bIsCursorInsideWindow;
	bool m_bHasCursorTypeChanged;
	HCURSOR m_mouseCursor;
	CURSORTYPE m_cursorType;

	// custom
	int m_iDPIOverride;
	bool m_bIsRestartScheduled;
	static int m_iNumCoresForProcessAffinity;
	static HHOOK g_hKeyboardHook;
};

#endif

#endif
