//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		software rasterizer implementation of Image
//
// $NoKeywords: $swimg
//===============================================================================//

#include "SWImage.h"

#include "ResourceManager.h"
#include "Environment.h"
#include "Engine.h"

SWImage::SWImage(UString filepath, bool mipmapped, bool keepInSystemMemory) : Image(filepath, mipmapped, keepInSystemMemory)
{
	// TODO:
}

SWImage::SWImage(int width, int height, bool mipmapped, bool keepInSystemMemory) : Image(width, height, mipmapped, keepInSystemMemory)
{
	// TODO:
}

void SWImage::init()
{
	// TODO:
	if (m_bAsyncReady)
		m_bReady = true;
}

void SWImage::initAsync()
{
	if (!m_bCreatedImage)
		printf("Resource Manager: Loading %s\n", m_sFilePath.toUtf8());

	m_bAsyncReady = loadRawImage();
}

void SWImage::destroy()
{
	// TODO:
}

void SWImage::bind(unsigned int textureUnit)
{
	// TODO:
}

void SWImage::unbind()
{
	// TODO:
}

void SWImage::setFilterMode(Graphics::FILTER_MODE filterMode)
{
	// TODO:
}

void SWImage::setWrapMode(Graphics::WRAP_MODE wrapMode)
{
	// TODO:
}
