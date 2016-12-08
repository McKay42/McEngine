//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		OpenGL implementation of Image
//
// $NoKeywords: $glimg
//===============================================================================//

#include "OpenGLImage.h"

#include "ResourceManager.h"
#include "Environment.h"
#include "Engine.h"
#include "File.h"

#include "OpenGLHeaders.h"

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

OpenGLImage::OpenGLImage(UString filepath, bool mipmapped) : Image(filepath, mipmapped)
{
	m_GLTexture = 0;

	m_bHasAlphaChannel = true;
	m_bAlignRawDataToBytes = false;
	m_iNumChannels = 4;

	m_iTextureUnitBackup = 0;
}

OpenGLImage::OpenGLImage(int width, int height, bool clampToEdge) : Image(width, height, clampToEdge)
{
	m_GLTexture = 0;

	m_bHasAlphaChannel = true;
	m_bAlignRawDataToBytes = false;
	m_bAsyncReady = true;
	m_iNumChannels = 4;

	m_iTextureUnitBackup = 0;
}

void OpenGLImage::init()
{
	if (m_GLTexture != 0 || !m_bAsyncReady) return; // only load if we are not already loaded

	if (m_rawImage.size() < m_iWidth*m_iHeight*m_iNumChannels) // sanity check
	{
		engine->showMessageError("Image Error", UString::format("Loaded image has only %i/%i bytes in file %s", m_rawImage.size(), m_iWidth*m_iHeight*m_iNumChannels, m_sFilePath.toUtf8()));
		return;
	}

	if (m_iNumChannels != 4 && m_iNumChannels != 3 && m_iNumChannels != 1) // another sanity check
	{
		engine->showMessageError("Image Error", UString::format("Unsupported number of color channels (%i) in file %s", m_iNumChannels, m_sFilePath.toUtf8()));
		return;
	}

	// create texture and bind
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &m_GLTexture);
	glBindTexture(GL_TEXTURE_2D, m_GLTexture);

	// set texture filtering mode (mipmapping is disabled by default)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_bMipmapped ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLfloat maxAnisotropy;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);

	// texture wrapping, defaults to clamp
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// TODO: HACKHACK: this if block is only here for backwards compatibility with CGProject and (maybe) MetroidModelViewer (needs check)
	if (m_bCreatedImage) // for lightmaps, which are NPOT
	{
		if (m_bClampToEdge)
		{
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}

		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	}

	if (m_bAlignRawDataToBytes)
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//debugLog("going to teximage (mipmapped = %i, alphachannel = %i, width = %i, height = %i, buffer size = %i)\n", (int)m_bMipmapped, (int)m_bHasAlphaChannel, m_iWidth, m_iHeight, m_rawImage.size());

	// m_iNumChannels == 4 ? GL_RGBA : (m_iNumChannels == 3 ? GL_RGB : (m_iNumChannels == 1 ? GL_LUMINANCE : GL_RGBA))
	// set pixel format and load pixels (defaults to no mipmapping)
	if (m_bMipmapped) // TODO: the 4 seems to be wrong? defaults to wrong LOD sometimes... // TODO: gluBuild2DMipmaps() is more or less deprecated, refactor this
		gluBuild2DMipmaps(GL_TEXTURE_2D, 4, m_iWidth, m_iHeight, m_iNumChannels == 4 ? GL_RGBA : (m_iNumChannels == 3 ? GL_RGB : (m_iNumChannels == 1 ? GL_LUMINANCE : GL_RGBA)), GL_UNSIGNED_BYTE, &m_rawImage[0]);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, m_iNumChannels == 4 ? GL_RGBA : (m_iNumChannels == 3 ? GL_RGB : (m_iNumChannels == 1 ? GL_LUMINANCE : GL_RGBA)), m_iWidth, m_iHeight, 0, m_iNumChannels == 4 ? GL_RGBA : (m_iNumChannels == 3 ? GL_RGB : (m_iNumChannels == 1 ? GL_LUMINANCE : GL_RGBA)), GL_UNSIGNED_BYTE, &m_rawImage[0]);

	// free memory
	m_rawImage = std::vector<unsigned char>();

	// check for errors
	int GLerror = glGetError();
	if (GLerror != 0)
	{
		m_GLTexture = 0;
		debugLog("OpenGL Image Error: %i on file %s!\n", GLerror, m_sFilePath.toUtf8());
		engine->showMessageError("Image Error", UString::format("OpenGL Image error %i on file %s", GLerror, m_sFilePath.toUtf8()));
	}
	else
		m_bReady = true;
}

