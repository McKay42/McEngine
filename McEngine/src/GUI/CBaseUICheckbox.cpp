//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		a simple checkbox
//
// $NoKeywords: $
//===============================================================================//

#include "CBaseUICheckbox.h"

CBaseUICheckbox::CBaseUICheckbox(float xPos, float yPos, float xSize, float ySize, UString name, UString text) : CBaseUIButton(xPos,yPos,xSize,ySize,name,text)
{
	m_bChecked = false;
	m_changeCallback = NULL;
	CBaseUIButton::setClickCallback( fastdelegate::MakeDelegate(this, &CBaseUICheckbox::onPressed) );
}

void CBaseUICheckbox::draw(Graphics *g)
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
		drawHoverRect(g, 3);
	if (m_bActive)
		drawHoverRect(g, 6);

	// draw block
	g->drawRect(m_vPos.x+getBlockBorder(), m_vPos.y+getBlockBorder(), getBlockSize(), getBlockSize());
	if (m_bChecked)
		g->fillRect(m_vPos.x+getBlockBorder()+2, m_vPos.y+getBlockBorder()+2, getBlockSize()-3, getBlockSize()-3);

	// draw text
	if (m_font != NULL && m_sText.length() > 0)
	{
		//g->pushClipRect(McRect(m_vPos.x+1, m_vPos.y+1, m_vSize.x-1, m_vSize.y-1));

			g->setColor(m_textColor);
			g->pushTransform();
				g->translate((int)(m_vPos.x + getBlockBorder()*2 + getBlockSize()), (int)(m_vPos.y + m_vSize.y/2.0f + m_fStringHeight/2.0f));
				///g->translate(m_vPos.x + (m_vSize.x-blockBorder*2-blockSize)/2.0f - m_fStringWidth/2.0f + blockBorder*2 + blockSize, m_vPos.y + m_vSize.y/2.0f + m_fStringHeight/2.0f);
				g->translate(1,1);
				g->setColor(0xff212121);
				g->drawString(m_font, m_sText);
				g->translate(-1,-1);
				g->setColor(m_textColor);
				g->drawString(m_font, m_sText);
			g->popTransform();

		//g->popClipRect();
	}
}

CBaseUICheckbox *CBaseUICheckbox::setChecked(bool checked, bool fireChangeEvent)
{
	if (m_bChecked != checked)
	{
		m_bChecked = checked;
		if (m_changeCallback != NULL && fireChangeEvent)
			m_changeCallback(this);
	}

	return this;
}

void CBaseUICheckbox::onPressed()
{
	m_bChecked = !m_bChecked;
	if (m_changeCallback != NULL)
		m_changeCallback(this);
}

CBaseUICheckbox *CBaseUICheckbox::setSizeToContent(int horizontalBorderSize, int verticalBorderSize)
{
	// HACKHACK: broken
	CBaseUIButton::setSizeToContent(horizontalBorderSize, verticalBorderSize);
	///setSize(m_fStringWidth+2*horizontalBorderSize, m_fStringHeight + 2*verticalBorderSize);
	setSize(getBlockBorder()*2 + getBlockSize() + getBlockBorder() + m_fStringWidth + horizontalBorderSize*2, m_fStringHeight + verticalBorderSize*2);

	return this;
}

CBaseUICheckbox *CBaseUICheckbox::setWidthToContent(int horizontalBorderSize)
{
	// HACKHACK: broken
	CBaseUIButton::setWidthToContent(horizontalBorderSize);
	///setSize(m_fStringWidth+2*horizontalBorderSize, m_fStringHeight + 2*verticalBorderSize);
	setSizeX(getBlockBorder()*2 + getBlockSize() + getBlockBorder() + m_fStringWidth + horizontalBorderSize*2);

	return this;
}
