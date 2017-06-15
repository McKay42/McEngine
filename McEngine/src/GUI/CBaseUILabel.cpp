//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		a simple label
//
// $NoKeywords: $
//===============================================================================//

#include "CBaseUILabel.h"

#include "Engine.h"
#include "ResourceManager.h"

CBaseUILabel::CBaseUILabel(float xPos, float yPos, float xSize, float ySize, UString name, UString text) : CBaseUIElement(xPos, yPos, xSize, ySize, name)
{
	m_font = engine->getResourceManager()->getFont("FONT_DEFAULT");
	setText(text);

	// colors
	m_frameColor = 0xffffffff;
	m_backgroundColor = 0xff000000;
	m_textColor = 0xffffffff;

	// settings
	m_bDrawFrame = true;
	m_bDrawBackground = true;
	m_bCenterText = false;
}

void CBaseUILabel::draw(Graphics *g)
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

	// draw text
	drawText(g);
}

void CBaseUILabel::drawText(Graphics *g)
{
	if (m_font != NULL && m_sText.length() > 0)
	{
		float xPosAdd = 0;
		switch (m_textJustification)
		{
		case TEXT_JUSTIFICATION_LEFT:
			break;
		case TEXT_JUSTIFICATION_CENTERED:
			xPosAdd = m_vSize.x/2.0f - m_fStringWidth/2.0f;
			break;
		case TEXT_JUSTIFICATION_RIGHT:
			xPosAdd = m_vSize.x - m_fStringWidth;
			break;
		}

		//g->pushClipRect(McRect(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y));

			g->setColor(m_textColor);
			g->pushTransform();
				g->translate((int)(m_vPos.x + (m_bCenterText ? + m_vSize.x/2.0f - m_fStringWidth/2.0f : xPosAdd)), (int)(m_vPos.y + m_vSize.y/2.0f + m_fStringHeight/2.0f));
				g->drawString(m_font, m_sText);
			g->popTransform();

		//g->popClipRect();
	}
}

void CBaseUILabel::update()
{
	CBaseUIElement::update();
}

void CBaseUILabel::updateStringMetrics()
{
	if (m_font != NULL)
	{
		m_fStringWidth = m_font->getStringWidth(m_sText);
		m_fStringHeight = m_font->getHeight();
	}
}
