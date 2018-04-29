//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		macOS objective-c wrapper, main_OSX_objc calls main_OSX_cpp
//
// $NoKeywords: $macwrp
//===============================================================================//

#ifdef __APPLE__

#ifndef MAIN_OSX_CPP_H
#define MAIN_OSX_CPP_H

class Timer;

class MacOSWrapper
{
public:
	struct VECTOR2
	{
		float x;
		float y;
	};

	// reverse callbacks (c++ -> objective-c)

	// native
	static void microSleep(int microSeconds);

	// Environment
	static const char *getUsername();
	static void openURLInDefaultBrowser(const char *url);

	// clipboard
	static const char *getClipboardText();
	static void setClipboardText(const char *text);

	// dialogs & message boxes
	static void showMessageInfo(const char *title, const char *message);
	static void showMessageWarning(const char *title, const char *message);
	static void showMessageError(const char *title, const char *message);
	static void showMessageErrorFatal(const char *title, const char *message);

	// window
	static void center();
	static void focus();
	static void minimize();
	static void maximize();
	static void enableFullscreen();
	static void disableFullscreen();
	static void setWindowTitle(const char *title);
	static void setWindowPos(int x, int y);
	static void setWindowSize(int width, int height);
	static void setWindowResizable(bool resizable);
	static VECTOR2 getWindowPos();
	static VECTOR2 getWindowSize();
	static int getMonitor();
	static VECTOR2 getNativeScreenSize();

	// mouse
	static VECTOR2 getMousePos();
	static void setCursor(int cursor);
	static void setCursorVisible(bool visible);
	static void setMousePos(int x, int y);
	static void setCursorClip(bool clip);

	// Graphics
	static void endScene();
	static void setVSync(bool vsync);

public:
	MacOSWrapper();
	~MacOSWrapper();

	// native callbacks (objective-c -> c++)

	void loadApp();
	void main_objc_before_winproc();
	void main_objc_after_winproc();

	void onFocusGained();
	void onFocusLost();

	void onMouseRawMove(int xDelta, int yDelta);
	void onMouseWheelVertical(int delta);
	void onMouseWheelHorizontal(int delta);
	void onMouseLeftChange(bool mouseLeftDown);
	void onMouseMiddleChange(bool mouseMiddleDown);
	void onMouseRightChange(bool mouseRightDown);

	void onKeyboardKeyDown(unsigned long keyCode);
	void onKeyboardKeyUp(unsigned long keyCode);
	void onKeyboardChar(unsigned long charCode);

	void requestResolutionChange(float newWidth, float newHeight);

private:
	Timer *m_frameTimer;
	Timer *m_deltaTimer;

	float m_fPrevTime;
};

#endif

#endif
