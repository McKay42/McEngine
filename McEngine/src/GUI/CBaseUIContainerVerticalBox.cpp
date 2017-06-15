/*
 * CBaseUIContainerVerticalBox.cpp
 *
 *  Created on: Jun 7, 2017
 *      Author: Psy
 */

#include "CBaseUIContainerVerticalBox.h"

CBaseUIContainerVerticalBox::CBaseUIContainerVerticalBox(float posX, float posY, float sizeX, float sizeY, UString name) : CBaseUIContainerBase(name)
{
	m_vPos.x = posX;
	m_vPos.y = posY;
	m_vmPos = m_vPos;
	m_vSize.x = sizeX;
	m_vSize.y = sizeY;
	m_vmSize = m_vSize;

	m_bHeightOverride = false;
	m_bScaleByHeightOnly = false;
	m_padding = 0;
}

CBaseUIContainerVerticalBox::~CBaseUIContainerVerticalBox()
{

}

void CBaseUIContainerVerticalBox::drawDebug(Graphics *g, Color color)
{
	if (!m_bVisible) return;

	g->setColor(color);
	g->drawRect(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y);

}

void CBaseUIContainerVerticalBox::updateLayout()
{
	if (m_parent != nullptr)
		m_parent->updateLayout();

	float heightStorage = 0;
	if (!m_bHeightOverride)
		heightStorage = (m_vSize.y - 2 * m_vMargin.y - (m_vElements.size() - 1) * m_padding) / m_vElements.size();

	float posStorage = m_vPos.y;

	for (int i=0; i<m_vElements.size(); i++)
	{
		if (!m_bHeightOverride && !m_bScaleByHeightOnly)
		{
			m_vElements[i]->setSizeAbsolute(m_vSize.x - 2 * m_vMargin.x, heightStorage);
			m_vElements[i]->setPosAbsolute(m_vPos.x + m_vMargin.x, m_vPos.y + m_vMargin.y + (m_padding + heightStorage) * i);
		}

		else
		{
			if (!m_bScaleByHeightOnly)
				m_vElements[i]->setSizeAbsolute(m_vSize.x - 2 * m_vMargin.x ,m_vSize.y * m_vElements[i]->getRelSize().y);
			else
				m_vElements[i]->setSizeAbsolute(m_vSize.x - 2 * m_vMargin.x, m_vSize.x / 16 * 9 * m_vElements[i]->getRelSize().y);

			m_vElements[i]->setPosAbsolute(m_vPos.x + m_vMargin.x, posStorage + (posStorage == m_vPos.y ? m_padding : m_vMargin.y));

			posStorage = m_vElements[i]->getPos().y + m_vElements[i]->getSize().y;
		}
	}
}

void CBaseUIContainerVerticalBox::updateElement(CBaseUIElement *element)
{
	updateLayout();
}

void CBaseUIContainerVerticalBox::onMoved()
{
	float heightStorage = 0;
	if (!m_bHeightOverride)
		heightStorage = (m_vSize.y - 2 * m_vMargin.y - (m_vElements.size() - 1) * m_padding) / m_vElements.size();

	float posStorage = m_vPos.y;

	for (int i=0; i<m_vElements.size(); i++)
	{
		if (!m_bHeightOverride && !m_bScaleByHeightOnly)
		{
			m_vElements[i]->setSizeAbsolute(m_vSize.x - 2 * m_vMargin.x, heightStorage);
			m_vElements[i]->setPosAbsolute(m_vPos.x + m_vMargin.x, m_vPos.y + m_vMargin.y + (m_padding + heightStorage) * i);
		}

		else
		{
			if (!m_bScaleByHeightOnly)
				m_vElements[i]->setSizeAbsolute(m_vSize.x - 2 * m_vMargin.x , m_vSize.y * m_vElements[i]->getRelSize().y);
			else
				m_vElements[i]->setSizeAbsolute(m_vSize.x - 2 * m_vMargin.x, m_vSize.x / 16 * 9 * m_vElements[i]->getRelSize().y);

			m_vElements[i]->setPosAbsolute(m_vPos.x + m_vMargin.x, posStorage + (posStorage == m_vPos.y ? m_padding : m_vMargin.y));

			posStorage = m_vElements[i]->getPos().y + m_vElements[i]->getSize().y;
		}
	}
}

void CBaseUIContainerVerticalBox::onResized()
{
	float heightStorage = 0;
	if (!m_bHeightOverride)
		heightStorage = (m_vSize.y - 2 * m_vMargin.y - (m_vElements.size() - 1) * m_padding) / m_vElements.size();

	float posStorage = m_vPos.y;

	for (int i=0; i<m_vElements.size(); i++)
	{
		if (!m_bHeightOverride && !m_bScaleByHeightOnly)
		{
			m_vElements[i]->setSizeAbsolute(m_vSize.x - 2 * m_vMargin.x, heightStorage);
			m_vElements[i]->setPosAbsolute(m_vPos.x + m_vMargin.x, m_vPos.y + m_vMargin.y + (m_padding + heightStorage) * i);
		}

		else
		{
			if (!m_bScaleByHeightOnly)
				m_vElements[i]->setSizeAbsolute(m_vSize.x - 2 * m_vMargin.x ,m_vSize.y * m_vElements[i]->getRelSize().y);
			else
				m_vElements[i]->setSizeAbsolute(m_vSize.x - 2 * m_vMargin.x, m_vSize.x / 16 * 9 * m_vElements[i]->getRelSize().y);

			m_vElements[i]->setPosAbsolute(m_vPos.x + m_vMargin.x, posStorage + (posStorage == m_vPos.y ? m_padding : m_vMargin.y));

			posStorage = m_vElements[i]->getPos().y + m_vElements[i]->getSize().y;
		}
	}
}
