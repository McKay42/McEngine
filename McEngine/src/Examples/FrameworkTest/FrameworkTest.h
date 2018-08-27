//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		engine framework test app
//
// $NoKeywords: $test
//===============================================================================//

#ifndef FRAMEWORKTEST_H
#define FRAMEWORKTEST_H

#include "App.h"
#include "MouseListener.h"
#include "GamepadListener.h"

class CBaseUIButton;

class FrameworkTest : public App, public MouseListener, public GamepadListener
{
public:
	FrameworkTest();
	virtual ~FrameworkTest();

	virtual void draw(Graphics *g);
	virtual void update();

	virtual void onResolutionChanged(Vector2 newResolution);

	virtual void onFocusGained();
	virtual void onFocusLost();

	virtual void onMinimized();
	virtual void onRestored();

public:
	// keyboard
	virtual void onKeyDown(KeyboardEvent &e);
	virtual void onKeyUp(KeyboardEvent &e);
	virtual void onChar(KeyboardEvent &e);

	// mouse
	void onLeftChange(bool down);
	void onMiddleChange(bool down);
	void onRightChange(bool down);

	void onWheelVertical(int delta);
	void onWheelHorizontal(int delta);

	// gamepad
	void onButtonDown(GAMEPADBUTTON b);
	void onButtonUp(GAMEPADBUTTON b);

	void onConnected();
	void onDisconnected();

	// UI
	CBaseUIButton *m_testButton;
};

#endif
