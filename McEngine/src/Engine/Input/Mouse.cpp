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

ConVar debug_mouse("debug_mouse", false);
ConVar mouse_sensitivity("mouse_sensitivity", 1.0f);
ConVar mouse_raw_input("mouse_raw_input", false);
ConVar mouse_raw_input_absolute_to_window("mouse_raw_input_absolute_to_window", false);
ConVar mouse_fakelag("mouse_fakelag", 0.000f);
ConVar tablet_sensitivity_ignore("tablet_sensitivity_ignore", false);
ConVar win_ink_workaround("win_ink_workaround", false);

Mouse::Mouse() : InputDevice()
{
	m_bMouseLeftDown = false;
	m_bMouseMiddleDown = false;
	m_bMouseRightDown = false;

	m_iWheelDeltaVertical = 0;
	m_iWheelDeltaHorizontal = 0;
	m_iWheelDeltaVerticalActual = 0;
	m_iWheelDeltaHorizontalActual = 0;

	m_bAbsolute = false;
	m_bVirtualDesktop = false;
	m_bSwitch = false;
	m_vOffset = Vector2(0,0);
	m_vScale = Vector2(1,1);
	m_vActualPos = m_vPrevPos = m_vPos = env->getMousePos();
	desktopRect = env->getDesktopRect();
}

void Mouse::draw(Graphics *g)
{
	if (!debug_mouse.getBool())
		return;

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
	const Vector2 scaledOffset = m_vOffset*m_vScale;
	const Vector2 scaledEngineScreenSize = engine->getScreenSize()*m_vScale + 2*scaledOffset;
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
	m_vRawDelta = m_vRawDeltaActual;
	if (!m_bAbsolute)
		m_vRawDeltaActual.zero();

	m_iWheelDeltaVertical = m_iWheelDeltaVerticalActual;
	m_iWheelDeltaVerticalActual = 0;

	m_iWheelDeltaHorizontal = m_iWheelDeltaHorizontalActual;
	m_iWheelDeltaHorizontalActual = 0;

	if (!engine->hasFocus() || env->isCursorVisible() || !env->isCursorInWindow() || m_bAbsolute)
	{
		// this block here handles movement if the OS cursor is visible, or if we have an absolute input device (tablet)

		if (m_bSwitch) // auto-detect mode switches
		{
			if (debug_mouse.getBool())
				debugLog("Mouse: Switching to absolute block.\n");

			m_bSwitch = false;

			// TODO: this here relies on setPos() being executed quickly enough, otherwise the next call is going to teleport the mouse to the center due to the previous position still being active
			// fuck microsoft for making SetCursorPos() inconsistent
			if (engine->hasFocus())
			{
				setPos(m_vActualPos);
				onPosChange(m_vActualPos);
			}
		}
		else
		{
			Vector2 nextCursorPos;
			if (m_bAbsolute && !tablet_sensitivity_ignore.getBool())
			{
				// HACKHACK: this works on windows only!!!
				float rawRangeX = 65536; // absolute coord range, but what if I want to have a tablet that's more accurate than 1/65536-th? >:(
				float rawRangeY = 65536;

				if (win_ink_workaround.getBool()) // if enabled, uses the screen resolution as the coord range
				{
					rawRangeX = env->getNativeScreenSize().x;
					rawRangeY = env->getNativeScreenSize().y;
				}

				if (mouse_raw_input_absolute_to_window.getBool())
				{
					const Vector2 scaledOffset = m_vOffset*m_vScale;
					const Vector2 scaledEngineScreenSize = engine->getScreenSize()*m_vScale + 2*scaledOffset;

					nextCursorPos.x = (((float)((m_vRawDelta.x - rawRangeX/2) * mouse_sensitivity.getFloat()) + rawRangeX/2) / rawRangeX) * scaledEngineScreenSize.x - scaledOffset.x;
					nextCursorPos.y = (((float)((m_vRawDelta.y - rawRangeY/2) * mouse_sensitivity.getFloat()) + rawRangeY/2) / rawRangeY) * scaledEngineScreenSize.y - scaledOffset.y;
				}
				else
				{
                    // shift and scale to desktop
					McRect screen = m_bVirtualDesktop ? env->getVirtualScreenRect() : desktopRect;
					const Vector2 posInScreenCoords = Vector2((m_vRawDelta.x/rawRangeX) * screen.getWidth() + screen.getX(), (m_vRawDelta.y/rawRangeY) * screen.getHeight() + screen.getY());

					// offset to window
					nextCursorPos = posInScreenCoords - env->getWindowPos();

					// apply sensitivity, scale and offset to engine
					nextCursorPos.x = ((nextCursorPos.x - engine->getScreenSize().x / 2) * mouse_sensitivity.getFloat() + engine->getScreenSize().x / 2);
					nextCursorPos.y = ((nextCursorPos.y - engine->getScreenSize().y / 2) * mouse_sensitivity.getFloat() + engine->getScreenSize().y / 2);
				}
			}
			else
				nextCursorPos = env->getMousePos();

			onPosChange(nextCursorPos);
		}
	}
	else
	{
		// this block here handles relative movement, both for raw input and non-raw input (mouse, touchpad)
		McRect windowRect = McRect(0, 0, engine->getScreenWidth(), engine->getScreenHeight());

		if (!m_bSwitch) // auto-detect mode switches
		{
			if (debug_mouse.getBool())
				debugLog("Mouse: Switching to relative block.\n");

			m_vPrevPos = env->getMousePos();
			m_vActualPos = m_vPos - m_vOffset; // compensate for virtual offset
			m_bSwitch = true;
		}
		else
		{
			// calculate delta
			Vector2 delta;
			if (!mouse_raw_input.getBool())
			{
				Vector2 pos = env->getMousePos();
				if ((int)pos.x != (int)m_vPrevPos.x || (int)pos.y != (int)m_vPrevPos.y) // without this check some people would get mouse drift
					delta = (pos - m_vPrevPos)*mouse_sensitivity.getFloat();
			}
			else
				delta = m_vRawDelta;

			// calculate temporary new position
			m_vActualPos += delta;

			// respect cursor clipping
			if (env->isCursorClipped())
			{
				McRect cursorClip = env->getCursorClip();
				m_vActualPos.x = clamp<float>(m_vActualPos.x, cursorClip.getMinX(), cursorClip.getMaxX());
				m_vActualPos.y = clamp<float>(m_vActualPos.y, cursorClip.getMinY(), cursorClip.getMaxY());
			}

			// set new virtual position
			m_vPos = (m_vOffset + m_vActualPos)*m_vScale;
			m_vPos.x = (int)m_vPos.x;
			m_vPos.y = (int)m_vPos.y;
		}

		// handle real cursor setPos, detect mode switches early
		if (windowRect.contains(m_vActualPos)) // if the actual cursor is still within the window
		{
			// set and remember prev pos
			setPos(m_vPos);
			m_vPrevPos = m_vPos;
		}
		else // the actual cursor has left the window, switch virtual <=> actual
		{
			// TODO: if scale is not (1,1), switching modes here will cause a cursor teleport (due to cursor movement between different resolutions, which should not be allowed)
			setPos(m_vActualPos);
			m_vPrevPos = m_vPos = m_vActualPos;
		}
	}
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
	for (int i=0; i<m_listeners.size(); i++)
	{
		if (m_listeners[i] == mouseListener)
		{
			m_listeners.erase(m_listeners.begin() + i);
			i--;
		}
	}
}

