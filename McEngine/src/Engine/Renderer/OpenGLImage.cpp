//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		OpenGL implementation of Image
//
// $NoKeywords: $glimg
//===============================================================================//

#include "OpenGLImage.h"

#if defined(MCENGINE_FEATURE_OPENGL) || defined (MCENGINE_FEATURE_OPENGLES)

#include "ResourceManager.h"
#include "Environment.h"
#include "Engine.h"
#include "ConVar.h"
#include "File.h"

#include "OpenGLHeaders.h"

OpenGLImage::OpenGLImage(UString filepath, bool mipmapped, bool keepInSystemMemory) : Image(filepath, mipmapped, keepInSystemMemory)
{
	m_GLTexture = 0;
	m_iTextureUnitBackup = 0;
}

OpenGLImage::OpenGLImage(int width, int height, bool mipmapped, bool keepInSystemMemory) : Image(width, height, mipmapped, keepInSystemMemory)
{
	m_GLTexture = 0;
	m_iTextureUnitBackup = 0;
}

void OpenGLImage::init()
{
	if ((m_GLTexture != 0 && !m_bKeepInSystemMemory) || !m_bAsyncReady) return; // only load if we are not already loaded

	// create texture object
	if (m_GLTexture == 0)
	{
		// DEPRECATED LEGACY (1)
		glEnable(GL_TEXTURE_2D);
		glGetError(); // clear gl error state

		// create texture and bind
		glGenTextures(1, &m_GLTexture);
		glBindTexture(GL_TEXTURE_2D, m_GLTexture);

		// DEPRECATED LEGACY (2)
		glEnable(GL_TEXTURE_2D);
		glGetError(); // clear gl error state

		// set texture filtering mode (mipmapping is disabled by default)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_bMipmapped ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

#ifdef MCENGINE_FEATURE_OPENGL

		// TODO: wtf, why is this even here? causes texture atlas uv cracks/bleeding on point sampled meshes
		/*
		GLfloat maxAnisotropy;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
		glGetError(); // clear gl error state (env LIBGL_ALWAYS_SOFTWARE=1 and mesa would break textures otherwise)
		*/

#endif

		// texture wrapping, defaults to clamp
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	// upload to gpu
	int GLerror = 0;
	{
		glBindTexture(GL_TEXTURE_2D, m_GLTexture);

		const int jpgUnpackAlignment = 1;
		int prevUnpackAlignment = 4;
		if (m_type == Image::TYPE::TYPE_JPG) // HACKHACK: wat
		{
			glGetIntegerv(GL_UNPACK_ALIGNMENT, &prevUnpackAlignment);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		}

		const GLint internalFormat = (m_iNumChannels == 4 ? GL_RGBA : (m_iNumChannels == 3 ? GL_RGB : (m_iNumChannels == 1 ? GL_LUMINANCE : GL_RGBA)));
		const GLint format = (m_iNumChannels == 4 ? GL_RGBA : (m_iNumChannels == 3 ? GL_RGB : (m_iNumChannels == 1 ? GL_LUMINANCE : GL_RGBA)));

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_iWidth, m_iHeight, 0, format, GL_UNSIGNED_BYTE, &m_rawImage[0]);
		if (m_bMipmapped)
		{
			// DEPRECATED LEGACY (1) (ignore mipmap generation errors)
			GLerror = (GLerror == 0 ? glGetError() : GLerror);

			glGenerateMipmap(GL_TEXTURE_2D);

			// DEPRECATED LEGACY (2)
			glGetError(); // clear gl error state
		}

		if (m_type == Image::TYPE::TYPE_JPG && prevUnpackAlignment != jpgUnpackAlignment)
			glPixelStorei(GL_UNPACK_ALIGNMENT, prevUnpackAlignment);
	}

	// free memory
	if (!m_bKeepInSystemMemory)
		m_rawImage = std::vector<unsigned char>();

	// check for errors
	GLerror = (GLerror == 0 ? glGetError() : GLerror);
	if (GLerror != 0)
	{
		m_GLTexture = 0;
		debugLog("OpenGL Image Error: %i on file %s!\n", GLerror, m_sFilePath.toUtf8());
		engine->showMessageError("Image Error", UString::format("OpenGL Image error %i on file %s", GLerror, m_sFilePath.toUtf8()));
		return;
	}

	m_bReady = true;

	if (m_filterMode != Graphics::FILTER_MODE::FILTER_MODE_LINEAR)
		setFilterMode(m_filterMode);

	if (m_wrapMode != Graphics::WRAP_MODE::WRAP_MODE_CLAMP)
		setWrapMode(m_wrapMode);
}

void OpenGLImage::initAsync()
{
	if (m_GLTexture != 0) return; // only load if we are not already loaded

	if (!m_bCreatedImage)
	{
		if (ResourceManager::debug_rm->getBool())
			debugLog("Resource Manager: Loading %s\n", m_sFilePath.toUtf8());

		m_bAsyncReady = loadRawImage();
	}
}

void OpenGLImage::destroy()
{
	if (m_GLTexture != 0)
	{
		glDeleteTextures(1, &m_GLTexture);
		m_GLTexture = 0;
	}

	m_rawImage = std::vector<unsigned char>();
}

void OpenGLImage::bind(unsigned int textureUnit)
{
	if (!m_bReady) return;

	m_iTextureUnitBackup = textureUnit;

	// switch texture units before enabling+binding
	glActiveTexture(GL_TEXTURE0 + textureUnit);

	// set texture
	glBindTexture(GL_TEXTURE_2D, m_GLTexture);

	// needed for legacy support (OpenGLLegacyInterface)
	// DEPRECATED LEGACY
	glEnable(GL_TEXTURE_2D);
	glGetError(); // clear gl error state
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

void OpenGLImage::setFilterMode(Graphics::FILTER_MODE filterMode)
{
	Image::setFilterMode(filterMode);
	if (!m_bReady) return;

	bind();
	{
		switch (filterMode)
		{
		case Graphics::FILTER_MODE::FILTER_MODE_NONE:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;
		case Graphics::FILTER_MODE::FILTER_MODE_LINEAR:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
		case Graphics::FILTER_MODE::FILTER_MODE_MIPMAP:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
		}
	}
	unbind();
}

void OpenGLImage::setWrapMode(Graphics::WRAP_MODE wrapMode)
{
	Image::setWrapMode(wrapMode);
	if (!m_bReady) return;

	bind();
	{
		switch (wrapMode)
		{
		case Graphics::WRAP_MODE::WRAP_MODE_CLAMP: // NOTE: there is also GL_CLAMP, which works a bit differently concerning the border color
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			break;
		case Graphics::WRAP_MODE::WRAP_MODE_REPEAT:
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			break;
		}
	}
	unbind();
}

void OpenGLImage::handleGLErrors()
{
	int GLerror = glGetError();
	if (GLerror != 0)
		debugLog("OpenGL Image Error: %i on file %s!\n", GLerror, m_sFilePath.toUtf8());
}

#endif
