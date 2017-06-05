/*
 * CBaseUIContainerHorizontalBox.cpp
 *
 *  Created on: Jun 1, 2017
 *      Author: Psy
 */

#include <CBaseUIContainerHorizontalBox.h>

CBaseUIContainerHorizontalBox::CBaseUIContainerHorizontalBox(float posX, float posY, float sizeX, float sizeY, UString name) : CBaseUIContainerBase(name)
{
	m_vPos.x = posX;
	m_vPos.y = posY;
	m_vmPos = m_vPos;
	m_vSize.x = sizeX;
	m_vSize.y = sizeY;
	m_vmSize = m_vSize;

	m_bWidthOverride = false;
	m_bScaleByHeightOnly = false;
}

CBaseUIContainerHorizontalBox::~CBaseUIContainerHorizontalBox()
{

}

void CBaseUIContainerHorizontalBox::updateLayout()
{

}

void CBaseUIContainerHorizontalBox::updateElement(CBaseUIElement *element)
{

}
