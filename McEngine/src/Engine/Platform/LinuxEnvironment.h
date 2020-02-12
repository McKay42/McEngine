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

class LinuxEnvironment : public Environment
{
public:
	LinuxEnvironment() {;}
	virtual ~LinuxEnvironment() {;}

	virtual void update() = 0;

	// engine/factory
	virtual Graphics *createRenderer() = 0;
	ContextMenu *createContextMenu();

	// system
	OS getOS();
	virtual void shutdown() = 0;
	virtual void restart() = 0;
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
	virtual UString getClipBoardText() = 0;
	virtual void setClipBoardText(UString text) = 0;

	// dialogs & message boxes
	void showMessageInfo(UString title, UString message);
	void showMessageWarning(UString title, UString message);
	void showMessageError(UString title, UString message);
	void showMessageErrorFatal(UString title, UString message);
	UString openFileWindow(const char *filetypefilters, UString title, UString initialpath);
	UString openFolderWindow(UString title, UString initialpath);

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
	virtual void setWindowResizable(bool resizable) = 0;
	virtual void setWindowGhostCorporeal(bool corporeal) = 0;
	virtual void setMonitor(int monitor) = 0;
	virtual Vector2 getWindowPos() = 0;
	virtual Vector2 getWindowSize() = 0;
	virtual int getMonitor() = 0;
	virtual std::vector<McRect> getMonitors() = 0;
	virtual Vector2 getNativeScreenSize() = 0;
	virtual McRect getVirtualScreenRect() = 0;
	virtual McRect getDesktopRect() = 0;
	virtual int getDPI() = 0;
	virtual bool isFullscreen() = 0;
	virtual bool isWindowResizable() = 0;

	// mouse
	virtual bool isCursorInWindow() = 0;
	virtual bool isCursorVisible() = 0;
	virtual bool isCursorClipped() = 0;
	virtual Vector2 getMousePos() = 0;
	virtual McRect getCursorClip() = 0;
	virtual CURSORTYPE getCursor() = 0;
	virtual void setCursor(CURSORTYPE cur) = 0;
	virtual void setCursorVisible(bool visible) = 0;
	virtual void setMousePos(int x, int y) = 0;
	virtual void setCursorClip(bool clip, McRect rect) = 0;

	// keyboard
	virtual UString keyCodeToString(KEYCODE keyCode) = 0;

private:
	static int getFilesInFolderFilter(const struct dirent *entry);
	static int getFoldersInFolderFilter(const struct dirent *entry);
};

#endif

#endif
