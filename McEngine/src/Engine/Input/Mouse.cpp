//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		mouse wrapper
//
// $NoKeywords: $mouse
//===============================================================================//

#include "Mouse.h"

#include "ConVar.h"
#include "Engine.h"
#include "Environment.h"
#include "ResourceManager.h"

ConVar debug_mouse("debug_mouse", false, FCVAR_CHEAT);
ConVar debug_mouse_clicks("debug_mouse_clicks", false, FCVAR_NONE);

ConVar mouse_sensitivity("mouse_sensitivity", 1.0f, FCVAR_NONE);
ConVar mouse_raw_input("mouse_raw_input", false, FCVAR_NONE);
ConVar mouse_raw_input_absolute_to_window("mouse_raw_input_absolute_to_window", false, FCVAR_NONE);
ConVar mouse_fakelag("mouse_fakelag", 0.000f, FCVAR_NONE, "delay all mouse movement by this many seconds (e.g. 0.1 = 100 ms delay)");

ConVar tablet_sensitivity_ignore("tablet_sensitivity_ignore", false, FCVAR_NONE);

ConVar win_ink_workaround("win_ink_workaround", false, FCVAR_NONE);

Mouse::Mouse() : InputDevice()
{
	m_bMouseLeftDown = false;
	m_bMouseMiddleDown = false;
	m_bMouseRightDown = false;
	m_bMouse4Down = false;
	m_bMouse5Down = false;

	m_iWheelDeltaVertical = 0;
	m_iWheelDeltaHorizontal = 0;
	m_iWheelDeltaVerticalActual = 0;
	m_iWheelDeltaHorizontalActual = 0;

	m_bSetPosWasCalledLastFrame = false;
	m_bAbsolute = false;
	m_bVirtualDesktop = false;
	m_vOffset = Vector2(0, 0);
	m_vScale = Vector2(1, 1);
	m_vActualPos = m_vPosWithoutOffset = m_vPos = env->getMousePos();
	desktopRect = env->getDesktopRect();
}

void Mouse::draw(Graphics *g)
{
	if (!debug_mouse.getBool()) return;

	drawDebug(g);

	// green rect = virtual cursor pos
	g->setColor(0xff00ff00);
	float size = 20.0f;
	g->drawRect(m_vActualPos.x-size/2, m_vActualPos.y-size/2, size, size);

	// red rect = real cursor pos
	g->setColor(0xffff0000);
	Vector2 envPos = env->getMousePos();
	g->drawRect(envPos.x-size/2, envPos.y-size/2, size, size);

	// red = cursor clip
	if (env->isCursorClipped())
	{
		McRect cursorClip = env->getCursorClip();
		g->drawRect(cursorClip.getMinX(), cursorClip.getMinY(), cursorClip.getWidth()-1, cursorClip.getHeight()-1);
	}

	// green = scaled & offset virtual area
	const Vector2 scaledOffset = m_vOffset;
	const Vector2 scaledEngineScreenSize = engine->getScreenSize()*m_vScale;
	g->setColor(0xff00ff00);
	g->drawRect(-scaledOffset.x, -scaledOffset.y, scaledEngineScreenSize.x, scaledEngineScreenSize.y);
}

