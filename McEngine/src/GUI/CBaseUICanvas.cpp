/*
 * CBaseUICanvas.cpp
 *
 *  Created on: May 28, 2017
 *      Author: Psy
 */

#include "CBaseUICanvas.h"
#include "Engine.h"

CBaseUICanvas::CBaseUICanvas(float xPos, float yPos, float xSize, float ySize, UString name) : CBaseUIElement(xPos, yPos, xSize, ySize, name){;}

CBaseUICanvas::~CBaseUICanvas()
{
	clear();
}

void CBaseUICanvas::clear()
{
	for (int i=0; i<m_vSlots.size(); i++)
	{
		delete m_vSlots[i]->element;
		delete m_vSlots[i];
	}

	m_vSlots = std::vector<Slot*>();
}

void CBaseUICanvas::empty()
{
	for (int i=0; i<m_vSlots.size(); i++)
		m_vSlots[i]->element->setParent(nullptr);

	m_vSlots = std::vector<Slot*>();
}

void CBaseUICanvas::addSlot(Slot *slot)
{
	if (slot == NULL) return;

	slot->element->setParent(this);

	if (!slot->scaleByHeightOnly)
		slot->element->setSizeAbsolute(slot->element->getRelSize() * m_vSize);
	else
		slot->element->setSizeAbsolute(slot->element->getRelSize().x * ((m_vSize.y / 9) * 16), slot->element->getRelSize().y * m_vSize.y);

	slot->element->setPosAbsolute(slot->element->getRelPos() * m_vSize);

	m_vSlots.push_back(slot);
}

void CBaseUICanvas::addSlot(CBaseUIElement *element, bool scaleByHeightOnly)
{
	if (element == NULL) return;

	Slot *slot = new Slot;
	element->setParent(this);
	slot->element = element;
	slot->scaleByHeightOnly = scaleByHeightOnly;

	if (!slot->scaleByHeightOnly)
		slot->element->setSizeAbsolute(slot->element->getRelSize() * m_vSize);
	else
		slot->element->setSizeAbsolute(slot->element->getRelSize().x * ((m_vSize.y / 9) * 16), slot->element->getRelSize().y * m_vSize.y);

	slot->element->setPosAbsolute(slot->element->getRelPos() * m_vSize);

	m_vSlots.push_back(slot);
}

void CBaseUICanvas::addSlotBack(Slot *slot)
{
	if (slot == NULL) return;

	slot->element->setParent(this);
	slot->element->setSizeAbsolute(slot->element->getRelSize().x * ((m_vSize.y / 9) * 16), slot->element->getRelSize().y * m_vSize.y);
	slot->element->setPosAbsolute(slot->element->getRelPos() * m_vSize);

	m_vSlots.insert(m_vSlots.begin(), slot);
}

void CBaseUICanvas::addSlotBack(CBaseUIElement *element, bool scaleByHeightOnly)
{
	if (element == NULL) return;

	Slot *slot = new Slot;
	element->setParent(this);
	slot->element = element;
	slot->scaleByHeightOnly = scaleByHeightOnly;

	if (!slot->scaleByHeightOnly)
		slot->element->setSizeAbsolute(slot->element->getRelSize() * m_vSize);
	else
		slot->element->setSizeAbsolute(slot->element->getRelSize().x * ((m_vSize.y / 9) * 16), slot->element->getRelSize().y * m_vSize.y);

	slot->element->setPosAbsolute(slot->element->getRelPos() * m_vSize);

	m_vSlots.insert(m_vSlots.begin(), slot);
}

void CBaseUICanvas::insertSlot(Slot *slot, Slot *index){

	if (slot == NULL || index == NULL) return;

	for (int i=0; i<m_vSlots.size(); i++)
	{
		if (m_vSlots[i] == index)
		{
			slot->element->setParent(this);

			if (!slot->scaleByHeightOnly)
				slot->element->setSizeAbsolute(slot->element->getRelSize() * m_vSize);
			else
				slot->element->setSizeAbsolute(slot->element->getRelSize().x * ((m_vSize.y / 9) * 16), slot->element->getRelSize().y * m_vSize.y);

			slot->element->setPosAbsolute(slot->element->getRelPos() * m_vSize);

			m_vSlots.insert(m_vSlots.begin() + clamp<int>(i, 0, m_vSlots.size()), slot);
			return;
		}
	}

	debugLog("Warning: CBaseUICanvas::insertSlot() couldn't find index\n");
}


void CBaseUICanvas::insertSlot(CBaseUIElement *element, Slot *index)
{
	if (element == NULL || index == NULL) return;

	Slot *slot = new Slot;
	slot->element = element;

	for (int i=0; i<m_vSlots.size(); i++)
	{
		if (m_vSlots[i] == index)
		{
			slot->element->setParent(this);
			slot->element->setSizeAbsolute(slot->element->getRelSize().x * ((m_vSize.y / 9) * 16), slot->element->getRelSize().y * m_vSize.y);
			slot->element->setPosAbsolute(slot->element->getRelPos() * m_vSize);

			m_vSlots.insert(m_vSlots.begin() + clamp<int>(i, 0, m_vSlots.size()), slot);
			return;
		}
	}

	debugLog("Warning: CBaseUICanvas::insertSlot() couldn't find index\n");
}

