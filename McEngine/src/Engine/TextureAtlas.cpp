//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		container for dynamically merging multiple images into one
//
// $NoKeywords: $imgtxat
//===============================================================================//

#include "TextureAtlas.h"

#include "Engine.h"
#include "ResourceManager.h"

TextureAtlas::TextureAtlas(int width, int height) : Resource()
{
	m_iWidth = width;
	m_iHeight = height;

	engine->getResourceManager()->requestNextLoadUnmanaged();
	m_atlasImage = engine->getResourceManager()->createImage(m_iWidth, m_iHeight);

	m_iCurX = PADDING;
	m_iCurY = PADDING;
	m_iMaxHeight = 0;
}

void TextureAtlas::init()
{
	engine->getResourceManager()->loadResource(m_atlasImage);

	m_bReady = true;
}

void TextureAtlas::initAsync()
{
	m_bAsyncReady = true;
}

void TextureAtlas::destroy()
{
	SAFE_DELETE(m_atlasImage);
}

Vector2 TextureAtlas::put(int width, int height, bool flipHorizontal, bool flipVertical, Color *pixels)
{
	if (width < 1 || height < 1)
		return Vector2();

	if (width > (m_iWidth - PADDING*2) || height > (m_iHeight - m_iCurY - PADDING))
	{
		debugLog("TextureAtlas::put( %i, %i ) WARNING: Out of bounds / impossible fit!\n", width, height);
		return Vector2();
	}
	if (m_atlasImage == NULL)
	{
		debugLog("TextureAtlas::put() ERROR: m_atlasImage == NULL!\n");
		return Vector2();
	}
	if (pixels == NULL)
	{
		debugLog("TextureAtlas::put() ERROR: pixels == NULL!\n");
		return Vector2();
	}

	// very simple packing logic: overflow down individual lines/rows, highest element in line determines max
	if (m_iCurX + width + PADDING > m_iWidth)
	{
		m_iCurX = PADDING;
		m_iCurY += m_iMaxHeight + PADDING;
		m_iMaxHeight = 0;
	}
	if (height > m_iMaxHeight)
		m_iMaxHeight = height;

	if (m_iCurY + height + PADDING > m_iHeight)
	{
		debugLog("TextureAtlas::put( %i, %i ) WARNING: Out of bounds / impossible fit!\n", width, height);
		return Vector2();
	}

	// insert pixels
	{
		for (int y=0; y<height; y++)
		{
			for (int x=0; x<width; x++)
			{
				int actualX = (flipHorizontal ? width - x - 1 : x);
				int actualY = (flipVertical ? height - y - 1 : y);

				m_atlasImage->setPixel(m_iCurX + x, m_iCurY + y, pixels[actualY*width + actualX]);
			}
		}
	}
	Vector2 pos = Vector2(m_iCurX, m_iCurY);

	// move offset for next element
	m_iCurX += width + PADDING;

	return pos;
}