void Mouse::drawDebug(Graphics *g)
{
	Vector2 pos = getPos();

	g->setColor(0xff000000);
	g->drawLine(pos.x-1, pos.y-1, 0-1, pos.y-1);
	g->drawLine(pos.x-1, pos.y-1, engine->getScreenWidth()-1, pos.y-1);
	g->drawLine(pos.x-1, pos.y-1, pos.x-1, 0-1);
	g->drawLine(pos.x-1, pos.y-1, pos.x-1, engine->getScreenHeight()-1);

	g->setColor(0xffffffff);
	g->drawLine(pos.x, pos.y, 0, pos.y);
	g->drawLine(pos.x, pos.y, engine->getScreenWidth(), pos.y);
	g->drawLine(pos.x, pos.y, pos.x, 0);
	g->drawLine(pos.x, pos.y, pos.x, engine->getScreenHeight());

	float rectSizePercent = 0.05f;
	float aspectRatio = (float)engine->getScreenWidth() / (float)engine->getScreenHeight();
	Vector2 rectSize = Vector2(engine->getScreenWidth(), engine->getScreenHeight()*aspectRatio)*rectSizePercent;

	g->setColor(0xff000000);
	g->drawRect(pos.x - rectSize.x/2.0f-1, pos.y - rectSize.y/2.0f-1, rectSize.x, rectSize.y);

	g->setColor(0xffffffff);
	g->drawRect(pos.x - rectSize.x/2.0f, pos.y - rectSize.y/2.0f, rectSize.x, rectSize.y);

	McFont *posFont = engine->getResourceManager()->getFont("FONT_DEFAULT");
	UString posString = UString::format("[%i, %i]", (int)pos.x, (int)pos.y);
	float stringWidth = posFont->getStringWidth(posString);
	float stringHeight = posFont->getHeight();
	Vector2 textOffset = Vector2(pos.x + rectSize.x/2.0f + stringWidth + 5 > engine->getScreenWidth() ? -rectSize.x/2.0f - stringWidth - 5 : rectSize.x/2.0f + 5, (pos.y + rectSize.y/2.0f + stringHeight > engine->getScreenHeight()) ? -rectSize.y/2.0f - stringHeight : rectSize.y/2.0f + stringHeight );

	g->pushTransform();
	g->translate(pos.x+textOffset.x, pos.y+textOffset.y);
	g->drawString(posFont, UString::format("[%i, %i]", (int)pos.x, (int)pos.y));
	g->popTransform();
}