void CBaseUICanvas::insertSlot(CBaseUIElement* element, bool scaleByHeightOnly, Slot *index)
{
	if (element == NULL || index == NULL) return;

	Slot *slot = new Slot;
	slot->element = element;
	slot->scaleByHeightOnly = scaleByHeightOnly;

	for (int i=0; i<m_vSlots.size(); i++)
	{
		if (m_vSlots[i] == index)
		{
			slot->element->setParent(this);

			if (!slot->scaleByHeightOnly)
				slot->element->setSizeAbsolute(slot->element->getRelSize() * m_vSize);
			else
				slot->element->setSizeAbsolute(slot->element->getRelSize().x * ((m_vSize.y / 9) * 16), slot->element->getRelSize().y * m_vSize.y);

			slot->element->setPosAbsolute(slot->element->getRelPos() * m_vSize);

			m_vSlots.insert(m_vSlots.begin() + clamp<int>(i, 0, m_vSlots.size()), slot);
			return;
		}
	}

	debugLog("Warning: CBaseUICanvas::insertSlot() couldn't find index\n");
}

void CBaseUICanvas::insertSlotBack(Slot *slot, Slot *index)
{
	if (slot == NULL || index == NULL) return;

	for (int i=0; i<m_vSlots.size(); i++)
	{
		if (m_vSlots[i] == index)
		{
			slot->element->setParent(this);

			if (!slot->scaleByHeightOnly)
				slot->element->setSizeAbsolute(slot->element->getRelSize() * m_vSize);
			else
				slot->element->setSizeAbsolute(slot->element->getRelSize().x * ((m_vSize.y / 9) * 16), slot->element->getRelSize().y * m_vSize.y);

			slot->element->setPosAbsolute(slot->element->getRelPos() * m_vSize);

			m_vSlots.insert(m_vSlots.begin() + clamp<int>(i+1, 0, m_vSlots.size()), slot);
			return;
		}
	}

	debugLog("Warning: CBaseUICanvas::insertSlotBack() couldn't find index\n");
}

void CBaseUICanvas::insertSlotBack(CBaseUIElement *element, Slot *index)
{
	if (element == NULL || index == NULL) return;

	Slot *slot = new Slot;
	slot->element = element;

	for (int i=0; i<m_vSlots.size(); i++)
	{
		if (m_vSlots[i] == index)
		{
			slot->element->setParent(this);
			slot->element->setSizeAbsolute(slot->element->getRelSize().x * ((m_vSize.y / 9) * 16), slot->element->getRelSize().y * m_vSize.y);
			slot->element->setPosAbsolute(slot->element->getRelPos() * m_vSize);

			m_vSlots.insert(m_vSlots.begin() + clamp<int>(i+1, 0, m_vSlots.size()), slot);
			return;
		}
	}

	debugLog("Warning: CBaseUICanvas::insertSlotBack() couldn't find index\n");
}

void CBaseUICanvas::insertSlotBack(CBaseUIElement *element, bool scaleByHeightOnly, Slot *index)
{
	if (element == NULL || index == NULL) return;

	Slot *slot = new Slot;
	slot->element = element;
	slot->scaleByHeightOnly = scaleByHeightOnly;

	for (int i=0; i<m_vSlots.size(); i++)
	{
		if (m_vSlots[i] == index)
		{
			slot->element->setParent(this);

			if (!slot->scaleByHeightOnly)
				slot->element->setSizeAbsolute(slot->element->getRelSize() * m_vSize);
			else
				slot->element->setSizeAbsolute(slot->element->getRelSize().x * ((m_vSize.y / 9) * 16), slot->element->getRelSize().y * m_vSize.y);

			slot->element->setPosAbsolute(slot->element->getRelPos() * m_vSize);

			m_vSlots.insert(m_vSlots.begin() + clamp<int>(i+1, 0, m_vSlots.size()), slot);
			return;
		}
	}

	debugLog("Warning: CBaseUICanvas::insertSlotBack() couldn't find index\n");
}

void CBaseUICanvas::removeSlot(Slot *slot)
{
	for (int i=0; i<m_vSlots.size(); i++)
	{
		if (m_vSlots[i] == slot)
		{
			m_vSlots[i]->element->setParent(nullptr);
			m_vSlots.erase(m_vSlots.begin()+i);
			return;
		}
	}

	debugLog("Warning: CBaseUICanvas::removeSlot() couldn't find element\n");
}

void CBaseUICanvas::deleteSlot(Slot *slot)
{
	for (int i=0; i<m_vSlots.size(); i++)
	{
		if (m_vSlots[i] == slot)
		{
			delete m_vSlots[i]->element;
			delete m_vSlots[i];
			m_vSlots.erase(m_vSlots.begin()+i);
			return;
		}
	}

	debugLog("Warning: CBaseUICanvas::removeSlot() couldn't find element\n");
}

CBaseUICanvas::Slot *CBaseUICanvas::getSlotByElementName(UString name)
{
	for (int i=0; i<m_vSlots.size(); i++)
	{
		if (m_vSlots[i]->element->getName() == name)
			return m_vSlots[i];
	}

	debugLog("Error: CBaseUICanvas::getSlotByElementName() \"%s\" does not exist!!!\n", name.toUtf8());
	return NULL;
}

std::vector<CBaseUIElement*> CBaseUICanvas::getAllElements()
{
	std::vector<CBaseUIElement*> elements;

	for (int i=0; i<m_vSlots.size(); i++)
	{
		elements.push_back(m_vSlots[i]->element);
	}

	return elements;
}

void CBaseUICanvas::draw(Graphics *g)
{
	if (!m_bVisible) return;

	for (int i=0; i<m_vSlots.size(); i++)
		m_vSlots[i]->element->draw(g);
}

void CBaseUICanvas::drawDebug(Graphics *g, Color color)
{
	if (!m_bVisible) return;

	// Draw debug box for this canvas
	g->setColor(color);
	g->drawRect(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y);
}

void CBaseUICanvas::update()
{
	if (!m_bVisible) return;

	for (int i=0; i<m_vSlots.size(); i++)
	{
		m_vSlots[i]->element->update();
	}
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
