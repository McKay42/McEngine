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

	m_iPadding = 1;

	engine->getResourceManager()->requestNextLoadUnmanaged();
	m_atlasImage = engine->getResourceManager()->createImage(m_iWidth, m_iHeight);

	m_iCurX = m_iPadding;
	m_iCurY = m_iPadding;
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

	if (width > (m_iWidth - m_iPadding*2) || height > (m_iHeight - m_iCurY - m_iPadding))
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
	if (m_iCurX + width + m_iPadding > m_iWidth)
	{
		m_iCurX = m_iPadding;
		m_iCurY += m_iMaxHeight + m_iPadding;
		m_iMaxHeight = 0;
	}
	if (height > m_iMaxHeight)
		m_iMaxHeight = height;

	if (m_iCurY + height + m_iPadding > m_iHeight)
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

		// TODO: make this into an API
		// mirror border pixels
		if (m_iPadding > 1)
		{
			// left
			for (int y=-1; y<height+1; y++)
			{
				const int x = 0;
				int actualX = (flipHorizontal ? width - x - 1 : x);
				int actualY = clamp<int>((flipVertical ? height - y - 1 : y), 0, height-1);

				m_atlasImage->setPixel(m_iCurX + x - 1, m_iCurY + y, pixels[actualY*width + actualX]);
			}
			// right
			for (int y=-1; y<height+1; y++)
			{
				const int x = width - 1;
				int actualX = (flipHorizontal ? width - x - 1 : x);
				int actualY = clamp<int>((flipVertical ? height - y - 1 : y), 0, height-1);

				m_atlasImage->setPixel(m_iCurX + x + 1, m_iCurY + y, pixels[actualY*width + actualX]);
			}
			// top
			for (int x=-1; x<width+1; x++)
			{
				const int y = 0;
				int actualX = clamp<int>((flipHorizontal ? width - x - 1 : x), 0, width-1);
				int actualY = (flipVertical ? height - y - 1 : y);

				m_atlasImage->setPixel(m_iCurX + x, m_iCurY + y - 1, pixels[actualY*width + actualX]);
			}
			// bottom
			for (int x=-1; x<width+1; x++)
			{
				const int y = height - 1;
				int actualX = clamp<int>((flipHorizontal ? width - x - 1 : x), 0, width-1);
				int actualY = (flipVertical ? height - y - 1 : y);

				m_atlasImage->setPixel(m_iCurX + x, m_iCurY + y + 1, pixels[actualY*width + actualX]);
			}
		}
	}
	Vector2 pos = Vector2(m_iCurX, m_iCurY);

	// move offset for next element
	m_iCurX += width + m_iPadding;

	return pos;
}