void Mouse::update()
{
	m_vDelta.zero();

	m_vRawDelta = m_vRawDeltaActual;
	m_vRawDeltaActual.zero();

	m_vRawDeltaAbsolute = m_vRawDeltaAbsoluteActual; // don't zero an absolute!

	resetWheelDelta();

	// TODO: clean up OS specific handling, specifically all the linux blocks

	// if the operating system cursor is potentially being used or visible in any way, do not interfere with it! (sensitivity, setCursorPos(), etc.)
	// same goes for a sensitivity of 1 without raw input, it is not necessary to call env->setPos() in that case

	McRect windowRect = McRect(0, 0, engine->getScreenWidth(), engine->getScreenHeight());
	const bool osCursorVisible = env->isCursorVisible() || !env->isCursorInWindow() || !engine->hasFocus();
	const bool sensitivityAdjustmentNeeded = mouse_sensitivity.getFloat() != 1.0f;

	const Vector2 osMousePos = env->getMousePos();

	Vector2 nextPos = osMousePos;

	if (osCursorVisible || (!sensitivityAdjustmentNeeded && !mouse_raw_input.getBool()) || m_bAbsolute || env->getOS() == Environment::OS::OS_HORIZON || env->getOS() == Environment::OS::OS_LINUX) // HACKHACK: linux hack
	{
		// this block handles visible/active OS cursor movement without sensitivity adjustments, and absolute input device movement
		if (m_bAbsolute)
		{
			// absolute input (with sensitivity)
			if (!tablet_sensitivity_ignore.getBool())
			{
				// NOTE: these range values work on windows only!
				// TODO: standardize the input values before they even reach the engine, this should not be in here
				float rawRangeX = 65536; // absolute coord range, but what if I want to have a tablet that's more accurate than 1/65536-th? >:(
				float rawRangeY = 65536;

				// if enabled, uses the screen resolution as the coord range, instead of 65536
				if (win_ink_workaround.getBool())
				{
					rawRangeX = env->getNativeScreenSize().x;
					rawRangeY = env->getNativeScreenSize().y;
				}

				// NOTE: mouse_raw_input_absolute_to_window only applies if raw input is enabled in general
				if (mouse_raw_input.getBool() && mouse_raw_input_absolute_to_window.getBool())
				{
					const Vector2 scaledOffset = m_vOffset;
					const Vector2 scaledEngineScreenSize = engine->getScreenSize() + 2*scaledOffset;

					nextPos.x = (((float)((m_vRawDeltaAbsolute.x - rawRangeX/2) * mouse_sensitivity.getFloat()) + rawRangeX/2) / rawRangeX) * scaledEngineScreenSize.x - scaledOffset.x;
					nextPos.y = (((float)((m_vRawDeltaAbsolute.y - rawRangeY/2) * mouse_sensitivity.getFloat()) + rawRangeY/2) / rawRangeY) * scaledEngineScreenSize.y - scaledOffset.y;
				}
				else
				{
                    // shift and scale to desktop
					McRect screen = m_bVirtualDesktop ? env->getVirtualScreenRect() : desktopRect;
					const Vector2 posInScreenCoords = Vector2((m_vRawDeltaAbsolute.x/rawRangeX) * screen.getWidth() + screen.getX(), (m_vRawDeltaAbsolute.y/rawRangeY) * screen.getHeight() + screen.getY());

					// offset to window
					nextPos = posInScreenCoords - env->getWindowPos();

					// apply sensitivity, scale and offset to engine
					nextPos.x = ((nextPos.x - engine->getScreenSize().x / 2) * mouse_sensitivity.getFloat() + engine->getScreenSize().x / 2);
					nextPos.y = ((nextPos.y - engine->getScreenSize().y / 2) * mouse_sensitivity.getFloat() + engine->getScreenSize().y / 2);
				}
			}
		}
		else
		{
			// relative input (without sensitivity)
			// (nothing to do here except updating the delta, since nextPos is already set to env->getMousePos() by default)
			m_vDelta = osMousePos - m_vPrevOsMousePos;
		}
	}
	else
	{
		// this block handles relative input with sensitivity adjustments, either raw or non-raw

		// calculate delta (either raw, or non-raw)
		if (mouse_raw_input.getBool())
			m_vDelta = m_vRawDelta; // this is already scaled to the sensitivity
		else
		{
			// non-raw input is always in pixels, sub-pixel movement is handled/buffered by the operating system
			if ((int)osMousePos.x != (int)m_vPrevOsMousePos.x || (int)osMousePos.y != (int)m_vPrevOsMousePos.y) // without this check some people would get mouse drift
				m_vDelta = (osMousePos - m_vPrevOsMousePos) * mouse_sensitivity.getFloat();
		}

		nextPos = m_vPosWithoutOffset + m_vDelta;

		// special case: relative input is ALWAYS clipped/confined to the window
		nextPos.x = clamp<float>(nextPos.x, windowRect.getMinX(), windowRect.getMaxX());
		nextPos.y = clamp<float>(nextPos.y, windowRect.getMinY(), windowRect.getMaxY());
	}

	// clip/confine cursor
	if (env->isCursorClipped() && env->getOS() != Environment::OS::OS_LINUX) // HACKHACK: linux hack
	{
		const McRect cursorClip = env->getCursorClip();

		const float minX = cursorClip.getMinX() - m_vOffset.x;
		const float minY = cursorClip.getMinY() - m_vOffset.y;

		const float maxX = minX + cursorClip.getWidth()*m_vScale.x;
		const float maxY = minY + cursorClip.getHeight()*m_vScale.y;

		nextPos.x = clamp<float>(nextPos.x, minX + 1, maxX - 1);
		nextPos.y = clamp<float>(nextPos.y, minY + 1, maxY - 1);
	}

	// set new virtual cursor position (this applies the offset as well)
	onPosChange(nextPos);

	// set new os cursor position, but only if the osMousePos is still within the window and the sensitivity is not 1;
	// raw input ALWAYS needs env->setPos()
	// first person games which call engine->getMouse()->setPos() every frame to manually re-center the cursor NEVER need env->setPos()
	// absolute input NEVER needs env->setPos()
	// also update prevOsMousePos
	if (windowRect.contains(osMousePos) && (sensitivityAdjustmentNeeded || mouse_raw_input.getBool()) && !m_bSetPosWasCalledLastFrame && !m_bAbsolute && env->getOS() != Environment::OS::OS_LINUX) // HACKHACK: linux hack
	{
		const Vector2 newOsMousePos = m_vPosWithoutOffset;

		env->setMousePos(newOsMousePos.x, newOsMousePos.y);

		// assume that the operating system has set the cursor to nextPos quickly enough for the next frame
		// also, force clamp to pixels, as this happens there too (to avoid drifting in the non-raw delta calculation)
		m_vPrevOsMousePos = newOsMousePos;
		m_vPrevOsMousePos.x = (int)m_vPrevOsMousePos.x;
		m_vPrevOsMousePos.y = (int)m_vPrevOsMousePos.y;

		// 3 cases can happen in the next frame:
		// 1) the operating system did not update the cursor quickly enough. osMousePos != nextPos
		// 2) the operating system did update the cursor quickly enough, but the user moved the mouse in the meantime. osMousePos != nextPos
		// 3) the operating system did update the cursor quickly enough, and the user did not move the mouse in the meantime. osMousePos == nextPos

		// it's impossible to determine if osMousePos != nextPos was caused by the user moving the mouse, or by the operating system not updating the cursor position quickly enough

		// 2 cases can happen after trusting the operating system to apply nextPos correctly:
		// 1) delta is applied twice, due to the operating system not updating quickly enough, cursor jumps too far
		// 2) delta is applied once, cursor works as expected

		// all of this shit can be avoided by just enabling mouse_raw_input
	}
	else
		m_vPrevOsMousePos = osMousePos;

	m_bSetPosWasCalledLastFrame = false;
}

