//================ Copyright (c) 2011, PG, All rights reserved. =================//
//
// Purpose:		a container for UI elements
//
// $NoKeywords: $
//===============================================================================//

#include "CBaseUIContainer.h"
#include "Engine.h"

CBaseUIContainer::CBaseUIContainer(float Xpos, float Ypos, float Xsize, float Ysize, UString name) : CBaseUIElement(Xpos, Ypos, Xsize, Ysize, name)
{
}

CBaseUIContainer::~CBaseUIContainer()
{
	clear();
}

void CBaseUIContainer::clear()
{
	for (int i=0; i<m_vElements.size(); i++)
	{
		delete m_vElements[i];
	}
	m_vElements = std::vector<CBaseUIElement*>();
}

void CBaseUIContainer::empty()
{
	m_vElements = std::vector<CBaseUIElement*>();
}

CBaseUIContainer *CBaseUIContainer::addBaseUIElement(CBaseUIElement *element, float xPos, float yPos)
{
	if (element == NULL) return this;

	element->setRelPos(xPos, yPos);
	element->setPos(m_vPos + element->getRelPos());
	m_vElements.push_back(element);

	return this;
}

CBaseUIContainer *CBaseUIContainer::addBaseUIElement(CBaseUIElement *element)
{
	if (element == NULL) return this;

	element->setRelPos(element->getPos().x, element->getPos().y);
	element->setPos(m_vPos + element->getRelPos());
	m_vElements.push_back(element);

	return this;
}

CBaseUIContainer *CBaseUIContainer::addBaseUIElementBack(CBaseUIElement *element, float xPos, float yPos)
{
	if (element == NULL) return this;

	element->setRelPos(xPos, yPos);
	element->setPos(m_vPos + element->getRelPos());
	m_vElements.insert(m_vElements.begin(), element);

	return this;
}


CBaseUIContainer *CBaseUIContainer::addBaseUIElementBack(CBaseUIElement *element)
{
	if (element == NULL) return this;

	element->setRelPos(element->getPos().x, element->getPos().y);
	element->setPos(m_vPos + element->getRelPos());
	m_vElements.insert(m_vElements.begin(), element);

	return this;
}

CBaseUIContainer *CBaseUIContainer::insertBaseUIElement(CBaseUIElement *element, CBaseUIElement *index)
{
	if (element == NULL || index == NULL) return this;

	element->setRelPos(element->getPos().x, element->getPos().y);
	element->setPos(m_vPos + element->getRelPos());
	for (int i=0; i<m_vElements.size(); i++)
	{
		if (m_vElements[i] == index)
		{
			m_vElements.insert(m_vElements.begin() + clamp<int>(i, 0, m_vElements.size()), element);
			return this;
		}
	}

	debugLog("Warning: CBaseUIContainer::insertBaseUIElement() couldn't find index\n");

	return this;
}

CBaseUIContainer *CBaseUIContainer::insertBaseUIElementBack(CBaseUIElement *element, CBaseUIElement *index)
{
	if (element == NULL || index == NULL) return this;

	element->setRelPos(element->getPos().x, element->getPos().y);
	element->setPos(m_vPos + element->getRelPos());
	for (int i=0; i<m_vElements.size(); i++)
	{
		if (m_vElements[i] == index)
		{
			m_vElements.insert(m_vElements.begin() + clamp<int>(i+1, 0, m_vElements.size()), element);
			return this;
		}
	}

	debugLog("Warning: CBaseUIContainer::insertBaseUIElementBack() couldn't find index\n");

	return this;
}

CBaseUIContainer *CBaseUIContainer::removeBaseUIElement(CBaseUIElement *element)
{
	for (int i=0; i<m_vElements.size(); i++)
	{
		if (m_vElements[i] == element)
		{
			m_vElements.erase(m_vElements.begin()+i);
			return this;
		}
	}

	debugLog("Warning: CBaseUIContainer::removeBaseUIElement() couldn't find element\n");

	return this;
}

