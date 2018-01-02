//================ Copyright (c) 2012, PG, All rights reserved. =================//
//
// Purpose:		a simple slider
//
// $NoKeywords: $
//===============================================================================//

#include "CBaseUISlider.h"

#include "Engine.h"
#include "Mouse.h"
#include "AnimationHandler.h"
#include "Keyboard.h"

CBaseUISlider::CBaseUISlider(float xPos, float yPos, float xSize, float ySize, UString name) : CBaseUIElement(xPos,yPos,xSize,ySize,name)
{
	m_bDrawFrame = true;
	m_bDrawBackground = true;
	m_bHorizontal = false;
	m_bHasChanged = false;
	m_bAnimated = true;
	m_bLiveUpdate = false;
	m_bAllowMouseWheel = true;

	m_backgroundColor = COLOR(255, 0, 0, 0);
	m_frameColor = COLOR(255,255,255,255);

	m_fCurValue = 0.0f;
	m_fCurPercent = 0.0f;
	m_fMinValue = 0.0f;
	m_fMaxValue = 1.0f;
	m_fKeyDelta = 0.1f;

	m_vBlockSize = Vector2(xSize < ySize ? xSize : ySize, xSize < ySize ? xSize : ySize);

	m_sliderChangeCallback = NULL;

	setOrientation(xSize > ySize);
}

void CBaseUISlider::draw(Graphics *g)
{
	if (!m_bVisible) return;

	// draw background
	if (m_bDrawBackground)
	{
		g->setColor(m_backgroundColor);
		g->fillRect(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y + 1);
	}

	// draw frame
	g->setColor(m_frameColor);
	if (m_bDrawFrame)
		g->drawRect(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y + 1);

	// draw sliding line
	if (!m_bHorizontal)
		g->drawLine(m_vPos.x + m_vSize.x/2.0f, m_vPos.y + m_vBlockSize.y/2.0, m_vPos.x + m_vSize.x/2.0f, m_vPos.y + m_vSize.y - m_vBlockSize.y/2.0f);
	else
		g->drawLine(m_vPos.x + (m_vBlockSize.x-1)/2 + 1, m_vPos.y + m_vSize.y/2.0f + 1, m_vPos.x + m_vSize.x - (m_vBlockSize.x-1)/2, m_vPos.y + m_vSize.y/2.0f + 1);

	drawBlock(g);
}

void CBaseUISlider::drawBlock(Graphics *g)
{
	// draw block
	Vector2 center = m_vPos + Vector2(m_vBlockSize.x/2 + (m_vSize.x-m_vBlockSize.x)*getPercent(), m_vSize.y/2);
	Vector2 topLeft = center - m_vBlockSize/2;
	Vector2 topRight = center + Vector2(m_vBlockSize.x/2 + 1, -m_vBlockSize.y/2);
	Vector2 halfLeft = center + Vector2(-m_vBlockSize.x/2, 1);
	Vector2 halfRight = center + Vector2(m_vBlockSize.x/2 + 1, 1);
	Vector2 bottomLeft = center + Vector2(-m_vBlockSize.x/2, m_vBlockSize.y/2 + 1);
	Vector2 bottomRight = center + Vector2(m_vBlockSize.x/2 + 1, m_vBlockSize.y/2 + 1);

	g->drawQuad(topLeft,
				topRight,
				halfRight + Vector2(0,1),
				halfLeft + Vector2(0,1),
				COLOR(255,255,255,255),
				COLOR(255,255,255,255),
				COLOR(255,241,241,241),
				COLOR(255,241,241,241));

	g->drawQuad(halfLeft,
				halfRight,
				bottomRight,
				bottomLeft,
				COLOR(255,225,225,225),
				COLOR(255,225,225,225),
				COLOR(255,255,255,255),
				COLOR(255,255,255,255));


	/*
	g->drawQuad(Vector2(m_vPos.x+std::round(m_vBlockPos.x)+1, m_vPos.y+std::round(m_vBlockPos.y)+1),
				Vector2(m_vPos.x+std::round(m_vBlockPos.x)+1 + m_vBlockSize.x-1, m_vPos.y+std::round(m_vBlockPos.y)+1),
				Vector2(m_vPos.x+std::round(m_vBlockPos.x)+1 + m_vBlockSize.x-1, m_vPos.y+std::round(m_vBlockPos.y)+1 + m_vBlockSize.y/2),
				Vector2(m_vPos.x+std::round(m_vBlockPos.x)+1,  m_vPos.y+std::round(m_vBlockPos.y)+1 + m_vBlockSize.y/2),
				COLOR(255,255,255,255),
				COLOR(255,255,255,255),
				COLOR(255,241,241,241),
				COLOR(255,241,241,241));

	g->drawQuad(Vector2(m_vPos.x+std::round(m_vBlockPos.x)+1, m_vPos.y+std::round(m_vBlockPos.y)+1+std::round(m_vBlockSize.y/2.0f)),
				Vector2(m_vPos.x+std::round(m_vBlockPos.x)+1 + m_vBlockSize.x-1, m_vPos.y+std::round(m_vBlockPos.y)+1+std::round(m_vBlockSize.y/2.0f)),
				Vector2(m_vPos.x+std::round(m_vBlockPos.x)+1 + m_vBlockSize.x-1, m_vPos.y+std::round(m_vBlockPos.y)+1+std::round(m_vBlockSize.y/2.0f) + m_vBlockSize.y-(std::round(m_vBlockSize.y/2.0f))),
				Vector2(m_vPos.x+std::round(m_vBlockPos.x)+1, m_vPos.y+std::round(m_vBlockPos.y)+1+std::round(m_vBlockSize.y/2.0f)  + m_vBlockSize.y-(std::round(m_vBlockSize.y/2.0f))),
				COLOR(255,225,225,225),
				COLOR(255,225,225,225),
				COLOR(255,255,255,255),
				COLOR(255,255,255,255));
	*/

	/*
	g->setColor(0xff00ff00);
	g->fillRect(m_vPos.x+m_vBlockPos.x+1, m_vPos.y+m_vBlockPos.y+1, m_vBlockSize.x-1, m_vBlockSize.y);
	*/
}