void Mouse::addListener(MouseListener *mouseListener, bool insertOnTop)
{
	if (mouseListener == NULL)
	{
		engine->showMessageError("Mouse Error", "addListener(NULL)!");
		return;
	}

	if (insertOnTop)
		m_listeners.insert(m_listeners.begin(), mouseListener);
	else
		m_listeners.push_back(mouseListener);
}

void Mouse::removeListener(MouseListener *mouseListener)
{
	for (size_t i=0; i<m_listeners.size(); i++)
	{
		if (m_listeners[i] == mouseListener)
		{
			m_listeners.erase(m_listeners.begin() + i);
			i--;
		}
	}
}

void Mouse::resetWheelDelta()
{
	m_iWheelDeltaVertical = m_iWheelDeltaVerticalActual;
	m_iWheelDeltaVerticalActual = 0;

	m_iWheelDeltaHorizontal = m_iWheelDeltaHorizontalActual;
	m_iWheelDeltaHorizontalActual = 0;
}

void Mouse::onPosChange(Vector2 pos)
{
	m_vPos = (m_vOffset + pos);
	m_vPosWithoutOffset = pos;

	m_vActualPos = m_vPos;

	setPosXY(m_vPos.x, m_vPos.y);
}

void Mouse::setPosXY(float x, float y)
{
	if (mouse_fakelag.getFloat() > 0.0f)
	{
		FAKELAG_PACKET p;
		p.time = engine->getTime() + mouse_fakelag.getFloat();
		p.pos = Vector2(x, y);
		m_fakelagBuffer.push_back(p);

		float engineTime = engine->getTime();
		for (size_t i=0; i<m_fakelagBuffer.size(); i++)
		{
			if (engineTime >= m_fakelagBuffer[i].time)
			{
				m_vFakeLagPos = m_fakelagBuffer[i].pos;

				m_fakelagBuffer.erase(m_fakelagBuffer.begin() + i);
				i--;
			}
		}
		m_vPos = m_vFakeLagPos;
	}
	else
	{
		m_vPos.x = x;
		m_vPos.y = y;
	}
}

