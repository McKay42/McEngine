//================ Copyright (c) 2018, PG, All rights reserved. =================//
//
// Purpose:		SDL ("partial", SDL does not provide all functions!)
//
// $NoKeywords: $sdlenv
//===============================================================================//

#ifndef SDLENVIRONMENT_H
#define SDLENVIRONMENT_H

#include "cbase.h"

#ifdef MCENGINE_FEATURE_SDL

#include "SDL.h"

#include "Environment.h"

class SDLEnvironment : public Environment
{
public:
	SDLEnvironment(SDL_Window *window);
	virtual ~SDLEnvironment() {;}

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
	void openURLInDefaultBrowser(UString url); // NOTE: non-SDL

	// user
	UString getUsername();
	UString getUserDataPath();

	// file IO
	bool fileExists(UString filename);
	bool directoryExists(UString directoryName); // NOTE: non-SDL
	bool createDirectory(UString directoryName); // NOTE: non-SDL
	bool renameFile(UString oldFileName, UString newFileName);
	bool deleteFile(UString filePath);
	std::vector<UString> getFilesInFolder(UString folder);		// NOTE: non-SDL
	std::vector<UString> getFoldersInFolder(UString folder);	// NOTE: non-SDL
	std::vector<UString> getLogicalDrives();					// NOTE: non-SDL
	UString getFolderFromFilePath(UString filepath);			// NOTE: non-SDL
	UString getFileExtensionFromFilePath(UString filepath, bool includeDot = false);

	// clipboard
	UString getClipBoardText();
	void setClipBoardText(UString text);

	// dialogs & message boxes
	void showMessageInfo(UString title, UString message);
	void showMessageWarning(UString title, UString message);
	void showMessageError(UString title, UString message);
	void showMessageErrorFatal(UString title, UString message);
	UString openFileWindow(const char *filetypefilters, UString title, UString initialpath);	// NOTE: non-SDL
	UString openFolderWindow(UString title, UString initialpath);								// NOTE: non-SDL

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
	std::vector<McRect> getMonitors() {return m_vMonitors;}
	Vector2 getNativeScreenSize();
	McRect getVirtualScreenRect();
	McRect getDesktopRect();
	bool isFullscreen() {return m_bFullscreen;}
	bool isWindowResizable() {return m_bResizable;}

	// mouse
	bool isCursorInWindow() {return m_bIsCursorInsideWindow;}
	bool isCursorVisible() {return m_bCursorVisible;}
	bool isCursorClipped() {return m_bCursorClipped;}
	Vector2 getMousePos();
	McRect getCursorClip() {return m_cursorClip;}
	CURSORTYPE getCursor() {return m_cursorType;}
	void setCursor(CURSORTYPE cur);
	void setCursorVisible(bool visible);
	void setMousePos(int x, int y);
	void setCursorClip(bool clip, McRect rect);

	// keyboard
	UString keyCodeToString(KEYCODE keyCode);

	// ILLEGAL:
	inline SDL_Window *getWindow() {return m_window;}

private:
	SDL_Window *m_window;

	ConVar *m_mouse_sensitivity_ref;

	// monitors
	std::vector<McRect> m_vMonitors;

	// window
	bool m_bResizable;
	bool m_bFullscreen;

	// mouse
	bool m_bIsCursorInsideWindow;
	bool m_bCursorVisible;
	bool m_bCursorClipped;
	McRect m_cursorClip;
	CURSORTYPE m_cursorType;
};

#endif

#endif
