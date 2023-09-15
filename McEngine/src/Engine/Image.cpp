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

void jpegErrorExit(j_common_ptr cinfo)
{
	char jpegLastErrorMsg[JMSG_LENGTH_MAX];

	jpegErrorManager *err = (jpegErrorManager*)cinfo->err;

	(*(cinfo->err->format_message))(cinfo, jpegLastErrorMsg);
	jpegLastErrorMsg[JMSG_LENGTH_MAX - 1] = '\0';

	printf("JPEG Error: %s", jpegLastErrorMsg);

	longjmp(err->setjmp_buffer, 1);
}

void Image::saveToImage(unsigned char *data, unsigned int width, unsigned int height, UString filepath)
{
	debugLog("Saving image to %s ...\n", filepath.toUtf8());

	const unsigned error = lodepng::encode(filepath.toUtf8(), data, width, height, LodePNGColorType::LCT_RGB, 8);
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
	m_filterMode = Graphics::FILTER_MODE::FILTER_MODE_LINEAR;
	m_wrapMode = Graphics::WRAP_MODE::WRAP_MODE_CLAMP;
	m_iNumChannels = 4;
	m_iWidth = 1;
	m_iHeight = 1;

	m_bHasAlphaChannel = true;
	m_bCreatedImage = false;
}

Image::Image(int width, int height, bool mipmapped, bool keepInSystemMemory) : Resource()
{
	m_bMipmapped = mipmapped;
	m_bKeepInSystemMemory = keepInSystemMemory;

	m_type = Image::TYPE::TYPE_RGBA;
	m_filterMode = Graphics::FILTER_MODE::FILTER_MODE_LINEAR;
	m_wrapMode = Graphics::WRAP_MODE::WRAP_MODE_CLAMP;
	m_iNumChannels = 4;
	m_iWidth = width;
	m_iHeight = height;

	m_bHasAlphaChannel = true;
	m_bCreatedImage = true;

	// reserve and fill with pink pixels
	m_rawImage.resize(m_iWidth * m_iHeight * m_iNumChannels);
	for (int i=0; i<m_iWidth*m_iHeight; i++)
	{
		m_rawImage.push_back(255);
		m_rawImage.push_back(0);
		m_rawImage.push_back(255);
		m_rawImage.push_back(255);
	}

	// special case: filled rawimage is always already async ready
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
			printf("Image Error: Couldn't find file %s\n", m_sFilePath.toUtf8());
			return false;
		}

		if (m_bInterrupted) // cancellation point
			return false;

		// load entire file
		File file(m_sFilePath);
		if (!file.canRead())
		{
			printf("Image Error: Couldn't canRead() file %s\n", m_sFilePath.toUtf8());
			return false;
		}
		if (file.getFileSize() < 4)
		{
			printf("Image Error: FileSize is < 4 in file %s\n", m_sFilePath.toUtf8());
			return false;
		}

		if (m_bInterrupted) // cancellation point
			return false;

		const char *data = file.readFile();
		if (data == NULL)
		{
			printf("Image Error: Couldn't readFile() file %s\n", m_sFilePath.toUtf8());
			return false;
		}

		if (m_bInterrupted) // cancellation point
			return false;

		// determine file type by magic number (png/jpg)
		bool isJPEG = false;
		bool isPNG = false;
		{
			const int numBytes = 4;

			unsigned char buf[numBytes];

			for (int i=0; i<numBytes; i++)
			{
				buf[i] = (unsigned char)data[i];
			}

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
			cinfo.err = jpeg_std_error(&err.pub);
			err.pub.error_exit = jpegErrorExit;
			if (setjmp(err.setjmp_buffer))
			{
			    jpeg_destroy_decompress(&cinfo);
			    printf("Image Error: JPEG error (see above) in file %s\n", m_sFilePath.toUtf8());
			    return false;
			}

			jpeg_mem_src(&cinfo, (unsigned char*)data, file.getFileSize());
#ifdef __APPLE__
			const int headerRes = jpeg_read_header(&cinfo, boolean::TRUE); // HACKHACK: wtf is this boolean enum here suddenly required?
#else
			const int headerRes = jpeg_read_header(&cinfo, TRUE);
#endif
			if (headerRes != JPEG_HEADER_OK)
			{
				jpeg_destroy_decompress(&cinfo);
				printf("Image Error: JPEG read_header() error %i in file %s\n", headerRes, m_sFilePath.toUtf8());
				return false;
			}

			m_iWidth = cinfo.image_width;
			m_iHeight = cinfo.image_height;
			m_iNumChannels = cinfo.num_components;

			// NOTE: color spaces which require color profiles are not supported (e.g. J_COLOR_SPACE::JCS_YCCK)

			if (m_iNumChannels == 4)
				m_bHasAlphaChannel = true;

			if (m_iWidth > 8192 || m_iHeight > 8192)
			{
				jpeg_destroy_decompress(&cinfo);
				printf("Image Error: JPEG image size is too big (%i x %i) in file %s\n", m_iWidth, m_iHeight, m_sFilePath.toUtf8());
				return false;
			}

			// preallocate
			m_rawImage.resize(m_iWidth * m_iHeight * m_iNumChannels);

			// extract each scanline of the image
			jpeg_start_decompress(&cinfo);
			JSAMPROW j;
			for (int y=0; y<m_iHeight; y++)
			{
				if (m_bInterrupted) // cancellation point
				{
					jpeg_destroy_decompress(&cinfo);
					return false;
				}

				j = (&m_rawImage[0] + (y * m_iWidth * m_iNumChannels));
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

			const unsigned error = lodepng::decode(m_rawImage, width, height, (const unsigned char*)data, file.getFileSize());

			m_iWidth = width;
			m_iHeight = height;

			if (error)
			{
				printf("Image Error: PNG error %i (%s) in file %s\n", error, lodepng_error_text(error), m_sFilePath.toUtf8());
				return false;
			}
		}
		else
		{
			printf("Image Error: Neither PNG nor JPEG in file %s\n", m_sFilePath.toUtf8());
			return false;
		}
	}

	if (m_bInterrupted) // cancellation point
		return false;

	// error checking

	// size sanity check
	if (m_rawImage.size() < (m_iWidth * m_iHeight * m_iNumChannels))
	{
		printf("Image Error: Loaded image has only %lu/%i bytes in file %s\n", (unsigned long)m_rawImage.size(), m_iWidth * m_iHeight * m_iNumChannels, m_sFilePath.toUtf8());
		//engine->showMessageError("Image Error", UString::format("Loaded image has only %i/%i bytes in file %s", m_rawImage.size(), m_iWidth*m_iHeight*m_iNumChannels, m_sFilePath.toUtf8()));
		return false;
	}

	// supported channels sanity check
	if (m_iNumChannels != 4 && m_iNumChannels != 3 && m_iNumChannels != 1)
	{
		printf("Image Error: Unsupported number of color channels (%i) in file %s", m_iNumChannels, m_sFilePath.toUtf8());
		//engine->showMessageError("Image Error", UString::format("Unsupported number of color channels (%i) in file %s", m_iNumChannels, m_sFilePath.toUtf8()));
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

		if (foundNonTransparentPixel)
			break;
	}
	if (!foundNonTransparentPixel)
	{
		printf("Image: Ignoring empty transparent image %s\n", m_sFilePath.toUtf8());
		return false;
	}

	return true;
}