void CBaseUISlider::update()
{
	CBaseUIElement::update();
	if (!m_bVisible) return;

	Vector2 mousepos = engine->getMouse()->getPos();

	// handle moving
	if (m_bActive)
	{
		// calculate new values
		if (!m_bHorizontal)
		{
			if (m_bAnimated)
				anim->moveQuadOut( &m_vBlockPos.y, clamp<float>( mousepos.y - m_vGrabBackup.y, 0.0f, m_vSize.y-m_vBlockSize.y ), 0.10f, 0, true );
			else
				m_vBlockPos.y = clamp<float>( mousepos.y - m_vGrabBackup.y, 0.0f, m_vSize.y-m_vBlockSize.y );

			m_fCurPercent = clamp<float>(1.0f - (std::round(m_vBlockPos.y) / (m_vSize.y-m_vBlockSize.y)), 0.0f, 1.0f);
		}
		else
		{
			if (m_bAnimated)
				anim->moveQuadOut( &m_vBlockPos.x, clamp<float>( mousepos.x - m_vGrabBackup.x, 0.0f, m_vSize.x-m_vBlockSize.x ), 0.10f, 0, true );
			else
				m_vBlockPos.x = clamp<float>( mousepos.x - m_vGrabBackup.x, 0.0f, m_vSize.x-m_vBlockSize.x );

			m_fCurPercent = clamp<float>(std::round(m_vBlockPos.x) / (m_vSize.x-m_vBlockSize.x), 0.0f, 1.0f);
		}

		// set new value
		if (m_bAnimated)
		{
			if (m_bLiveUpdate)
				setValue(lerp<float>(m_fMinValue, m_fMaxValue, m_fCurPercent), false);
			else
				m_fCurValue = lerp<float>(m_fMinValue, m_fMaxValue, m_fCurPercent);
		}
		else
			setValue(lerp<float>(m_fMinValue, m_fMaxValue, m_fCurPercent), false);

		m_bHasChanged = true;
	}
	else
	{
		// handle mouse wheel
		if (m_bMouseInside && m_bAllowMouseWheel)
		{
			int wheelDelta = engine->getMouse()->getWheelDeltaVertical();
			if (wheelDelta != 0)
			{
				const int multiplier = std::max(1, std::abs(wheelDelta) / 120);

				if (wheelDelta > 0)
					setValue(m_fCurValue + m_fKeyDelta*multiplier, m_bAnimated);
				else
					setValue(m_fCurValue - m_fKeyDelta*multiplier, m_bAnimated);
			}
		}
	}

	// handle animation value settings after mouse release
	if (!m_bActive)
	{
		if (anim->isAnimating( &m_vBlockPos.x ))
		{
			m_fCurPercent = clamp<float>(std::round(m_vBlockPos.x) / (m_vSize.x-m_vBlockSize.x), 0.0f, 1.0f);

			if (m_bLiveUpdate)
				setValue(lerp<float>(m_fMinValue, m_fMaxValue, m_fCurPercent), false);
			else
				m_fCurValue = lerp<float>(m_fMinValue, m_fMaxValue, m_fCurPercent);
		}

		if (anim->isAnimating( &m_vBlockPos.y ))
		{
			m_fCurPercent = clamp<float>(1.0f - (std::round(m_vBlockPos.y) / (m_vSize.y-m_vBlockSize.y)), 0.0f, 1.0f);

			if (m_bLiveUpdate)
				setValue(lerp<float>(m_fMinValue, m_fMaxValue, m_fCurPercent), false);
			else
				m_fCurValue = lerp<float>(m_fMinValue, m_fMaxValue, m_fCurPercent);
		}
	}
}

