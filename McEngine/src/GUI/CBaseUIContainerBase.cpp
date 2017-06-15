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
	m_bClipping = false;
}

CBaseUIContainerBase::~CBaseUIContainerBase()
{

}

void CBaseUIContainerBase::empty()
{
	for (int i=0; i<m_vElements.size(); i++)
		m_vElements[i]->setParent(nullptr);

	m_vElements = std::vector<std::shared_ptr<CBaseUIElement>>();
}

CBaseUIContainerBase *CBaseUIContainerBase::addElement(CBaseUIElement *element, bool back)
{
	if (element == NULL) return this;

	element->setParent(this);
	std::shared_ptr<CBaseUIElement> buffer(element);

	if (back)
		m_vElements.insert(m_vElements.begin(), buffer);
	else
		m_vElements.push_back(buffer);

	updateElement(element);
	return this;
}

CBaseUIContainerBase *CBaseUIContainerBase::addElement(std::shared_ptr<CBaseUIElement> element, bool back)
{
	if (element == NULL || element.get() == nullptr) return this;

	element->setParent(this);

	if (back)
		m_vElements.insert(m_vElements.begin(), element);
	else
		m_vElements.push_back(element);

	updateElement(element.get());
	return this;
}

CBaseUIContainerBase *CBaseUIContainerBase::insertElement(CBaseUIElement *element, CBaseUIElement *index, bool back)
{

	if (element == nullptr || index == nullptr) return this;

	for (int i=0; i<m_vElements.size(); i++)
	{
		if (m_vElements[i].get() == index)
		{
			std::shared_ptr<CBaseUIElement> buffer(element);
			element->setParent(this);

			if (back)
				m_vElements.insert(m_vElements.begin() + clamp<int>(i+1, 0, m_vElements.size()), buffer);
			else
				m_vElements.insert(m_vElements.begin() + clamp<int>(i, 0, m_vElements.size()), buffer);

			updateElement(element);
			return this;
		}
	}

	debugLog("Warning: %s::insertSlot() couldn't find index: %s\n", getName().toUtf8(), index->getName().toUtf8());

	return this;
}

CBaseUIContainerBase *CBaseUIContainerBase::insertElement(CBaseUIElement *element, std::shared_ptr<CBaseUIElement> index, bool back)
{
	if (element == nullptr || index == NULL || index.get() == nullptr) return this;

	for (int i=0; i<m_vElements.size(); i++)
	{
		if (m_vElements[i] == index)
		{
			std::shared_ptr<CBaseUIElement> buffer(element);
			element->setParent(this);

			if (back)
				m_vElements.insert(m_vElements.begin() + clamp<int>(i+1, 0, m_vElements.size()), buffer);
			else
				m_vElements.insert(m_vElements.begin() + clamp<int>(i, 0, m_vElements.size()), buffer);

			updateElement(element);
			return this;
		}
	}

	return this;
}

CBaseUIContainerBase *CBaseUIContainerBase::insertElement(std::shared_ptr<CBaseUIElement> element, CBaseUIElement *index, bool back)
{
	if (element == NULL || element.get() == nullptr || index == NULL) return this;

	for (int i=0; i<m_vElements.size(); i++)
	{
		if (m_vElements[i].get() == index)
		{
			element.get()->setParent(this);

			if (back)
				m_vElements.insert(m_vElements.begin() + clamp<int>(i+1, 0, m_vElements.size()), element);
			else
				m_vElements.insert(m_vElements.begin() + clamp<int>(i, 0, m_vElements.size()), element);

			updateElement(element.get());
			return this;
		}
	}

	return this;
}

CBaseUIContainerBase *CBaseUIContainerBase::insertElement(std::shared_ptr<CBaseUIElement> element, std::shared_ptr<CBaseUIElement> index, bool back)
{

	if (element == NULL || index == NULL || element.get() == nullptr || index.get()==nullptr) return this;

	for (int i=0; i<m_vElements.size(); i++)
	{
		if (m_vElements[i] == index)
		{
			element->setParent(this);
			if (back)
				m_vElements.insert(m_vElements.begin() + clamp<int>(i+1, 0, m_vElements.size()), element);
			else
				m_vElements.insert(m_vElements.begin() + clamp<int>(i, 0, m_vElements.size()), element);
			updateElement(element.get());
			return this;
		}
	}

	debugLog("Warning: %s::insertSlot() couldn't find index: %s\n", getName().toUtf8(), index.get()->getName().toUtf8());

	return this;
}

void CBaseUIContainerBase::removeElement(CBaseUIElement *element)
{
	for (int i=0; i<m_vElements.size(); i++)
	{
		if (m_vElements[i].get() == element)
		{
			m_vElements[i]->setParent(nullptr);
			m_vElements.erase(m_vElements.begin()+i);
			updateLayout();
			return;
		}
	}

	debugLog("Warning: CBaseUIContainerBase::removeElement() couldn't find element\n");
}

void CBaseUIContainerBase::removeElement(std::shared_ptr<CBaseUIElement> element)
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

	debugLog("Warning: CBaseUIContainerBase::removeElement() couldn't find element\n");
}

CBaseUIElement *CBaseUIContainerBase::getElementByName(UString name, bool searchNestedContainers)
{
	for (int i=0; i<m_vElements.size(); i++)
	{
		if (m_vElements[i]->getName() == name)
			return m_vElements[i].get();


		else if (searchNestedContainers)
		{
			CBaseUIContainerBase *container = dynamic_cast<CBaseUIContainerBase *>(m_vElements[i].get());
			if (container != nullptr)
				return container->getElementByName(name, true);
		}
	}

	debugLog("Error: CBaseUIContainerBase::getSlotByElementName() \"%s\" does not exist!!!\n", name.toUtf8());
	return nullptr;
}

std::shared_ptr<CBaseUIElement> CBaseUIContainerBase::getElementSharedByName(UString name, bool searchNestedContainers)
{
	for (int i=0; i<m_vElements.size(); i++)
	{
		if (m_vElements[i]->getName() == name)
			return m_vElements[i];


		else if (searchNestedContainers)
		{
			CBaseUIContainerBase *container = dynamic_cast<CBaseUIContainerBase *>(m_vElements[i].get());
			if (container != nullptr)
				return container->getElementSharedByName(name, true);
		}
	}

	debugLog("Error: CBaseUIContainerBase::getSlotByElementName() \"%s\" does not exist!!!\n", name.toUtf8());
	return nullptr;
}

void CBaseUIContainerBase::draw(Graphics *g)
{
	if (!m_bVisible) return;

	if (m_bClipping)
		g->pushClipRect(McRect(m_vPos.x+1,m_vPos.y+1,m_vSize.x-1,m_vSize.y));

	for (int i=0; i<m_vElements.size(); i++)
	{
		m_vElements[i]->draw(g);
	}

	if (m_bClipping)
		g->popClipRect();
}

void CBaseUIContainerBase::update()
{
	if (!m_bVisible) return;

	for (int i=0; i<m_vElements.size(); i++)
		m_vElements[i]->update();
}