void Mouse::onRawMove(int xDelta, int yDelta, bool absolute, bool virtualDesktop)
{
	m_bAbsolute = absolute;
	m_bVirtualDesktop = virtualDesktop;

	if (xDelta != 0 || yDelta != 0) // sanity check, else some people get mouse drift like above, I don't even
	{
		if (!m_bAbsolute) // mouse
			m_vRawDeltaActual += Vector2(xDelta, yDelta) * mouse_sensitivity.getFloat();
		else // tablet
			m_vRawDeltaAbsoluteActual = Vector2(xDelta, yDelta);
	}
}

void Mouse::onWheelVertical(int delta)
{
	m_iWheelDeltaVerticalActual += delta;

	for (size_t i=0; i<m_listeners.size(); i++)
	{
		m_listeners[i]->onWheelVertical(delta);
	}
}

void Mouse::onWheelHorizontal(int delta)
{
	m_iWheelDeltaHorizontalActual += delta;

	for (size_t i=0; i<m_listeners.size(); i++)
	{
		m_listeners[i]->onWheelHorizontal(delta);
	}
}

void Mouse::onLeftChange(bool leftDown)
{
	if (debug_mouse_clicks.getBool())
		debugLog("Mouse::onLeftChange( %i )\n", (int)leftDown);

	m_bMouseLeftDown = leftDown;

	for (size_t i=0; i<m_listeners.size(); i++)
	{
		m_listeners[i]->onLeftChange(m_bMouseLeftDown);
	}
}

void Mouse::onMiddleChange(bool middleDown)
{
	if (debug_mouse_clicks.getBool())
		debugLog("Mouse::onMiddleChange( %i )\n", (int)middleDown);

	m_bMouseMiddleDown = middleDown;

	for (size_t i=0; i<m_listeners.size(); i++)
	{
		m_listeners[i]->onMiddleChange(m_bMouseMiddleDown);
	}
}

void Mouse::onRightChange(bool rightDown)
{
	if (debug_mouse_clicks.getBool())
		debugLog("Mouse::onRightChange( %i )\n", (int)rightDown);

	m_bMouseRightDown = rightDown;

	for (size_t i=0; i<m_listeners.size(); i++)
	{
		m_listeners[i]->onRightChange(m_bMouseRightDown);
	}
}

void Mouse::onButton4Change(bool button4down)
{
	if (debug_mouse_clicks.getBool())
		debugLog("Mouse::onButton4Change( %i )\n", (int)button4down);

	m_bMouse4Down = button4down;

	for (size_t i=0; i<m_listeners.size(); i++)
	{
		m_listeners[i]->onButton4Change(m_bMouse4Down);
	}
}

void Mouse::onButton5Change(bool button5down)
{
	if (debug_mouse_clicks.getBool())
		debugLog("Mouse::onButton5Change( %i )\n", (int)button5down);

	m_bMouse5Down = button5down;

	for (size_t i=0; i<m_listeners.size(); i++)
	{
		m_listeners[i]->onButton5Change(m_bMouse5Down);
	}
}

void Mouse::setPos(Vector2 newPos)
{
	m_bSetPosWasCalledLastFrame = true;

	setPosXY(newPos.x, newPos.y);
	env->setMousePos((int)m_vPos.x, (int)m_vPos.y);

	m_vPrevOsMousePos = m_vPos;
	m_vPrevOsMousePos.x = (int)m_vPrevOsMousePos.x;
	m_vPrevOsMousePos.y = (int)m_vPrevOsMousePos.y;
}

void Mouse::setCursorType(CURSORTYPE cursorType)
{
	env->setCursor(cursorType);
}

void Mouse::setCursorVisible(bool cursorVisible)
{
	env->setCursorVisible(cursorVisible);
}

bool Mouse::isCursorVisible()
{
	return env->isCursorVisible();
}