CBaseUIContainer *CBaseUIContainer::deleteBaseUIElement(CBaseUIElement *element)
{
	for (int i=0; i<m_vElements.size(); i++)
	{
		if (m_vElements[i] == element)
		{
			delete element;
			m_vElements.erase(m_vElements.begin()+i);
			return this;
		}
	}

	debugLog("Warning: CBaseUIContainer::deleteBaseUIElement() couldn't find element\n");

	return this;
}

CBaseUIElement *CBaseUIContainer::getBaseUIElement(UString name)
{
	for (int i=0; i<m_vElements.size(); i++)
	{
		if (m_vElements[i]->getName() == name)
			return m_vElements[i];
	}
	debugLog("CBaseUIContainer ERROR: GetBaseUIElement() \"%s\" does not exist!!!\n",name.toUtf8());
	return NULL;
}

void CBaseUIContainer::draw(Graphics *g)
{
	if (!m_bVisible) return;

	for (int i=0; i<m_vElements.size(); i++)
	{
		if (!m_vElements[i]->isDrawnManually())
			m_vElements[i]->draw(g);
	}
}

void CBaseUIContainer::draw_debug(Graphics *g)
{
	g->setColor(0xffffffff);
	g->drawLine(m_vPos.x, m_vPos.y, m_vPos.x+m_vSize.x, m_vPos.y);
	g->drawLine(m_vPos.x, m_vPos.y, m_vPos.x, m_vPos.y+m_vSize.y);
	g->drawLine(m_vPos.x, m_vPos.y+m_vSize.y, m_vPos.x+m_vSize.x, m_vPos.y+m_vSize.y);
	g->drawLine(m_vPos.x+m_vSize.x, m_vPos.y, m_vPos.x+m_vSize.x, m_vPos.y+m_vSize.y);
}

void CBaseUIContainer::update()
{
	CBaseUIElement::update();
	if (!m_bVisible) return;

	for (int i=0; i<m_vElements.size(); i++)
	{
		m_vElements[i]->update();
	}
}

void CBaseUIContainer::update_pos()
{
	for (int i=0; i<m_vElements.size(); i++)
	{
		if (!m_vElements[i]->isPositionedManually())
			m_vElements[i]->setPos(m_vPos + m_vElements[i]->getRelPos());
	}
}

void CBaseUIContainer::update_pos(CBaseUIElement *element)
{
	if (element != NULL)
		element->setPos(m_vPos + element->getRelPos());
}

void CBaseUIContainer::onKeyUp(KeyboardEvent &e)
{
	for (int i=0; i<m_vElements.size(); i++)
	{
		if (m_vElements[i]->isVisible())
			m_vElements[i]->onKeyUp(e);
	}
}
void CBaseUIContainer::onKeyDown(KeyboardEvent &e)
{
	for (int i=0; i<m_vElements.size(); i++)
	{
		if (m_vElements[i]->isVisible())
			m_vElements[i]->onKeyDown(e);
	}
}

void CBaseUIContainer::onChar(KeyboardEvent &e)
{
	for (int i=0; i<m_vElements.size(); i++)
	{
		if (m_vElements[i]->isVisible())
			m_vElements[i]->onChar(e);
	}
}

void CBaseUIContainer::onFocusStolen()
{
	for (int i=0; i<m_vElements.size(); i++)
	{
		m_vElements[i]->stealFocus();
	}
}

void CBaseUIContainer::onEnabled()
{
	for (int i=0; i<m_vElements.size(); i++)
	{
		m_vElements[i]->setEnabled(true);
	}
}

void CBaseUIContainer::onDisabled()
{
	for (int i=0; i<m_vElements.size(); i++)
	{
		m_vElements[i]->setEnabled(false);
	}
}

void CBaseUIContainer::onMouseDownOutside()
{
	onFocusStolen();
}

bool CBaseUIContainer::isBusy()
{
	if (!m_bVisible)
		return false;

	for (int i=0; i<m_vElements.size(); i++)
	{
		if (m_vElements[i]->isBusy())
			return true;
	}

	return false;
}

bool CBaseUIContainer::isActive()
{
	if (!m_bVisible)
		return false;

	for (int i=0; i<m_vElements.size(); i++)
	{
		if (m_vElements[i]->isActive())
			return true;
	}

	return false;
}
