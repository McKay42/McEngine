/*
 * CBaseUIContainerBox.cpp
 *
 *  Created on: Jun 1, 2017
 *      Author: Psy
 */

#include "CBaseUIContainerBox.h"

CBaseUIContainerBox::CBaseUIContainerBox(float xPos, float yPos, UString name) : CBaseUIContainerBase(name)
{
	m_vPos.x = xPos;
	m_vPos.y = yPos;
	m_vmPos = m_vPos;
}

CBaseUIContainerBox::~CBaseUIContainerBox()
{
}

void CBaseUIContainerBox::updateLayout()
{
	if (m_parent != nullptr)
		m_parent->updateLayout();

	for (int i=0; i<m_vElements.size(); i++)
	{
		m_vElements[i]->setPosAbsolute(m_vElements[i]->getRelPos() + m_vPos);
	}
}

void CBaseUIContainerBox::updateElement(CBaseUIElement *element)
{
	if (dynamic_cast<CBaseUIContainerBase *> (element) != NULL)
	{
		updateLayout();
		return;
	}

	element->setPosAbsolute(element->getRelPos() + m_vPos);
}
