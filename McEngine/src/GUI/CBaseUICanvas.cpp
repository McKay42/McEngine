/*
 * CBaseUICanvas.cpp
 *
 *  Created on: May 28, 2017
 *      Author: Psy
 */

#include "CBaseUICanvas.h"
#include "Engine.h"

CBaseUICanvas::CBaseUICanvas(float xPos, float yPos, float xSize, float ySize, UString name) : CBaseUIContainerBase(name)
{
	m_vPos.x = xPos;
	m_vPos.y = yPos;
	m_vmPos = m_vPos;
	m_vSize.x = xSize;
	m_vSize.y = ySize;
	m_vmSize = m_vSize;
}

CBaseUICanvas::~CBaseUICanvas()
{
}

void CBaseUICanvas::drawDebug(Graphics *g, Color color)
{
	if (!m_bVisible) return;

	// Draw debug box for this canvas
	g->setColor(color);
	g->drawRect(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y);
}

void CBaseUICanvas::updateLayout()
{
	if (m_parent != nullptr)
		m_parent->updateLayout();

	for (int i=0; i<m_vElements.size(); i++)
	{
		if (!m_vElements[i]->isScaledByHeightOnly())
			m_vElements[i]->setSizeAbsolute(m_vElements[i]->getRelSize() * m_vSize);
		else
			m_vElements[i]->setSizeAbsolute(m_vElements[i]->getRelSize().x * ((m_vSize.y / 9) * 16), m_vElements[i]->getRelSize().y * m_vSize.y);

		m_vElements[i]->setPosAbsolute(m_vPos + m_vElements[i]->getRelPos() * m_vSize);
	}
}

void CBaseUICanvas::updateElement(CBaseUIElement *element)
{
	if (element->isScaledByHeightOnly())
		element->setSizeAbsolute(element->getRelSize() * m_vSize);
	else
		element->setSizeAbsolute(element->getRelSize().x * ((m_vSize.y / 9) * 16), element->getRelSize().y * m_vSize.y);

	element->setPosAbsolute(m_vPos + element->getRelPos() * m_vSize);
}

void CBaseUICanvas::onMoved()
{
	for (int i=0; i<m_vElements.size(); i++)
	{
		if (!m_vElements[i]->isScaledByHeightOnly())
			m_vElements[i]->setSizeAbsolute(m_vElements[i]->getRelSize() * m_vSize);
		else
			m_vElements[i]->setSizeAbsolute(m_vElements[i]->getRelSize().x * ((m_vSize.y / 9) * 16), m_vElements[i]->getRelSize().y * m_vSize.y);

		m_vElements[i]->setPosAbsolute(m_vPos + m_vElements[i]->getRelPos() * m_vSize);
	}
}

void CBaseUICanvas::onResized()
{
	for (int i=0; i<m_vElements.size(); i++)
	{
		if (!m_vElements[i]->isScaledByHeightOnly())
			m_vElements[i]->setSizeAbsolute(m_vElements[i]->getRelSize() * m_vSize);
		else
			m_vElements[i]->setSizeAbsolute(m_vElements[i]->getRelSize().x * ((m_vSize.y / 9) * 16), m_vElements[i]->getRelPos().y * m_vSize.y);

		m_vElements[i]->setPosAbsolute(m_vPos + m_vElements[i]->getRelPos() * m_vSize);
	}
}
