//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		a simple image class
//
// $NoKeywords: $
//===============================================================================//

#include "CBaseUIImage.h"

#include "Engine.h"
#include "ResourceManager.h"

CBaseUIImage::CBaseUIImage(UString imageResourceName, float xPos, float yPos, float xSize, float ySize, UString name) : CBaseUIElement(xPos,yPos,xSize,ySize,name)
{
	m_bScaleToFit = true; // must be up here because it's used in setImage()
	setImage(engine->getResourceManager()->getImage(imageResourceName));

	m_fRot = 0.0f;
	m_vScale.x = 1.0f;
	m_vScale.y = 1.0f;

	// if our image is null, autosize to the element size
	if (m_image == NULL)
	{
		m_vSize.x = xSize;
		m_vSize.y = ySize;
	}

	m_frameColor = COLOR(255,255,255,255);
	m_backgroundColor = COLOR(255,0,0,0);
	m_color = 0xffffffff;

	m_bDrawFrame = false;
	m_bDrawBackground = false;
}

void CBaseUIImage::draw(Graphics *g)
{
	if (!m_bVisible) return;

	// draw background
	if (m_bDrawBackground)
	{
		g->setColor( m_backgroundColor );
		g->fillRect(m_vPos.x+1,m_vPos.y+1,m_vSize.x-1,m_vSize.y-1);
	}

	// draw image
	if (m_image != NULL)
	{
		g->setColor(m_color);
		g->pushTransform();

			// scale
			if (m_bScaleToFit)
				g->scale(m_vScale.x, m_vScale.y);

			// rotate
			if (m_fRot != 0.0f)
				g->rotate(m_fRot);

			// center and draw
			g->translate(m_vPos.x + (m_vSize.x/2) + (!m_bScaleToFit ? 1 : 0), m_vPos.y + (m_vSize.y/2) + (!m_bScaleToFit ? 1 : 0));
			g->drawImage(m_image);

		g->popTransform();
	}

	// draw frame
	if (m_bDrawFrame)
	{
		g->setColor(m_frameColor);
		g->drawRect(m_vPos.x,m_vPos.y,m_vSize.x,m_vSize.y);
	}
}

void CBaseUIImage::setImage(Image *img)
{
	m_image = img;

	if (m_image != NULL)
	{
		if (m_bScaleToFit)
		{
			m_vSize.x = m_image->getWidth();
			m_vSize.y = m_image->getHeight();
		}

		m_vScale.x = m_vSize.x / (float)m_image->getWidth();
		m_vScale.y = m_vSize.y / (float)m_image->getHeight();
	}
	else
	{
		m_vScale.x = 1;
		m_vScale.y = 1;
	}
}
