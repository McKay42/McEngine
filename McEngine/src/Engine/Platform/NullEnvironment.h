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

	// os calls
	void shutdown();
	UString getUsername() {return "<NULL>";}
	UString getUserDataPath() {return "<NULL>";}
	bool fileExists(UString filename) {return false;}
	UString getClipBoardText() {return "";}
	void setClipBoardText(UString text) {;}
	void openURLInDefaultBrowser(UString url) {;}
	UString openFileWindow(const char *filetypefilters, UString title, UString initialpath) {return "";}
	UString openFolderWindow(UString title, UString initialpath) {return "";}
	std::vector<UString> getFilesInFolder(UString folder) {return std::vector<UString>();}
	std::vector<UString> getFoldersInFolder(UString folder) {return std::vector<UString>();}
	std::vector<UString> getLogicalDrives() {return std::vector<UString>();}
	UString getFolderFromFilePath(UString filepath) {return "";}
	UString getFileExtensionFromFilePath(UString filepath, bool includeDot = false) {return "";}

	// message boxes
	void showMessageInfo(UString title, UString message) {;}
	void showMessageWarning(UString title, UString message) {;}
	void showMessageError(UString title, UString message) {;}
	void showMessageErrorFatal(UString title, UString message) {;}

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
	void setWindowGhostCorporeal(bool corporeal) {;}
	Vector2 getWindowPos() {return Vector2(0, 0);}
	Vector2 getWindowSize() {return Vector2(1280, 720);}
	Vector2 getNativeScreenSize() {return Vector2(1920, 1080);}
	Rect getVirtualScreenRect() {return Rect(0, 0, 1920, 1080);}
	Rect getDesktopRect() {return Rect(0, 0, 1920, 1080);}
	bool isFullscreen() {return false;}

	// mouse
	bool isCursorInWindow() {return true;}
	bool isCursorVisible() {return true;}
	bool isCursorClipped() {return false;}
	Vector2 getMousePos() {return Vector2(0, 0);}
	Rect getCursorClip() {return Rect(0, 0, 0, 0);}
	void setCursor(CURSORTYPE cur) {;}
	void setCursorVisible(bool visible) {;}
	void setMousePos(int x, int y) {;}
	void setCursorClip(bool clip, Rect rect) {;}
};

#endif
