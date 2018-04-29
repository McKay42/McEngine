//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		macOS objective-c wrapper, main_OSX_objc calls main_OSX_cpp
//
// $NoKeywords: $macwrp
//===============================================================================//

#ifdef __APPLE__

#include "main_OSX_cpp.h"

#include "Engine.h"
#include "Timer.h"
#include "ConVar.h"

#include "MacOSEnvironment.h"

#include "OpenGLHeaders.h"

#ifndef MCENGINE_FEATURE_SDL



Engine *g_engine = NULL;

bool g_bRunning = true;

bool g_bUpdate = true;
bool g_bDraw = true;
bool g_bDrawing = false;

bool g_bMinimized = false; // for fps_max_background
bool g_bHasFocus = true; // for fps_max_background // TODO: focus handling!

ConVar fps_max("fps_max", 60.0f);
ConVar fps_max_background("fps_max_background", 30.0f);
ConVar fps_unlimited("fps_unlimited", false);



MacOSWrapper::MacOSWrapper()
{
    // create timers
	m_frameTimer = new Timer();
	m_frameTimer->start();
	m_frameTimer->update();

	m_deltaTimer = new Timer();
	m_deltaTimer->start();
	m_deltaTimer->update();

	m_fPrevTime = 0.0f;

	// initialize engine
	// TODO: args
	g_engine = new Engine(new MacOSEnvironment(this), "");
}

MacOSWrapper::~MacOSWrapper()
{
	delete g_engine;
	delete m_frameTimer;
	delete m_deltaTimer;
}

void MacOSWrapper::loadApp()
{
	g_engine->loadApp();
}

void MacOSWrapper::main_objc_before_winproc()
{
	m_frameTimer->update();
}

void MacOSWrapper::main_objc_after_winproc()
{
	// update
	if (g_bUpdate)
		g_engine->onUpdate();

	// draw
	if (g_bDraw)
	{
		g_bDrawing = true;
			g_engine->onPaint();
		g_bDrawing = false;
	}

	// delay the next frame
	m_frameTimer->update();
	m_deltaTimer->update();

	if (m_deltaTimer->getElapsedTime() > m_fPrevTime)
	{
		m_fPrevTime = m_deltaTimer->getElapsedTime() + 1.0f;
		printf("fps = %f, delta = %f\n", (1.0f / m_deltaTimer->getDelta()), m_deltaTimer->getDelta());
	}

	engine->setFrameTime(m_deltaTimer->getDelta());

	const bool inBackground = g_bMinimized || !g_bHasFocus;
	if (!fps_unlimited.getBool() || inBackground)
	{
		double delayStart = m_frameTimer->getElapsedTime();
		double delayTime;
		if (inBackground)
			delayTime = (1.0 / (double)fps_max_background.getFloat()) - m_frameTimer->getDelta();
		else
			delayTime = (1.0 / (double)fps_max.getFloat()) - m_frameTimer->getDelta();

		while (delayTime > 0.0)
		{
			if (inBackground) // real waiting (very inaccurate, but very good for little background cpu utilization)
				microSleep((int)((1.0f / fps_max_background.getFloat())*1000.0f*1000.0f));
			else // more or less "busy" waiting, but giving away the rest of the timeslice at least
				microSleep(1);

			// decrease the delayTime by the time we spent in this loop
			// if the loop is executed more than once, note how delayStart now gets the value of the previous iteration from getElapsedTime()
			// this works because the elapsed time is only updated in update(). now we can easily calculate the time the Sleep() took and subtract it from the delayTime
			delayStart = m_frameTimer->getElapsedTime();
			m_frameTimer->update();
			delayTime -= (m_frameTimer->getElapsedTime() - delayStart);
		}
	}
}

void MacOSWrapper::onFocusGained()
{
	g_bHasFocus = true;
	g_engine->onFocusGained();
}

void MacOSWrapper::onFocusLost()
{
	g_bHasFocus = false;
	g_engine->onFocusLost();
}

void MacOSWrapper::onMouseRawMove(int xDelta, int yDelta)
{
	g_engine->onMouseRawMove(xDelta, yDelta);
}

void MacOSWrapper::onMouseWheelVertical(int delta)
{
	g_engine->onMouseWheelVertical(delta);
}

void MacOSWrapper::onMouseWheelHorizontal(int delta)
{
	g_engine->onMouseWheelHorizontal(delta);
}

void MacOSWrapper::onMouseLeftChange(bool mouseLeftDown)
{
	g_engine->onMouseLeftChange(mouseLeftDown);
}

void MacOSWrapper::onMouseMiddleChange(bool mouseMiddleDown)
{
	g_engine->onMouseMiddleChange(mouseMiddleDown);
}

void MacOSWrapper::onMouseRightChange(bool mouseRightDown)
{
	g_engine->onMouseRightChange(mouseRightDown);
}

void MacOSWrapper::onKeyboardKeyDown(unsigned long keyCode)
{
	g_engine->onKeyboardKeyDown(keyCode);
}

void MacOSWrapper::onKeyboardKeyUp(unsigned long keyCode)
{
	g_engine->onKeyboardKeyUp(keyCode);
}

void MacOSWrapper::onKeyboardChar(unsigned long charCode)
{
	g_engine->onKeyboardChar(charCode);
}

void MacOSWrapper::requestResolutionChange(float newWidth, float newHeight)
{
	g_engine->requestResolutionChange(Vector2(newWidth, newHeight));
}

#endif

#endif
