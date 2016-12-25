//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		top level interface for native OS calls
//
// $NoKeywords: $env
//===============================================================================//

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "cbase.h"

#include "Cursors.h"

class Environment
{
public:
	Environment() {;}
	virtual ~Environment() {;}

	virtual void update() {;}

	// OS calls
	virtual void shutdown() = 0;
	virtual UString getUsername() = 0;
	virtual UString getUserDataPath() = 0;
	virtual bool fileExists(UString filename) = 0;
	virtual UString getClipBoardText() = 0;
	virtual void setClipBoardText(UString text) = 0;
	virtual UString openFileWindow(const char *filetypefilters, UString title, UString initialpath) = 0;
	virtual UString openFolderWindow(UString title, UString initialpath) = 0;
	virtual void openURLInDefaultBrowser(UString url) = 0;
	virtual std::vector<UString> getFilesInFolder(UString folder) = 0;
	virtual std::vector<UString> getFoldersInFolder(UString folder) = 0;
	virtual std::vector<UString> getLogicalDrives() = 0;
	virtual UString getFolderFromFilePath(UString filepath) = 0;
	virtual UString getFileExtensionFromFilePath(UString filepath, bool includeDot = false) = 0;

	// message boxes
	virtual void showMessageInfo(UString title, UString message) = 0;
	virtual void showMessageWarning(UString title, UString message) = 0;
	virtual void showMessageError(UString title, UString message) = 0;
	virtual void showMessageErrorFatal(UString title, UString message) = 0;

	// window
	virtual void focus() = 0;
	virtual void center() = 0;
	virtual void minimize() = 0;
	virtual void maximize() = 0;
	virtual void enableFullscreen() = 0;
	virtual void disableFullscreen() = 0;
	virtual void setWindowTitle(UString title) = 0;
	virtual void setWindowPos(int x, int y) = 0;
	virtual void setWindowSize(int width, int height) = 0;
	virtual void setWindowGhostCorporeal(bool corporeal) = 0;
	virtual Vector2 getWindowPos() = 0;
	virtual Vector2 getWindowSize() = 0;
	virtual Vector2 getNativeScreenSize() = 0;
	virtual Rect getVirtualScreenRect() = 0;
	virtual Rect getDesktopRect() = 0;
	virtual bool isFullscreen() = 0;

	// mouse
	virtual bool isCursorInWindow() = 0;
	virtual bool isCursorVisible() = 0;
	virtual bool isCursorClipped() = 0;
	virtual Vector2 getMousePos() = 0;
	virtual Rect getCursorClip() = 0;
	virtual void setCursor(CURSORTYPE cur) = 0;
	virtual void setCursorVisible(bool visible) = 0;
	virtual void setMousePos(int x, int y) = 0;
	virtual void setCursorClip(bool clip, Rect rect) = 0;
};

extern Environment *env;

#endif
