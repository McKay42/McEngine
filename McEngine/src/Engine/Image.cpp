//================ Copyright (c) 2012, PG, All rights reserved. =================//
//
// Purpose:		image wrapper
//
// $NoKeywords: $img
//===============================================================================//

#include "Image.h"
#include "ResourceManager.h"
#include "Environment.h"
#include "Engine.h"

#include "lodepng.h"

void Image::saveToImage(unsigned char *data, unsigned int width, unsigned int height, UString filepath)
{
	debugLog("Saving image to %s ...\n", filepath.toUtf8());

	unsigned error = lodepng::encode(filepath.toUtf8(), data, width, height, LodePNGColorType::LCT_RGB, 8);
	if (error)
	{
		debugLog("PNG error %i on file %s", error, filepath.toUtf8());
		UString errorMessage = UString::format("PNG error %i on file ", error);
		errorMessage.append(filepath);
		engine->showMessageError(errorMessage, lodepng_error_text(error));
		return;
	}
}

Image::Image(ResourceManager *loader, UString filepath, bool mipmapped) : Resource(loader,filepath)
{
	m_bMipmapped = mipmapped;
	m_bClampToEdge = false;
	m_iWidth = 0;
	m_iHeight = 0;
	m_bCreatedImage = false;
}

Image::Image(int width, int height, bool clampToEdge) : Resource()
{
	m_bMipmapped = true;
	m_iWidth = width;
	m_iHeight = height;
	m_bCreatedImage = true;
	m_bClampToEdge = clampToEdge;

	// reserve and fill with pink pixels
	m_rawImage.resize(4*m_iWidth*m_iHeight);
	for (int i=0; i<m_iWidth*m_iHeight; i++)
	{
		m_rawImage.push_back(255);
		m_rawImage.push_back(0);
		m_rawImage.push_back(255);
		m_rawImage.push_back(255);
	}

	m_bReady = true;
}

Image::~Image()
{
	m_rawImage = std::vector<unsigned char>();
}

void Image::setPixel(int x, int y, Color color)
{
	if (!m_bReady || (4 * y * m_iWidth + 4 * x + 3) > (m_rawImage.size()-1)) return;

	m_rawImage[4 * y * m_iWidth + 4 * x + 0] = COLOR_GET_Ri(color);
	m_rawImage[4 * y * m_iWidth + 4 * x + 1] = COLOR_GET_Gi(color);
	m_rawImage[4 * y * m_iWidth + 4 * x + 2] = COLOR_GET_Bi(color);
	m_rawImage[4 * y * m_iWidth + 4 * x + 3] = COLOR_GET_Ai(color);
}

Color Image::getPixel(int x, int y)
{
	if (!m_bReady || (4 * y * m_iWidth + 4 * x + 3) > (m_rawImage.size()-1)) return 0xffffff00;

	uint32_t r = m_rawImage[4 * y * m_iWidth + 4 * x + 0];
	uint32_t g = m_rawImage[4 * y * m_iWidth + 4 * x + 1];
	uint32_t b = m_rawImage[4 * y * m_iWidth + 4 * x + 2];
	uint32_t a = m_rawImage[4 * y * m_iWidth + 4 * x + 3];

	return COLOR(a,r,g,b);
}

void Image::writeToFile(UString folder)
{
	if (!m_bReady || !m_bCreatedImage)
		return;

	if (m_iWidth <= 0 || m_iHeight <= 0 || m_rawImage.size() == 0 || m_rawImage.size() < 4 || m_rawImage.size() % 4 != 0)
		return;

	folder.append(m_sName);
	folder.append(".png");

	// HACKHACK: for metroid model viewer only!!!
	// switch from inverse alpha
	std::vector<unsigned char> tempRGBAraw;
	tempRGBAraw.reserve(m_rawImage.size());
	for (int i=0; i<m_rawImage.size(); i+=4)
	{
		tempRGBAraw.push_back(m_rawImage[i]);		//R
		tempRGBAraw.push_back(m_rawImage[i+1]);		//G
		tempRGBAraw.push_back(m_rawImage[i+2]);		//B
		tempRGBAraw.push_back(255 - m_rawImage[i+3]);	//A
	}

	unsigned error = lodepng::encode(folder.toUtf8(), (const unsigned char*)(&tempRGBAraw[0]), m_iWidth, m_iHeight);
	if (error)
	{
		debugLog("PNG error %i on file %s", error, folder.toUtf8());
		UString errorMessage = UString::format("PNG error %i on file ", error);
		errorMessage.append(folder);
		engine->showMessageError(errorMessage, lodepng_error_text(error));
		return;
	}
}
