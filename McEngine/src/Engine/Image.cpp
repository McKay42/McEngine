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
#include "File.h"

#include "lodepng.h"
#include "jpeglib.h"

#include <setjmp.h>

struct jpegErrorManager
{
    // "public" fields
    struct jpeg_error_mgr pub;

    // for returning to the caller
    jmp_buf setjmp_buffer;
};

char jpegLastErrorMsg[JMSG_LENGTH_MAX];
void jpegErrorExit (j_common_ptr cinfo)
{
	jpegErrorManager *err = (jpegErrorManager*) cinfo->err;
	( *(cinfo->err->format_message) ) (cinfo, jpegLastErrorMsg);
	longjmp(err->setjmp_buffer, 1);
}

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

Image::Image(UString filepath, bool mipmapped, bool keepInSystemMemory) : Resource(filepath)
{
	m_bMipmapped = mipmapped;
	m_bKeepInSystemMemory = keepInSystemMemory;

	m_type = Image::TYPE::TYPE_PNG;
	m_iNumChannels = 4;
	m_iWidth = 0;
	m_iHeight = 0;

	m_bHasAlphaChannel = true;
	m_bCreatedImage = false;
}

Image::Image(int width, int height, bool mipmapped, bool keepInSystemMemory) : Resource()
{
	m_bMipmapped = mipmapped;
	m_bKeepInSystemMemory = keepInSystemMemory;

	m_type = Image::TYPE::TYPE_RGB;
	m_iNumChannels = 4;
	m_iWidth = width;
	m_iHeight = height;

	m_bHasAlphaChannel = true;
	m_bCreatedImage = true;

	// reserve and fill with pink pixels
	m_rawImage.resize(4*m_iWidth*m_iHeight);
	for (int i=0; i<m_iWidth*m_iHeight; i++)
	{
		m_rawImage.push_back(255);
		m_rawImage.push_back(0);
		m_rawImage.push_back(255);
		m_rawImage.push_back(255);
	}

	m_bAsyncReady = true;
}

bool Image::loadRawImage()
{
	// if it isn't a created image (created within the engine), load it from the corresponding file
	if (!m_bCreatedImage)
	{
		if (m_rawImage.size() > 0) // has already been loaded (or loading it again after setPixel(s))
			return true;

		if (!env->fileExists(m_sFilePath))
		{
			printf("Image Error: Couldn't find file %s !\n", m_sFilePath.toUtf8());
			return false;
		}

		if (m_bInterrupted) // cancellation point
			return false;

		// load entire file
		File file(m_sFilePath);
		if (!file.canRead())
		{
			printf("Image Error: Couldn't canRead() %s !\n", m_sFilePath.toUtf8());
			return false;
		}
		if (file.getFileSize() < 4)
		{
			printf("Image Error: FileSize is < 4 in %s !\n", m_sFilePath.toUtf8());
			return false;
		}

		if (m_bInterrupted) // cancellation point
			return false;

		const char *data = file.readFile();
		if (data == NULL)
		{
			printf("Image Error: Couldn't readFile() %s !\n", m_sFilePath.toUtf8());
			return false;
		}

		if (m_bInterrupted) // cancellation point
			return false;

		// determine file type by magic number (png/jpg)
		bool isJPEG = false;
		bool isPNG = false;
		{
			unsigned char buf[4];
			buf[0] = (unsigned char)data[0];
			buf[1] = (unsigned char)data[1];
			buf[2] = (unsigned char)data[2];
			buf[3] = (unsigned char)data[3];
			if (buf[0] == 0xff && buf[1] == 0xD8 && buf[2] == 0xff) // 0xFFD8FF
				isJPEG = true;
			else if (buf[0] == 0x89 && buf[1] == 0x50 && buf[2] == 0x4E && buf[3] == 0x47) // 0x89504E47 (%PNG)
				isPNG = true;
		}

		// depending on the type, load either jpeg or png
		if (isJPEG)
		{
			m_type = Image::TYPE::TYPE_JPG;

			m_bHasAlphaChannel = false;

			// decode jpeg
			jpegErrorManager err;
			jpeg_decompress_struct cinfo;

			jpeg_create_decompress(&cinfo);
			cinfo.err = jpeg_std_error(&err.pub); // FUCK YOU, PIECE OF SHIT LIBRARY, crashing due to a missing error handler ffs
			err.pub.error_exit = jpegErrorExit;
			if (setjmp(err.setjmp_buffer))
			{
			    jpeg_destroy_decompress(&cinfo);
			    printf("Image Error: JPEG error (%s) on file %s\n", jpegLastErrorMsg, m_sFilePath.toUtf8());
			    return false;
			}

			jpeg_mem_src(&cinfo, (unsigned char*)data, file.getFileSize());
#ifdef __APPLE__
			int headerRes = jpeg_read_header(&cinfo, boolean::TRUE); // HACKHACK MACMAC: wtf is this boolean enum here suddenly
#else
			int headerRes = jpeg_read_header(&cinfo, TRUE);
#endif
			if (headerRes != JPEG_HEADER_OK)
			{
				jpeg_destroy_decompress(&cinfo);
				printf("Image Error: JPEG read_header() error %i on file %s\n", headerRes, m_sFilePath.toUtf8());
				return false;
			}

			m_iWidth = cinfo.image_width;
			m_iHeight = cinfo.image_height;
			m_iNumChannels = cinfo.num_components;
			if (m_iNumChannels == 4)
				m_bHasAlphaChannel = true;

			if (m_iWidth > 4096 || m_iHeight > 4096)
			{
				jpeg_destroy_decompress(&cinfo);
				printf("Image Error: JPEG image size is too big (%i x %i)!\n", m_iWidth, m_iHeight);
				return false;
			}

			//int rowStride = m_iWidth * m_iNumChannels;
			//JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, rowStride, 1);

			// extract each scanline of the image
			jpeg_start_decompress(&cinfo);
			m_rawImage.resize(m_iWidth*m_iHeight*m_iNumChannels);
			JSAMPROW j;
			for (int i=0; i<m_iHeight; ++i)
			{
				if (m_bInterrupted) // cancellation point
				{
					jpeg_destroy_decompress(&cinfo);
					return false;
				}

				j = (&m_rawImage[0] + (i * m_iWidth * m_iNumChannels));
				jpeg_read_scanlines(&cinfo, &j, 1);
			}

			jpeg_finish_decompress(&cinfo);
			jpeg_destroy_decompress(&cinfo);
		}
		else if (isPNG)
		{
			m_type = Image::TYPE::TYPE_PNG;

			unsigned int width = 0; // yes, these are here on purpose
			unsigned int height = 0;
			unsigned error = lodepng::decode(m_rawImage, width, height, (unsigned char*)data, file.getFileSize());
			m_iWidth = width;
			m_iHeight = height;
			if (error)
			{
				printf("Image Error: PNG error %i (%s) on file %s\n", error, lodepng_error_text(error), m_sFilePath.toUtf8());
				return false;
			}
		}
		else
		{
			printf("Image Error, file %s it neither a PNG nor a JPEG image!\n", m_sFilePath.toUtf8());
			return false;
		}
	}

	if (m_bInterrupted) // cancellation point
		return false;

	// error checking
	if (m_rawImage.size() < m_iWidth*m_iHeight*m_iNumChannels) // sanity check
	{
		engine->showMessageError("Image Error", UString::format("Loaded image has only %i/%i bytes in file %s", m_rawImage.size(), m_iWidth*m_iHeight*m_iNumChannels, m_sFilePath.toUtf8()));
		return false;
	}

	if (m_iNumChannels != 4 && m_iNumChannels != 3 && m_iNumChannels != 1) // another sanity check
	{
		engine->showMessageError("Image Error", UString::format("Unsupported number of color channels (%i) in file %s", m_iNumChannels, m_sFilePath.toUtf8()));
		return false;
	}

	// optimization: ignore completely transparent images (don't render)
	bool foundNonTransparentPixel = false;
	for (int x=0; x<m_iWidth; x++)
	{
		if (m_bInterrupted) // cancellation point
			return false;

		for (int y=0; y<m_iHeight; y++)
		{
			if (COLOR_GET_Ai(getPixel(x, y)) > 0)
			{
				foundNonTransparentPixel = true;
				break;
			}
		}
	}
	if (!foundNonTransparentPixel)
	{
		debugLog("Image: Ignoring empty transparent image \"%s\".\n", m_sFilePath.toUtf8());
		return false;
	}

	return true;
}

