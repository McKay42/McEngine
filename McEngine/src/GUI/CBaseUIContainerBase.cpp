/*
 * CBaseUIContainerBase.cpp
 *
 *  Created on: May 31, 2017
 *      Author: Psy
 */

#include "CBaseUIContainerBase.h"
#include "Engine.h"

CBaseUIContainerBase::CBaseUIContainerBase(UString name) : CBaseUIElement(0, 0, 0, 0, name)
{

}

CBaseUIContainerBase::~CBaseUIContainerBase()
{
	clear();
}

void CBaseUIContainerBase::clear()
{
	for (int i=0; i<m_vElements.size(); i++)
		delete m_vElements[i];

	m_vElements = std::vector<CBaseUIElement*>();
}

void CBaseUIContainerBase::empty()
{
	for (int i=0; i<m_vElements.size(); i++)
		m_vElements[i]->setParent(nullptr);

	m_vElements = std::vector<CBaseUIElement*>();
}

CBaseUIContainerBase *CBaseUIContainerBase::addElement(CBaseUIElement *element)
{
	if (element == NULL) return this;

	element->setParent(this);
	m_vElements.push_back(element);
	updateElement(element);

	return this;
}

CBaseUIContainerBase *CBaseUIContainerBase::addElementBack(CBaseUIElement *element)
{
	if (element == NULL) return this;

	element->setParent(this);
	m_vElements.insert(m_vElements.begin(), element);
	updateElement(element);

	return this;
}
CBaseUIContainerBase *CBaseUIContainerBase::insertElement(CBaseUIElement *element, CBaseUIElement *index){

	if (element == NULL || index == NULL) return this;

	for (int i=0; i<m_vElements.size(); i++)
	{
		if (m_vElements[i] == index)
		{
			element->setParent(this);
			m_vElements.insert(m_vElements.begin() + clamp<int>(i, 0, m_vElements.size()), element);
			updateElement(element);
			return this;
		}
	}

	debugLog("Warning: CBaseUIContainerBase::insertSlot() couldn't find index\n");

	return this;
}

CBaseUIContainerBase *CBaseUIContainerBase::insertElementBack(CBaseUIElement *element, CBaseUIElement *index)
{
	if (element == NULL || index == NULL) return this;

	for (int i=0; i<m_vElements.size(); i++)
	{
		if (m_vElements[i] == index)
		{
			element->setParent(this);
			m_vElements.insert(m_vElements.begin() + clamp<int>(i+1, 0, m_vElements.size()), element);
			updateElement(element);
			return this;
		}
	}

	debugLog("Warning: CBaseUIContainerBase::insertSlotBack() couldn't find index\n");

	return this;
}

void CBaseUIContainerBase::removeElement(CBaseUIElement *element)
{
	for (int i=0; i<m_vElements.size(); i++)
	{
		if (m_vElements[i] == element)
		{
			m_vElements[i]->setParent(nullptr);
			m_vElements.erase(m_vElements.begin()+i);
			updateLayout();
			return;
		}
	}

	debugLog("Warning: CBaseUIContainerBase::removeSlot() couldn't find element\n");
}

void CBaseUIContainerBase::deleteElement(CBaseUIElement *element)
{
	for (int i=0; i<m_vElements.size(); i++)
	{
		if (m_vElements[i] == element)
		{
			delete m_vElements[i];
			m_vElements.erase(m_vElements.begin()+i);
			updateLayout();
			return;
		}
	}

	debugLog("Warning: CBaseUIContainerBase::removeSlot() couldn't find element\n");
}

CBaseUIElement *CBaseUIContainerBase::getElementByName(UString name)
{
	for (int i=0; i<m_vElements.size(); i++)
	{
		if (m_vElements[i]->getName() == name)
			return m_vElements[i];
	}

	debugLog("Error: CBaseUIContainerBase::getSlotByElementName() \"%s\" does not exist!!!\n", name.toUtf8());
	return NULL;
}

void CBaseUIContainerBase::draw(Graphics *g)
{
	if (!m_bVisible) return;

	for (int i=0; i<m_vElements.size(); i++)
	{
		m_vElements[i]->draw(g);
	}
}

void CBaseUIContainerBase::update()
{
	if (!m_bVisible) return;

	for (int i=0; i<m_vElements.size(); i++)
		m_vElements[i]->update();
}
