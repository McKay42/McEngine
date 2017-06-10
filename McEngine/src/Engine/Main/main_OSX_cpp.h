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

	static void showMessageInfo(const char *title, const char *message);
	static void showMessageWarning(const char *title, const char *message);
	static void showMessageError(const char *title, const char *message);
	static void showMessageErrorFatal(const char *title, const char *message);

	static void minimize();
	static void setWindowTitle(const char *title);
	static VECTOR2 getWindowSize();

	static VECTOR2 getMousePos();
	static void setCursor(int cursor);
	static void setCursorVisible(bool visible);
	static void setMousePos(int x, int y);

	// Graphics
	static void setVSync(bool vsync);

public:
	// native callbacks (objective-c -> c++)
	MacOSWrapper();
	~MacOSWrapper();

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