Color Image::getPixel(int x, int y)
{
	if (x < 0 || y < 0 || (4 * y * m_iWidth + 4 * x + 3) > (int)(m_rawImage.size()-1))
		return 0xffffff00;

	uint32_t r = 255;
	uint32_t g = 255;
	uint32_t b = 0;
	uint32_t a = 255;

	r = m_rawImage[4 * y * m_iWidth + 4 * x + 0];
	if (m_iNumChannels > 1)
	{
		g = m_rawImage[4 * y * m_iWidth + 4 * x + 1];
		b = m_rawImage[4 * y * m_iWidth + 4 * x + 2];

		if (m_iNumChannels > 3)
			a = m_rawImage[4 * y * m_iWidth + 4 * x + 3];
		else
			a = 255;
	}
	else
	{
		g = r;
		b = r;
		a = r;
	}

	return COLOR(a, r, g, b);
}

void Image::setPixel(int x, int y, Color color)
{
	if (x < 0 || y < 0 || (4 * y * m_iWidth + 4 * x + 3) > (int)(m_rawImage.size()-1)) return;

	m_rawImage[4 * y * m_iWidth + 4 * x + 0] = COLOR_GET_Ri(color);
	m_rawImage[4 * y * m_iWidth + 4 * x + 1] = COLOR_GET_Gi(color);
	m_rawImage[4 * y * m_iWidth + 4 * x + 2] = COLOR_GET_Bi(color);
	m_rawImage[4 * y * m_iWidth + 4 * x + 3] = COLOR_GET_Ai(color);
}

void Image::setPixels(std::vector<unsigned char> pixels)
{
	if (pixels.size() < m_iWidth*m_iHeight*m_iNumChannels)
	{
		debugLog("Image::setPixels() error, supplied array is too small!\n");
		return;
	}

	m_rawImage = pixels;
}

void Image::writeToFile(UString folder)
{
	if (!m_bReady || !m_bCreatedImage) return;
	if (m_iWidth <= 0 || m_iHeight <= 0 || m_rawImage.size() == 0 || m_rawImage.size() < 4 || m_rawImage.size() % 4 != 0) return;

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
