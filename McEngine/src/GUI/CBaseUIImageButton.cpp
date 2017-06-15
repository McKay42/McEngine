//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		a simple image button
//
// $NoKeywords: $
//===============================================================================//

#include "CBaseUIImageButton.h"

#include "Engine.h"
#include "ResourceManager.h"

CBaseUIImageButton::CBaseUIImageButton(UString imageResourceName, float xPos, float yPos, float xSize, float ySize, UString name) : CBaseUIButton(xPos, yPos, xSize, ySize, name, "")
{
	setImageResourceName(imageResourceName);

	m_fRot = 0.0f;
	m_vScale = Vector2(1, 1);
	m_bScaleToFit = false;
	m_bKeepAspectRatio = true;
}

void CBaseUIImageButton::draw(Graphics *g)
{
	if (!m_bVisible) return;

	// draw image
	Image *image = engine->getResourceManager()->getImage(m_sImageResourceName);
	if (image != NULL)
	{
		g->setColor(0xffffffff);
		g->pushTransform();

			// scale
			g->scale(m_vScale.x, m_vScale.y);

			// rotate
			if (m_fRot != 0.0f)
				g->rotate(m_fRot);

			// center and draw
			g->translate(m_vPos.x + (int)(m_vSize.x/2), m_vPos.y + (int)(m_vSize.y/2));
			g->drawImage(image);

		g->popTransform();
	}
}

CBaseUIImageButton *CBaseUIImageButton::setImageResourceName(UString imageResourceName)
{
	m_sImageResourceName = imageResourceName;

	Image *image = engine->getResourceManager()->getImage(m_sImageResourceName);
	if (image != NULL)
		setSize(Vector2(image->getWidth(), image->getHeight()));

	return this;
}

void CBaseUIImageButton::onResized()
{
	CBaseUIButton::onResized();

	Image *image = engine->getResourceManager()->getImage(m_sImageResourceName);
	if (m_bScaleToFit && image != NULL)
	{
		if (!m_bKeepAspectRatio)
		{
			m_vScale = Vector2(m_vSize.x/image->getWidth(), m_vSize.y/image->getHeight());
			m_vSize.x = (int)(image->getWidth()*m_vScale.x);
			m_vSize.y = (int)(image->getHeight()*m_vScale.y);
		}
		else
		{
			float scaleFactor = m_vSize.x/image->getWidth() < m_vSize.y/image->getHeight() ? m_vSize.x/image->getWidth() : m_vSize.y/image->getHeight();
			m_vScale = Vector2(scaleFactor, scaleFactor);
			m_vSize.x = (int)(image->getWidth()*m_vScale.x);
			m_vSize.y = (int)(image->getHeight()*m_vScale.y);
		}
	}
}
