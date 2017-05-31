/*
 * CBaseUIContainerBase.cpp
 *
 *  Created on: May 31, 2017
 *      Author: Psy
 */

#include "CBaseUIContainerBase.h"
#include "Engine.h"

CBaseUIContainerBase::CBaseUIContainerBase(UString name) : CBaseUIElement(0, 0, 0, 0, name) {

}

CBaseUIContainerBase::~CBaseUIContainerBase() {

}

void CBaseUIContainerBase::clear()
{
	for (int i=0; i<m_vSlots.size(); i++)
	{
		delete m_vSlots[i]->element;
		delete m_vSlots[i];
	}

	m_vSlots = std::vector<Slot*>();
}

void CBaseUIContainerBase::empty()
{
	for (int i=0; i<m_vSlots.size(); i++)
		m_vSlots[i]->element->setParent(nullptr);

	m_vSlots = std::vector<Slot*>();
}

void CBaseUIContainerBase::addSlot(Slot *slot)
{
	if (slot == NULL) return;

	slot->element->setParent(this);
	m_vSlots.push_back(slot);
	updateSlot(slot);
}

void CBaseUIContainerBase::addSlot(CBaseUIElement *element, bool scaleByHeightOnly)
{
	if (element == NULL) return;

	Slot *slot = new Slot;
	element->setParent(this);
	slot->element = element;
	slot->scaleByHeightOnly = scaleByHeightOnly;
	m_vSlots.push_back(slot);
	updateSlot(slot);
}

void CBaseUIContainerBase::addSlotBack(Slot *slot)
{
	if (slot == NULL) return;

	slot->element->setParent(this);
	m_vSlots.insert(m_vSlots.begin(), slot);
	updateSlot(slot);
}

void CBaseUIContainerBase::addSlotBack(CBaseUIElement *element, bool scaleByHeightOnly)
{
	if (element == NULL) return;

	Slot *slot = new Slot;
	element->setParent(this);
	slot->element = element;
	slot->scaleByHeightOnly = scaleByHeightOnly;
	m_vSlots.insert(m_vSlots.begin(), slot);
	updateSlot(slot);
}

void CBaseUIContainerBase::insertSlot(Slot *slot, Slot *index){

	if (slot == NULL || index == NULL) return;

	for (int i=0; i<m_vSlots.size(); i++)
	{
		if (m_vSlots[i] == index)
		{
			slot->element->setParent(this);

			m_vSlots.insert(m_vSlots.begin() + clamp<int>(i, 0, m_vSlots.size()), slot);
			updateSlot(slot);
			return;
		}
	}

	debugLog("Warning: CBaseUIContainerBase::insertSlot() couldn't find index\n");
}

void CBaseUIContainerBase::insertSlot(Slot *index, CBaseUIElement* element, bool scaleByHeightOnly)
{
	if (element == NULL || index == NULL) return;

	for (int i=0; i<m_vSlots.size(); i++)
	{
		if (m_vSlots[i] == index)
		{
			Slot *slot = new Slot;
			slot->element = element;
			slot->scaleByHeightOnly = scaleByHeightOnly;
			slot->element->setParent(this);
			m_vSlots.insert(m_vSlots.begin() + clamp<int>(i, 0, m_vSlots.size()), slot);
			updateSlot(slot);
			return;
		}
	}

	debugLog("Warning: CBaseUIContainerBase::insertSlot() couldn't find index\n");
}

void CBaseUIContainerBase::insertSlotBack(Slot *index, Slot *slot)
{
	if (slot == NULL || index == NULL) return;

	for (int i=0; i<m_vSlots.size(); i++)
	{
		if (m_vSlots[i] == index)
		{
			slot->element->setParent(this);
			m_vSlots.insert(m_vSlots.begin() + clamp<int>(i+1, 0, m_vSlots.size()), slot);
			updateSlot(slot);
			return;
		}
	}

	debugLog("Warning: CBaseUIContainerBase::insertSlotBack() couldn't find index\n");
}

void CBaseUIContainerBase::insertSlotBack(Slot *index, CBaseUIElement *element, bool scaleByHeightOnly)
{
	if (element == NULL || index == NULL) return;

	for (int i=0; i<m_vSlots.size(); i++)
	{
		if (m_vSlots[i] == index)
		{
			Slot *slot = new Slot;
			slot->element = element;
			slot->scaleByHeightOnly = scaleByHeightOnly;
			slot->element->setParent(this);
			m_vSlots.insert(m_vSlots.begin() + clamp<int>(i+1, 0, m_vSlots.size()), slot);
			updateSlot(slot);
			return;
		}
	}

	debugLog("Warning: CBaseUIContainerBase::insertSlotBack() couldn't find index\n");
}

void CBaseUIContainerBase::removeSlot(Slot *slot)
{
	for (int i=0; i<m_vSlots.size(); i++)
	{
		if (m_vSlots[i] == slot)
		{
			m_vSlots[i]->element->setParent(nullptr);
			m_vSlots.erase(m_vSlots.begin()+i);
			updateLayout();
			return;
		}
	}

	debugLog("Warning: CBaseUIContainerBase::removeSlot() couldn't find element\n");
}

void CBaseUIContainerBase::deleteSlot(Slot *slot)
{
	for (int i=0; i<m_vSlots.size(); i++)
	{
		if (m_vSlots[i] == slot)
		{
			delete m_vSlots[i]->element;
			delete m_vSlots[i];
			m_vSlots.erase(m_vSlots.begin()+i);
			updateLayout();
			return;
		}
	}

	debugLog("Warning: CBaseUIContainerBase::removeSlot() couldn't find element\n");
}

CBaseUIContainerBase::Slot *CBaseUIContainerBase::getSlotByElementName(UString name)
{
	for (int i=0; i<m_vSlots.size(); i++)
	{
		if (m_vSlots[i]->element->getName() == name)
			return m_vSlots[i];
	}

	debugLog("Error: CBaseUIContainerBase::getSlotByElementName() \"%s\" does not exist!!!\n", name.toUtf8());
	return NULL;
}

std::vector<CBaseUIElement*> CBaseUIContainerBase::getAllElements()
{
	std::vector<CBaseUIElement*> elements;

	for (int i=0; i<m_vSlots.size(); i++)
		elements.push_back(m_vSlots[i]->element);

	return elements;
}

void CBaseUIContainerBase::draw(Graphics *g)
{
	if (!m_bVisible) return;

	for (int i=0; i<m_vSlots.size(); i++)
	{
		m_vSlots[i]->element->draw(g);
	}
}

void CBaseUIContainerBase::update()
{
	if (!m_bVisible) return;

	for (int i=0; i<m_vSlots.size(); i++)
		m_vSlots[i]->element->update();
}
