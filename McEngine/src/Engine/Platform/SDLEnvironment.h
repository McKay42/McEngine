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

	virtual void update();

	// engine/factory
	virtual Graphics *createRenderer();
	virtual ContextMenu *createContextMenu();

	// system
	virtual OS getOS();
	virtual void shutdown();
	virtual void restart();
	virtual void sleep(unsigned int us); // NOTE: inaccurate
	virtual UString getExecutablePath();
	virtual void openURLInDefaultBrowser(UString url); // NOTE: non-SDL

	// user
	virtual UString getUsername(); // NOTE: non-SDL
	virtual UString getUserDataPath();

	// file IO
	virtual bool fileExists(UString filename);
	virtual bool directoryExists(UString directoryName); // NOTE: non-SDL
	virtual bool createDirectory(UString directoryName); // NOTE: non-SDL
	virtual bool renameFile(UString oldFileName, UString newFileName);
	virtual bool deleteFile(UString filePath);
	virtual std::vector<UString> getFilesInFolder(UString folder);		// NOTE: non-SDL
	virtual std::vector<UString> getFoldersInFolder(UString folder);	// NOTE: non-SDL
	virtual std::vector<UString> getLogicalDrives();					// NOTE: non-SDL
	virtual UString getFolderFromFilePath(UString filepath);			// NOTE: non-SDL
	virtual UString getFileExtensionFromFilePath(UString filepath, bool includeDot = false);
	virtual UString getFileNameFromFilePath(UString filePath);			// NOTE: non-SDL

	// clipboard
	virtual UString getClipBoardText();
	virtual void setClipBoardText(UString text);

	// dialogs & message boxes
	virtual void showMessageInfo(UString title, UString message);
	virtual void showMessageWarning(UString title, UString message);
	virtual void showMessageError(UString title, UString message);
	virtual void showMessageErrorFatal(UString title, UString message);
	virtual UString openFileWindow(const char *filetypefilters, UString title, UString initialpath);	// NOTE: non-SDL
	virtual UString openFolderWindow(UString title, UString initialpath);								// NOTE: non-SDL

	// window
	virtual void focus();
	virtual void center();
	virtual void minimize();
	virtual void maximize();
	virtual void enableFullscreen();
	virtual void disableFullscreen();
	virtual void setWindowTitle(UString title);
	virtual void setWindowPos(int x, int y);
	virtual void setWindowSize(int width, int height);
	virtual void setWindowResizable(bool resizable);
	virtual void setWindowGhostCorporeal(bool corporeal);
	virtual void setMonitor(int monitor);
	virtual Vector2 getWindowPos();
	virtual Vector2 getWindowSize();
	virtual int getMonitor();
	virtual std::vector<McRect> getMonitors() {return m_vMonitors;}
	virtual Vector2 getNativeScreenSize();
	virtual McRect getVirtualScreenRect();
	virtual McRect getDesktopRect();
	virtual int getDPI();
	virtual bool isFullscreen() {return m_bFullscreen;}
	virtual bool isWindowResizable() {return m_bResizable;}

	// mouse
	virtual bool isCursorInWindow() {return m_bIsCursorInsideWindow;}
	virtual bool isCursorVisible() {return m_bCursorVisible;}
	virtual bool isCursorClipped() {return m_bCursorClipped;}
	virtual Vector2 getMousePos();
	virtual McRect getCursorClip() {return m_cursorClip;}
	virtual CURSORTYPE getCursor() {return m_cursorType;}
	virtual void setCursor(CURSORTYPE cur);
	virtual void setCursorVisible(bool visible);
	virtual void setMousePos(int x, int y);
	virtual void setCursorClip(bool clip, McRect rect);

	// keyboard
	virtual UString keyCodeToString(KEYCODE keyCode);

	// ILLEGAL:
	void setWindow(SDL_Window *window) {m_window = window;}
	inline SDL_Window *getWindow() {return m_window;}
	void setWasLastMouseInputTouch(bool wasLastMouseInputTouch) {m_bWasLastMouseInputTouch = wasLastMouseInputTouch;}
	inline bool wasLastMouseInputTouch() const {return m_bWasLastMouseInputTouch;}

protected:
	SDL_Window *m_window;

private:
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

	// touch
	bool m_bWasLastMouseInputTouch;

	// clipboard
	const char *m_sPrevClipboardTextSDL;
};

#endif

#endif
