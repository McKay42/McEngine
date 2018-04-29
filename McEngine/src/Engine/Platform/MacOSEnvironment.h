//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		macOS
//
// $NoKeywords: $macenv
//===============================================================================//

#ifdef __APPLE__

#ifndef MACOSENVIRONMENT_H
#define MACOSENVIRONMENT_H

#include "Environment.h"

class MacOSWrapper;

class MacOSEnvironment : public Environment
{
public:
	MacOSEnvironment(MacOSWrapper *wrapper);
	virtual ~MacOSEnvironment();

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
	bool isFullscreen() {return m_bFullScreen;}
	bool isWindowResizable() {return m_bResizable;}

	// mouse
	bool isCursorInWindow();
	bool isCursorVisible();
	bool isCursorClipped() {return false;}
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
	inline MacOSWrapper *getWrapper() {return m_wrapper;}

private:
	static int getFilesInFolderFilter(const struct dirent *entry);
	static int getFoldersInFolderFilter(const struct dirent *entry);

	void setCursorVisible(bool visible, bool internalOverride);

	MacOSWrapper *m_wrapper;

	// monitors
	static std::vector<McRect> m_vMonitors;

	// window
	bool m_bFullScreen;
	bool m_bResizable;

	// mouse
	bool m_bCursorClipped;
	McRect m_cursorClip;
	bool m_bCursorRequest;
	bool m_bCursorReset;
	int m_iCursorInvisibleCounter;
	int m_iCursorVisibleCounter;
	bool m_bCursorVisible;
	bool m_bCursorVisibleInternalOverride;
	bool m_bIsCursorInsideWindow;
	CURSORTYPE m_cursorType;
};

#endif

#endif