void OpenGLImage::initAsync()
{
	if (m_GLTexture != 0) return; // only load if we are not already loaded

	if (!m_bCreatedImage)
		printf("Resource Manager: Loading %s\n", m_sFilePath.toUtf8());

	// load configuration (modified by the file loaders)

	// if it isn't a created image (created within the engine), load it from the corresponding file
	if (!m_bCreatedImage)
	{
		if (!env->fileExists(m_sFilePath))
		{
			printf("Image Error: Couldn't find file %s !\n", m_sFilePath.toUtf8());
			return;
		}

		// load entire file
		File file(m_sFilePath);
		if (!file.canRead())
		{
			printf("Image Error: Couldn't canRead() %s !\n", m_sFilePath.toUtf8());
			return;
		}
		if (file.getFileSize() < 4)
		{
			printf("Image Error: FileSize is < 4 in %s !\n", m_sFilePath.toUtf8());
			return;
		}
		const char *data = file.readFile();
		if (data == NULL)
		{
			printf("Image Error: Couldn't readFile() %s !\n", m_sFilePath.toUtf8());
			return;
		}

		// determine file type by magic number (png/jpg)
		bool isJPEG = false;
		bool isPNG = false;
		unsigned char buf[4];
		buf[0] = (unsigned char)data[0];
		buf[1] = (unsigned char)data[1];
		buf[2] = (unsigned char)data[2];
		buf[3] = (unsigned char)data[3];
		if (buf[0] == 0xff && buf[1] == 0xD8 && buf[2] == 0xff) // 0xFFD8FF
			isJPEG = true;
		else if (buf[0] == 0x89 && buf[1] == 0x50 && buf[2] == 0x4E && buf[3] == 0x47) // 0x89504E47 (%PNG)
			isPNG = true;

		// depending on the type, load either jpeg or png
		if (isJPEG)
		{
			m_bAlignRawDataToBytes = true;
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
			    return;
			}

			jpeg_mem_src(&cinfo, (unsigned char*)data, file.getFileSize());
			int headerRes = jpeg_read_header(&cinfo, TRUE);
			if (headerRes != JPEG_HEADER_OK)
			{
				jpeg_destroy_decompress(&cinfo);
				printf("Image Error: JPEG read_header() error %i on file %s\n", headerRes, m_sFilePath.toUtf8());
				return;
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
				return;
			}

			//int rowStride = m_iWidth * m_iNumChannels;
			//JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, rowStride, 1);

			// extract each scanline of the image
			jpeg_start_decompress(&cinfo);
			m_rawImage.resize(m_iWidth*m_iHeight*m_iNumChannels);
			JSAMPROW j;
			for (int i=0; i<m_iHeight; ++i)
			{
				j = (&m_rawImage[0] + (i * m_iWidth * m_iNumChannels));
				jpeg_read_scanlines(&cinfo, &j, 1);
			}

			jpeg_finish_decompress(&cinfo);
			jpeg_destroy_decompress(&cinfo);
		}
		else if (isPNG)
		{
			unsigned int width = 0; // yes, these are here on purpose
			unsigned int height = 0;
			unsigned error = lodepng::decode(m_rawImage, width, height, (unsigned char*)data, file.getFileSize());
			m_iWidth = width;
			m_iHeight = height;
			if (error)
			{
				printf("Image Error: PNG error %i (%s) on file %s\n", error, lodepng_error_text(error), m_sFilePath.toUtf8());
				return;
			}
		}
		else
		{
			printf("Image Error, file %s it neither a PNG nor a JPEG image!\n", m_sFilePath.toUtf8());
			return;
		}
	}

	m_bAsyncReady = true;
}

void OpenGLImage::destroy()
{
	if (m_GLTexture != 0)
		glDeleteTextures(1, &m_GLTexture);
	m_GLTexture = 0;

	m_rawImage = std::vector<unsigned char>();
}

void OpenGLImage::bind(unsigned int textureUnit)
{
	if (!m_bReady) return;

	m_iTextureUnitBackup = textureUnit;

	// switch texture units before enabling+binding
	glActiveTexture(GL_TEXTURE0 + m_iTextureUnitBackup);

	// set texture
	glBindTexture(GL_TEXTURE_2D, m_GLTexture);

	// needed for legacy support (OpenGLLegacyInterface)
	glEnable(GL_TEXTURE_2D);
}

void OpenGLImage::unbind()
{
	if (!m_bReady) return;

	// restore texture unit (just in case) and set to no texture
	glActiveTexture(GL_TEXTURE0 + m_iTextureUnitBackup);
	glBindTexture(GL_TEXTURE_2D, 0);

	// restore default texture unit
	if (m_iTextureUnitBackup != 0)
		glActiveTexture(GL_TEXTURE0);
}

void OpenGLImage::setFilterMode(FILTER_MODE filterMode)
{
	if (!m_bReady)
		return;

	bind();
	{
		switch (filterMode)
		{
		case FILTER_MODE_NONE:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;
		case FILTER_MODE_LINEAR:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
		case FILTER_MODE_MIPMAP:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
		}
	}
	unbind();
}

void OpenGLImage::setWrapMode(WRAP_MODE wrapMode)
{
	if (!m_bReady)
		return;

	bind();
	{
		switch (wrapMode)
		{
		case WRAP_MODE_CLAMP: // NOTE: there is also GL_CLAMP, which works a bit differently concerning the border color
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			break;
		case WRAP_MODE_REPEAT:
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			break;
		}
	}
	unbind();
}
