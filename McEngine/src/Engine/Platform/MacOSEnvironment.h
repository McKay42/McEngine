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

	// os calls
	void shutdown();
	UString getUsername();
	UString getUserDataPath() {return "<NULL>";}
	bool fileExists(UString filename);
	UString getClipBoardText() {return "";}
	void setClipBoardText(UString text) {;}
	void openURLInDefaultBrowser(UString url);
	UString openFileWindow(const char *filetypefilters, UString title, UString initialpath) {return "";}
	UString openFolderWindow(UString title, UString initialpath) {return "";}
	std::vector<UString> getFilesInFolder(UString folder);
	std::vector<UString> getFoldersInFolder(UString folder);
	std::vector<UString> getLogicalDrives() {return std::vector<UString>();}
	UString getFolderFromFilePath(UString filepath) {return "";}
	UString getFileExtensionFromFilePath(UString filepath, bool includeDot = false);

	// message boxes
	void showMessageInfo(UString title, UString message);
	void showMessageWarning(UString title, UString message);
	void showMessageError(UString title, UString message);
	void showMessageErrorFatal(UString title, UString message);

	// window
	void focus() {;}
	void center() {;}
	void minimize();
	void maximize() {;}
	void enableFullscreen() {;}
	void disableFullscreen() {;}
	void setWindowTitle(UString title);
	void setWindowPos(int x, int y) {;}
	void setWindowSize(int width, int height) {;}
	void setWindowGhostCorporeal(bool corporeal) {;}
	Vector2 getWindowPos() {return Vector2(0, 0);}
	Vector2 getWindowSize();
	Vector2 getNativeScreenSize() {return Vector2(1920, 1080);}
	McRect getVirtualScreenRect() {return McRect(0, 0, 1920, 1080);}
	McRect getDesktopRect() {return McRect(0, 0, 1920, 1080);}
	bool isFullscreen() {return false;}

	// mouse
	bool isCursorInWindow();
	bool isCursorVisible();
	bool isCursorClipped() {return false;}
	Vector2 getMousePos();
	McRect getCursorClip() {return McRect(0, 0, 0, 0);}
	void setCursor(CURSORTYPE cur);
	void setCursorVisible(bool visible);
	void setMousePos(int x, int y);
	void setCursorClip(bool clip, McRect rect);

	// ILLEGAL:
	inline MacOSWrapper *getWrapper() {return m_wrapper;}

private:
	static int getFilesInFolderFilter(const struct dirent *entry);
	static int getFoldersInFolderFilter(const struct dirent *entry);

	void setCursorVisible(bool visible, bool internalOverride);

	MacOSWrapper *m_wrapper;

	bool m_bCursorRequest;
	bool m_bCursorReset;

	int m_iCursorInvisibleCounter;
	int m_iCursorVisibleCounter;

	bool m_bCursorVisible;
	bool m_bCursorVisibleInternalOverride;
	bool m_bIsCursorInsideWindow;
};

#endif

#endif
