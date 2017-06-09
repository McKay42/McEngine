//================ Copyright (c) 2013, PG, All rights reserved. =================//
//
// Purpose:		a simple button
//
// $NoKeywords: $button
//===============================================================================//

#include "CBaseUIButton.h"
#include "Engine.h"
#include "Mouse.h"

#include "ResourceManager.h"

CBaseUIButton::CBaseUIButton(float xPos, float yPos, float xSize, float ySize, UString name, UString text) : CBaseUIElement(xPos, yPos, xSize, ySize, name)
{
	m_font = engine->getResourceManager()->getFont("FONT_DEFAULT");

	// settings
	m_bDrawFrame = true;
	m_bDrawBackground = true;
	m_bTextLeft = false;

	// colors
	m_frameColor = COLOR(255, 255, 255, 255);
	m_backgroundColor = COLOR(255, 0, 0, 0);
	m_textColor = 0xffffffff;
	m_textBrightColor = m_textDarkColor = 0;

	setText(text);
}

void CBaseUIButton::draw(Graphics *g)
{
	if (!m_bVisible) return;

	// draw background
	if (m_bDrawBackground)
	{
		g->setColor(m_backgroundColor);
		g->fillRect(m_vPos.x+1, m_vPos.y+1, m_vSize.x-1, m_vSize.y-1);
	}

	// draw frame
	if (m_bDrawFrame)
	{
		g->setColor(m_frameColor);
		g->drawRect(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y);
	}

	// draw hover rects
	g->setColor(m_frameColor);
	if (m_bMouseInside && m_bEnabled)
	{
		if (!m_bActive && !engine->getMouse()->isLeftDown())
			drawHoverRect(g, 3);
		else if (m_bActive)
			drawHoverRect(g, 3);
	}
	if (m_bActive && m_bEnabled)
		drawHoverRect(g, 6);

	// draw text
	drawText(g);
}

void CBaseUIButton::drawText(Graphics *g)
{
	if (m_font != NULL && m_sText.length() > 0)
	{
		g->pushClipRect(McRect(m_vPos.x+1, m_vPos.y+1, m_vSize.x-1, m_vSize.y-1));

			g->setColor(m_textColor);
			g->pushTransform();
				g->translate((int)(m_vPos.x + m_vSize.x/2.0f - m_fStringWidth/2.0f), (int)(m_vPos.y + m_vSize.y/2.0f + m_fStringHeight/2.0f));
				g->translate(1,1);

				if (m_textDarkColor != 0)
					g->setColor(m_textDarkColor);
				else
					g->setColor(INVERSE_COLOR(m_textColor));

				g->drawString(m_font, m_sText);

			g->popTransform();
			g->pushTransform();
				g->translate((int)(m_vPos.x + m_vSize.x/2.0f - m_fStringWidth/2.0f), (int)(m_vPos.y + m_vSize.y/2.0f + m_fStringHeight/2.0f));

				if (m_textBrightColor != 0)
					g->setColor(m_textBrightColor);
				else
					g->setColor(m_textColor);

				g->drawString(m_font, m_sText);

			g->popTransform();

		g->popClipRect();
	}
}

void CBaseUIButton::drawHoverRect(Graphics *g, int distance)
{
	g->drawLine(m_vPos.x, m_vPos.y-distance, m_vPos.x+m_vSize.x+1, m_vPos.y-distance);
	g->drawLine(m_vPos.x, m_vPos.y+m_vSize.y+distance, m_vPos.x+m_vSize.x+1, m_vPos.y+m_vSize.y+distance);
	g->drawLine(m_vPos.x-distance, m_vPos.y, m_vPos.x-distance, m_vPos.y+m_vSize.y+1);
	g->drawLine(m_vPos.x+m_vSize.x+distance, m_vPos.y, m_vPos.x+m_vSize.x+distance, m_vPos.y+m_vSize.y+1);
}

void CBaseUIButton::update()
{
	CBaseUIElement::update();
}

void CBaseUIButton::onMouseUpInside()
{
	onClicked();
}

void CBaseUIButton::onClicked()
{
	if (m_clickCallback != NULL)
		m_clickCallback(this);
	if (m_clickVoidCallback != NULL)
		m_clickVoidCallback();
}

void CBaseUIButton::updateStringMetrics()
{
	if (m_font != NULL)
	{
		m_fStringHeight = m_font->getHeight();

		if (m_bTextLeft)
			m_fStringWidth = m_vSize.x - 4;
		else
			m_fStringWidth = m_font->getStringWidth(m_sText);
	}
}

