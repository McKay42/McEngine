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
	clear();
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

	for (int i=0; i<m_vSlots.size(); i++)
	{
		if (!m_vSlots[i]->scaleByHeightOnly)
			m_vSlots[i]->element->setSizeAbsolute(m_vSlots[i]->element->getRelSize() * m_vSize);
		else
			m_vSlots[i]->element->setSizeAbsolute(m_vSlots[i]->element->getRelSize().x * ((m_vSize.y / 9) * 16), m_vSlots[i]->element->getRelSize().y * m_vSize.y);

		m_vSlots[i]->element->setPosAbsolute(m_vPos + m_vSlots[i]->element->getRelPos() * m_vSize);
	}
}

void CBaseUICanvas::updateSlot(Slot *slot)
{
	if (dynamic_cast<CBaseUIContainerBase *>(slot->element) != NULL)
	{
		updateLayout();
		return;
	}

	if (slot->scaleByHeightOnly)
		slot->element->setSizeAbsolute(slot->element->getRelSize() * m_vSize);
	else
		slot->element->setSizeAbsolute(slot->element->getRelSize().x * ((m_vSize.y / 9) * 16), slot->element->getRelSize().y * m_vSize.y);

	slot->element->setPosAbsolute(m_vPos + slot->element->getRelPos() * m_vSize);
}

void CBaseUICanvas::onMoved()
{
	for (int i=0; i<m_vSlots.size(); i++)
	{
		if (!m_vSlots[i]->scaleByHeightOnly)
			m_vSlots[i]->element->setSizeAbsolute(m_vSlots[i]->element->getRelSize() * m_vSize);
		else
			m_vSlots[i]->element->setSizeAbsolute(m_vSlots[i]->element->getRelSize().x * ((m_vSize.y / 9) * 16), m_vSlots[i]->element->getRelSize().y * m_vSize.y);

		m_vSlots[i]->element->setPosAbsolute(m_vPos + m_vSlots[i]->element->getRelPos() * m_vSize);
	}
}

void CBaseUICanvas::onResized()
{
	for (int i=0; i<m_vSlots.size(); i++)
	{
		if (!m_vSlots[i]->scaleByHeightOnly)
			m_vSlots[i]->element->setSizeAbsolute(m_vSlots[i]->element->getRelSize() * m_vSize);
		else
			m_vSlots[i]->element->setSizeAbsolute(m_vSlots[i]->element->getRelSize().x * ((m_vSize.y / 9) * 16), m_vSlots[i]->element->getRelPos().y * m_vSize.y);

		m_vSlots[i]->element->setPosAbsolute(m_vPos + m_vSlots[i]->element->getRelPos() * m_vSize);
	}
}