void Mouse::onPosChange(Vector2 pos)
{
	m_vPos = (m_vOffset + pos)*m_vScale;
	m_vPos.x = (int)m_vPos.x;
	m_vPos.y = (int)m_vPos.y;

	m_vPrevPos = m_vActualPos = m_vPos;

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
		for (int i=0; i<m_fakelagBuffer.size(); i++)
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
			m_vRawDeltaActual += Vector2(xDelta, yDelta)*mouse_sensitivity.getFloat();
		else // tablet
			m_vRawDeltaActual = Vector2(xDelta, yDelta);
	}
}

void Mouse::onWheelVertical(int delta)
{
	m_iWheelDeltaVerticalActual += delta;

	for (int i=0; i<m_listeners.size(); i++)
	{
		m_listeners[i]->onWheelVertical(delta);
	}
}

void Mouse::onWheelHorizontal(int delta)
{
	m_iWheelDeltaHorizontalActual += delta;

	for (int i=0; i<m_listeners.size(); i++)
	{
		m_listeners[i]->onWheelHorizontal(delta);
	}
}

void Mouse::onLeftChange(bool leftDown)
{
	m_bMouseLeftDown = leftDown;

	for (int i=0; i<m_listeners.size(); i++)
	{
		m_listeners[i]->onLeftChange(leftDown);
	}
}

void Mouse::onMiddleChange(bool middleDown)
{
	m_bMouseMiddleDown = middleDown;

	for (int i=0; i<m_listeners.size(); i++)
	{
		m_listeners[i]->onMiddleChange(middleDown);
	}
}

void Mouse::onRightChange(bool rightDown)
{
	m_bMouseRightDown = rightDown;

	for (int i=0; i<m_listeners.size(); i++)
	{
		m_listeners[i]->onRightChange(rightDown);
	}
}

void Mouse::setPos(Vector2 newPos)
{
	setPosXY(newPos.x, newPos.y);
	env->setMousePos(m_vPos.x, m_vPos.y);
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
