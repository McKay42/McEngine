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
#include "ConVar.h"

#include "SWGraphicsInterface.h"

SWImage::SWImage(UString filepath, bool mipmapped, bool keepInSystemMemory) : Image(filepath, mipmapped, keepInSystemMemory)
{
	m_iTextureUnitBackup = 0;
}

SWImage::SWImage(int width, int height, bool mipmapped, bool keepInSystemMemory) : Image(width, height, mipmapped, keepInSystemMemory)
{
	m_iTextureUnitBackup = 0;
}

void SWImage::init()
{
	if (m_bAsyncReady)
		m_bReady = true;
}

void SWImage::initAsync()
{
	if (!m_bCreatedImage)
	{
		if (ResourceManager::debug_rm->getBool())
			printf("Resource Manager: Loading %s\n", m_sFilePath.toUtf8());
	}

	m_bAsyncReady = loadRawImage();
}

void SWImage::destroy()
{
	m_rawImage = std::vector<unsigned char>();
}

void SWImage::bind(unsigned int textureUnit)
{
	((SWGraphicsInterface*)engine->getGraphics())->bindTexture(this, textureUnit);
}

void SWImage::unbind()
{
	((SWGraphicsInterface*)engine->getGraphics())->bindTexture(NULL, m_iTextureUnitBackup);
}

void SWImage::setFilterMode(Graphics::FILTER_MODE filterMode)
{
	Image::setFilterMode(filterMode);
}

void SWImage::setWrapMode(Graphics::WRAP_MODE wrapMode)
{
	Image::setWrapMode(wrapMode);
}
