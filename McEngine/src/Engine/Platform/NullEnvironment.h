//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		empty environment, for debugging and new OS implementations
//
// $NoKeywords: $ne
//===============================================================================//

#ifndef NULLENVIRONMENT_H
#define NULLENVIRONMENT_H

#include "Environment.h"

class NullEnvironment : public Environment
{
public:
	NullEnvironment();
	virtual ~NullEnvironment() {;}

	// engine/factory
	Graphics *createRenderer();
	ContextMenu *createContextMenu();

	// system
	OS getOS() {return Environment::OS::OS_NULL;}
	void shutdown();
	void restart();
	void sleep(unsigned int us) {;}
	UString getExecutablePath() {return "";}
	void openURLInDefaultBrowser(UString url) {;}

	// user
	UString getUsername() {return "<NULL>";}
	UString getUserDataPath() {return "<NULL>";}

	// file IO
	bool fileExists(UString filename) {return false;}
	bool directoryExists(UString directoryName) {return false;}
	bool createDirectory(UString directoryName) {return false;}
	bool renameFile(UString oldFileName, UString newFileName) {return false;}
	bool deleteFile(UString filePath) {return false;}
	std::vector<UString> getFilesInFolder(UString folder) {return std::vector<UString>();}
	std::vector<UString> getFoldersInFolder(UString folder) {return std::vector<UString>();}
	std::vector<UString> getLogicalDrives() {return std::vector<UString>();}
	UString getFolderFromFilePath(UString filepath) {return "";}
	UString getFileExtensionFromFilePath(UString filepath, bool includeDot = false) {return "";}

	// clipboard
	UString getClipBoardText() {return "";}
	void setClipBoardText(UString text) {;}

	// dialogs & message boxes
	void showMessageInfo(UString title, UString message) {;}
	void showMessageWarning(UString title, UString message) {;}
	void showMessageError(UString title, UString message) {;}
	void showMessageErrorFatal(UString title, UString message) {;}
	UString openFileWindow(const char *filetypefilters, UString title, UString initialpath) {return "";}
	UString openFolderWindow(UString title, UString initialpath) {return "";}

	// window
	void focus() {;}
	void center() {;}
	void minimize() {;}
	void maximize() {;}
	void enableFullscreen() {;}
	void disableFullscreen() {;}
	void setWindowTitle(UString title) {;}
	void setWindowPos(int x, int y) {;}
	void setWindowSize(int width, int height) {;}
	void setWindowResizable(bool resizable) {;}
	void setWindowGhostCorporeal(bool corporeal) {;}
	void setMonitor(int monitor) {;}
	Vector2 getWindowPos() {return Vector2(0, 0);}
	Vector2 getWindowSize() {return Vector2(1280, 720);}
	int getMonitor() {return 0;}
	std::vector<McRect> getMonitors() {return std::vector<McRect>();}
	Vector2 getNativeScreenSize() {return Vector2(1920, 1080);}
	McRect getVirtualScreenRect() {return McRect(0, 0, 1920, 1080);}
	McRect getDesktopRect() {return McRect(0, 0, 1920, 1080);}
	bool isFullscreen() {return false;}
	bool isWindowResizable() {return true;}

	// mouse
	bool isCursorInWindow() {return true;}
	bool isCursorVisible() {return true;}
	bool isCursorClipped() {return false;}
	Vector2 getMousePos() {return Vector2(0, 0);}
	McRect getCursorClip() {return McRect(0, 0, 0, 0);}
	CURSORTYPE getCursor() {return CURSORTYPE::CURSOR_NORMAL;}
	void setCursor(CURSORTYPE cur) {;}
	void setCursorVisible(bool visible) {;}
	void setMousePos(int x, int y) {;}
	void setCursorClip(bool clip, McRect rect) {;}

	// keyboard
	UString keyCodeToString(KEYCODE keyCode) {return UString::format("%lu", keyCode);}
};

#endif
