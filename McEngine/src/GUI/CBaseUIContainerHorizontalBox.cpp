/*
 * CBaseUIContainerHorizontalBox.cpp
 *
 *  Created on: Jun 1, 2017
 *      Author: Psy
 */

#include "CBaseUIContainerHorizontalBox.h"
#include "Engine.h"

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
	m_padding = 0;
}

CBaseUIContainerHorizontalBox::~CBaseUIContainerHorizontalBox()
{

}

void CBaseUIContainerHorizontalBox::drawDebug(Graphics *g, Color color)
{
	if (!m_bVisible) return;

	g->setColor(color);
	g->drawRect(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y);
}

void CBaseUIContainerHorizontalBox::updateLayout()
{
	if (m_parent != nullptr)
		m_parent->updateLayout();

	float widthStorage = 0;
	if (!m_bWidthOverride)
		widthStorage = (m_vSize.x - 2 * m_vMargin.x - (m_vElements.size() - 1) * m_padding) / m_vElements.size();

	float posStorage = m_vPos.x;

	for (int i=0; i<m_vElements.size(); i++)
	{
		if (!m_bWidthOverride && !m_bScaleByHeightOnly)
		{
			m_vElements[i]->setSizeAbsolute(widthStorage, m_vSize.y - 2 * m_vMargin.y);
			m_vElements[i]->setPosAbsolute(m_vPos.x + m_vMargin.x + (m_padding + widthStorage) * i, m_vPos.y + m_vMargin.y);
		}

		else
		{
			if (!m_bScaleByHeightOnly)
				m_vElements[i]->setSizeAbsolute(m_vSize.x * m_vElements[i]->getRelSize().x, m_vSize.y - 2 * m_vMargin.y);
			else
				m_vElements[i]->setSizeAbsolute(m_vSize.y / 9 * 16 * m_vElements[i]->getRelSize().x, m_vSize.y - 2 * m_vMargin.y);

			m_vElements[i]->setPosAbsolute(posStorage + (posStorage == m_vPos.x ? m_padding : m_vMargin.x), m_vPos.y + m_vMargin.y);

			posStorage = m_vElements[i]->getPos().x + m_vElements[i]->getSize().x;
		}
	}
}

void CBaseUIContainerHorizontalBox::updateElement(CBaseUIElement *element)
{
	updateLayout();
}

void CBaseUIContainerHorizontalBox::onMoved()
{
	float widthStorage = 0;
	if (!m_bWidthOverride)
		widthStorage = (m_vSize.x - 2 * m_vMargin.x - (m_vElements.size() - 1) * m_padding) / m_vElements.size();

	float posStorage = m_vPos.x;

	for (int i=0; i<m_vElements.size(); i++)
	{
		if (!m_bWidthOverride && !m_bScaleByHeightOnly)
		{
			m_vElements[i]->setSizeAbsolute(widthStorage, m_vSize.y - 2 * m_vMargin.y);
			m_vElements[i]->setPosAbsolute(m_vPos.x + m_vMargin.x + (m_padding + widthStorage) * i, m_vPos.y + m_vMargin.y);
		}

		else
		{
			if (!m_bScaleByHeightOnly)
				m_vElements[i]->setSizeAbsolute(m_vSize.x * m_vElements[i]->getRelSize().x, m_vSize.y - 2 * m_vMargin.y);
			else
				m_vElements[i]->setSizeAbsolute(m_vSize.y / 9 * 16 * m_vElements[i]->getRelSize().x, m_vSize.y - 2 * m_vMargin.y);

			m_vElements[i]->setPosAbsolute(posStorage + (posStorage == m_vPos.x ? m_padding : m_vMargin.x), m_vPos.y + m_vMargin.y);

			posStorage = m_vElements[i]->getPos().x + m_vElements[i]->getSize().x;
		}
	}
}

void CBaseUIContainerHorizontalBox::onResized()
{
	float widthStorage = 0;
	if (!m_bWidthOverride)
		widthStorage = (m_vSize.x - 2 * m_vMargin.x - (m_vElements.size() - 1) * m_padding) / m_vElements.size();

	float posStorage = m_vPos.x;

	for (int i=0; i<m_vElements.size(); i++)
	{
		if (!m_bWidthOverride && !m_bScaleByHeightOnly)
		{
			m_vElements[i]->setSizeAbsolute(widthStorage, m_vSize.y - 2 * m_vMargin.y);
			m_vElements[i]->setPosAbsolute(m_vPos.x + m_vMargin.x + (m_padding + widthStorage) * i, m_vPos.y + m_vMargin.y);
		}

		else
		{
			if (!m_bScaleByHeightOnly)
				m_vElements[i]->setSizeAbsolute(m_vSize.x * m_vElements[i]->getRelSize().x, m_vSize.y - 2 * m_vMargin.y);
			else
				m_vElements[i]->setSizeAbsolute(m_vSize.y / 9 * 16 * m_vElements[i]->getRelSize().x, m_vSize.y - 2 * m_vMargin.y);

			m_vElements[i]->setPosAbsolute(posStorage + (posStorage == m_vPos.x ? m_padding : m_vMargin.x), m_vPos.y + m_vMargin.y);

			posStorage = m_vElements[i]->getPos().x + m_vElements[i]->getSize().x;
		}
	}
}
