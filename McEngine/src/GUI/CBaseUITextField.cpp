//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		a not so simple textfield
//
// $NoKeywords: $
//===============================================================================//

// TODO: finish this

#include "CBaseUITextField.h"

#include "Engine.h"
#include "ResourceManager.h"

#include "CBaseUIContainer.h"

CBaseUITextField::CBaseUITextField(float xPos, float yPos, float xSize, float ySize, UString name, UString text) : CBaseUIScrollView(xPos,yPos,xSize,ySize,name)
{
	m_textObject = new TextObject(2, 1, xSize, ySize, text);
	m_textObject->setParentSize(m_vSize);
	getContainer()->addBaseUIElement(m_textObject);
	setScrollSizeToContent(0);
}

void CBaseUITextField::draw(Graphics *g)
{
	CBaseUIScrollView::draw(g);
	if (!m_bVisible) return;

	//printf("scrollsize.y = %f, size.y = %f\n", getScrollSize().y, getSize().y);
}

void CBaseUITextField::onResized()
{
	CBaseUIScrollView::onResized();
	m_textObject->setParentSize(m_vSize);
	//m_textObject->setSize(m_vSize);
	scrollToX(getScrollPosX());
	scrollToY(getScrollPosY());
	setScrollSizeToContent(0);
}

CBaseUITextField *CBaseUITextField::append(UString text)
{
	UString oldText = m_textObject->getText();
	oldText.append(text);
	m_textObject->setText(oldText);
	return this;
}


//*********************************************************************************//
//								   TextObject									   //
//*********************************************************************************//

CBaseUITextField::TextObject::TextObject(float xPos, float yPos, float xSize, float ySize, UString text) : CBaseUIElement(xPos,yPos,xSize,ySize,"")
{
	m_font = engine->getResourceManager()->getFont("FONT_DEFAULT");

	// colors
	m_textColor = 0xffffffff;

	setText(text);
}

void CBaseUITextField::TextObject::draw(Graphics *g)
{
	if (m_font == NULL || m_sText.length() == 0) return;

	//g->setColor(0xffffffff);
	//g->drawRect(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y);

	g->setColor(m_textColor);
	g->pushClipRect(McRect(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y));

		std::vector<UString> words = m_sText.split(" ");
		float spaceWidth = m_font->getStringWidth(" ");
		int border = 0;
		int lineSpacing = 4;
		float width = 0.0f;

		g->pushTransform();
		g->translate(m_vPos.x+border, m_vPos.y + m_fStringHeight + border);

			for (int i=0; i<words.size(); i++)
			{
				float curWordWidth = m_font->getStringWidth(words[i]);

				bool newLine = words[i].find("\n") != -1;
				if (((width + curWordWidth+spaceWidth) > m_vParentSize.x-border && i != 0) || newLine)
				{
					g->translate(-width, m_fStringHeight+border+lineSpacing);
					width = 0.0f;
					if (newLine)
						continue;
				}
				width += curWordWidth+spaceWidth;

				g->drawString(m_font, words[i]);
				g->translate(curWordWidth+spaceWidth, 0);
			}

		g->popTransform();

	g->popClipRect();
}

void CBaseUITextField::TextObject::updateStringMetrics()
{
	if (m_font == NULL) return;

	m_fStringHeight = m_font->getHeight();
}

CBaseUIElement *CBaseUITextField::TextObject::setText(UString text)
{
	m_sText = text;
	updateStringMetrics();
	return this;
}

void CBaseUITextField::TextObject::onResized()
{
	if (m_sText.length() < 1) return;

	std::vector<UString> words = m_sText.split(" ");
	if (words.size() < 1) return;

	float spaceWidth = m_font->getStringWidth(" ");
	int border = 0;
	float width = 0.0f;
	int lineSpacing = 4;
	float height = m_fStringHeight+border+lineSpacing;

	int oldSizeX = -1;
	int oldSizeY = -1;

	for (int i=0; i<words.size(); i++)
	{
		float curWordWidth = m_font->getStringWidth(words[i]);

		if (curWordWidth > m_vParentSize.x && curWordWidth > oldSizeX)
			oldSizeX = curWordWidth;

		bool newLine = words[i].find("\n") != -1;
		if (((width + curWordWidth+spaceWidth) > m_vParentSize.x-border && i != 0) || (newLine && i != (int)(words.size()-1)))
		{
			width = 0.0f;
			height += m_fStringHeight+border+lineSpacing;
		}
		width += curWordWidth+spaceWidth;
	}

	if (height > m_vParentSize.y && height > oldSizeY)
		oldSizeY = height;

	m_vSize.x = oldSizeX == -1 ? m_vParentSize.x+1 : oldSizeX;
	m_vSize.y = oldSizeY == -1 ? m_vParentSize.y+1 : oldSizeY;
}