void Image::setFilterMode(Graphics::FILTER_MODE filterMode)
{
	m_filterMode = filterMode;
}

void Image::setWrapMode(Graphics::WRAP_MODE wrapMode)
{
	m_wrapMode = wrapMode;
}

Color Image::getPixel(int x, int y) const
{
	const int indexBegin = m_iNumChannels * y * m_iWidth + m_iNumChannels * x;
	const int indexEnd = m_iNumChannels * y * m_iWidth + m_iNumChannels * x + m_iNumChannels;

	if (m_rawImage.size() < 1 || x < 0 || y < 0 || indexEnd < 0 || indexEnd > m_rawImage.size()) return 0xffffff00;

	unsigned char r = 255;
	unsigned char g = 255;
	unsigned char b = 0;
	unsigned char a = 255;

	r = m_rawImage[indexBegin + 0];
	if (m_iNumChannels > 1)
	{
		g = m_rawImage[indexBegin + 1];
		b = m_rawImage[indexBegin + 2];

		if (m_iNumChannels > 3)
			a = m_rawImage[indexBegin + 3];
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
	const int indexBegin = m_iNumChannels * y * m_iWidth + m_iNumChannels * x;
	const int indexEnd = m_iNumChannels * y * m_iWidth + m_iNumChannels * x + m_iNumChannels;

	if (m_rawImage.size() < 1 || x < 0 || y < 0 || indexEnd < 0 || indexEnd > m_rawImage.size()) return;

	m_rawImage[indexBegin + 0] = COLOR_GET_Ri(color);
	if (m_iNumChannels > 1)
		m_rawImage[indexBegin + 1] = COLOR_GET_Gi(color);
	if (m_iNumChannels > 2)
		m_rawImage[indexBegin + 2] = COLOR_GET_Bi(color);
	if (m_iNumChannels > 3)
		m_rawImage[indexBegin + 3] = COLOR_GET_Ai(color);
}

void Image::setPixels(const char *data, size_t size, TYPE type)
{
	if (data == NULL) return;

	// TODO: implement remaining types
	switch (type)
	{
	case TYPE::TYPE_PNG:
		{
			unsigned int width = 0; // yes, these are here on purpose
			unsigned int height = 0;

			const unsigned error = lodepng::decode(m_rawImage, width, height, (const unsigned char*)data, size);

			m_iWidth = width;
			m_iHeight = height;

			if (error)
				printf("Image Error: PNG error %i (%s) in file %s\n", error, lodepng_error_text(error), m_sFilePath.toUtf8());
		}
		break;

	default:
		debugLog("Image Error: Format not yet implemented\n");
		break;
	}
}

void Image::setPixels(const std::vector<unsigned char> &pixels)
{
	if (pixels.size() < (m_iWidth * m_iHeight * m_iNumChannels))
	{
		debugLog("Image Error: setPixels() supplied array is too small!\n");
		return;
	}

	m_rawImage = pixels;
}
