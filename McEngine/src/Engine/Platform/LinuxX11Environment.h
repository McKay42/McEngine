#ifdef __linux__

#ifndef LINUXX11ENVIRONMENT_H
#define LINUXX11ENVIRONMENT_H

#include "LinuxEnvironment.h"

#include <X11/X.h>
#include <X11/Xlib.h>

class LinuxX11Environment : public LinuxEnvironment
{
public:
	LinuxX11Environment(Display *display, Window window);
	virtual ~LinuxX11Environment();

	void update();

	// engine/factory
	Graphics *createRenderer();

	// system
	void shutdown();
	void restart();

	// clipboard
	UString getClipBoardText();
	void setClipBoardText(UString text);

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

	inline Display *getDisplay() const {return m_display;}
	inline Window getWindow() const {return m_window;}
	inline bool isRestartScheduled() const {return m_bIsRestartScheduled;}

	void handleSelectionRequest(XSelectionRequestEvent &evt);
private:
	void setWindowResizableInt(bool resizable, Vector2 windowSize);
	Vector2 getWindowSizeServer();

	Cursor makeBlankCursor();
	void setCursorInt(Cursor cursor);

	UString readWindowProperty(Window window, Atom prop, Atom fmt /* XA_STRING or UTF8_STRING */, bool deleteAfterReading);
	bool requestSelectionContent(UString &selection_content, Atom selection, Atom requested_format);
	void setClipBoardTextInt(UString clipText);
	UString getClipboardTextInt();

	Display *m_display;
	Window m_window;

	// monitors
	static std::vector<McRect> m_vMonitors;

	// window
	static bool m_bResizable;
	bool m_bFullScreen;
	Vector2 m_vLastWindowPos;
	Vector2 m_vLastWindowSize;
	int m_iDPI;

	// mouse
	bool m_bCursorClipped;
	McRect m_cursorClip;
	bool m_bCursorRequest;
	bool m_bCursorReset;
	bool m_bCursorVisible;
	bool m_bIsCursorInsideWindow;
	Cursor m_mouseCursor;
	Cursor m_invisibleCursor;
	CURSORTYPE m_cursorType;

	// clipboard
	UString m_sLocalClipboardContent;
	Atom m_atom_UTF8_STRING;
	Atom m_atom_CLIPBOARD;
	Atom m_atom_TARGETS;

	// custom
	bool m_bIsRestartScheduled;
	bool m_bResizeDelayHack;
	Vector2 m_vResizeHackSize;
	bool m_bPrevCursorHack;
	bool m_bFullscreenWasResizable;
	Vector2 m_vPrevDisableFullscreenWindowSize;
};

#endif

#endif