void CBaseUISlider::onKeyDown(KeyboardEvent &e)
{
	if (!m_bVisible) return;

	if (isMouseInside())
	{
		if (e == KEY_LEFT)
		{
			setValue(getFloat() - m_fKeyDelta, false);
			e.consume();
		}
		else if (e == KEY_RIGHT)
		{
			setValue(getFloat() + m_fKeyDelta, false);
			e.consume();
		}
	}
}

void CBaseUISlider::forceCallCallback()
{
	if (m_sliderChangeCallback != NULL)
		m_sliderChangeCallback(this);
}

void CBaseUISlider::updateBlockPos()
{
	if (!m_bHorizontal)
		m_vBlockPos.x = m_vSize.x/2.0f - m_vBlockSize.x/2.0f;
	else
		m_vBlockPos.y = m_vSize.y/2.0f - m_vBlockSize.y/2.0f;
}

CBaseUISlider *CBaseUISlider::setBounds(float minValue, float maxValue)
{
	m_fMinValue = minValue;
	m_fMaxValue = maxValue;

	m_fKeyDelta = (m_fMaxValue - m_fMinValue) / 10.0f;

	return this;
}

CBaseUISlider *CBaseUISlider::setValue(float value, bool animate)
{
	bool changeCallbackCheck = false;
	if (value != m_fCurValue)
	{
		changeCallbackCheck = true;
		m_bHasChanged = true;
	}

	m_fCurValue = clamp<float>(value, m_fMinValue, m_fMaxValue);
	float percent = getPercent();

	if (!m_bHorizontal)
	{
		if (animate)
			anim->moveQuadOut( &m_vBlockPos.y, (m_vSize.y-m_vBlockSize.y)*(1.0f-percent), 0.2f, 0, true );
		else
			m_vBlockPos.y = (m_vSize.y-m_vBlockSize.y)*(1.0f-percent);
	}
	else
	{
		if (animate)
			anim->moveQuadOut( &m_vBlockPos.x, (m_vSize.x-m_vBlockSize.x)*percent, 0.2f, 0, true );
		else
			m_vBlockPos.x = (m_vSize.x-m_vBlockSize.x)*percent;
	}

	if (changeCallbackCheck && m_sliderChangeCallback != NULL)
		m_sliderChangeCallback(this);

	updateBlockPos();

	return this;
}

CBaseUISlider *CBaseUISlider::setInitialValue(float value)
{
	m_fCurValue = clamp<float>(value, m_fMinValue, m_fMaxValue);
	float percent = getPercent();

	if (m_fCurValue == m_fMaxValue)
		percent = 1.0f;

	if (!m_bHorizontal)
		m_vBlockPos.y = (m_vSize.y-m_vBlockSize.y)*(1.0f-percent);
	else
		m_vBlockPos.x = (m_vSize.x-m_vBlockSize.x)*percent;

	updateBlockPos();

	return this;
}

void CBaseUISlider::setBlockSize(float xSize, float ySize)
{
	m_vBlockSize = Vector2(xSize, ySize);
}

float CBaseUISlider::getPercent()
{
	return clamp<float>((m_fCurValue-m_fMinValue) / (std::abs(m_fMaxValue-m_fMinValue)), 0.0f, 1.0f);
}

bool CBaseUISlider::hasChanged()
{
	if (anim->isAnimating(&m_vBlockPos.x))
		return true;
	if (m_bHasChanged)
	{
		m_bHasChanged = false;
		return true;
	}
	return false;
}

void CBaseUISlider::onFocusStolen()
{
	m_bBusy = false;
}

void CBaseUISlider::onMouseUpInside()
{
	m_bBusy = false;

	if (m_fCurValue != m_fPrevValue && m_sliderChangeCallback != NULL)
		m_sliderChangeCallback(this);
}

void CBaseUISlider::onMouseUpOutside()
{
	m_bBusy = false;

	if (m_fCurValue != m_fPrevValue && m_sliderChangeCallback != NULL)
		m_sliderChangeCallback(this);
}

void CBaseUISlider::onMouseDownInside()
{
	m_fPrevValue = m_fCurValue;

	if (McRect(m_vPos.x+m_vBlockPos.x,m_vPos.y+m_vBlockPos.y,m_vBlockSize.x,m_vBlockSize.y).contains(engine->getMouse()->getPos()))
		m_vGrabBackup = engine->getMouse()->getPos()-m_vBlockPos;
	else
		m_vGrabBackup = m_vPos + m_vBlockSize/2;

	m_bBusy = true;
}

void CBaseUISlider::onResized()
{
	setValue(getFloat(), false);
}
