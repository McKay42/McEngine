//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		OpenGL implementation of RenderTarget / render to texture
//
// $NoKeywords: $glrt
//===============================================================================//

#include "OpenGLRenderTarget.h"

#ifdef MCENGINE_FEATURE_OPENGL

#include "Engine.h"
#include "ConVar.h"
#include "VertexArrayObject.h"

#include "OpenGLHeaders.h"

OpenGLRenderTarget::OpenGLRenderTarget(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType) : RenderTarget(x, y, width, height, multiSampleType)
{
	m_iFrameBuffer = m_iRenderTexture = m_iDepthBuffer = m_iResolveTexture = m_iResolveFrameBuffer = 0;
	m_iFrameBufferBackup = m_iTextureUnitBackup = 0;
	m_iViewportBackup[0] = 0;
	m_iViewportBackup[1] = 0;
	m_iViewportBackup[2] = 0;
	m_iViewportBackup[3] = 0;
}

void OpenGLRenderTarget::init()
{
	debugLog("Building RenderTarget (%ix%i) ...\n", (int)m_vSize.x, (int)m_vSize.y);

	m_iFrameBuffer = m_iRenderTexture = m_iDepthBuffer = m_iResolveTexture = m_iResolveFrameBuffer = 0;

	int numMultiSamples = 2;
	switch (m_multiSampleType)
	{
	case Graphics::MULTISAMPLE_TYPE::MULTISAMPLE_2X:
		numMultiSamples = 2;
		break;
	case Graphics::MULTISAMPLE_TYPE::MULTISAMPLE_4X:
		numMultiSamples = 4;
		break;
	case Graphics::MULTISAMPLE_TYPE::MULTISAMPLE_8X:
		numMultiSamples = 8;
		break;
	case Graphics::MULTISAMPLE_TYPE::MULTISAMPLE_16X:
		numMultiSamples = 16;
		break;
	}

	// create framebuffer
	glGenFramebuffers(1, &m_iFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_iFrameBuffer);
	if (m_iFrameBuffer == 0)
	{
		engine->showMessageError("RenderTarget Error", "Couldn't glGenFramebuffers() or glBindFramebuffer()!");
		return;
	}

	// create depth buffer
	glGenRenderbuffers(1, &m_iDepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_iDepthBuffer);
	if (m_iDepthBuffer == 0)
	{
		engine->showMessageError("RenderTarget Error", "Couldn't glGenRenderBuffers() or glBindRenderBuffer()!");
		return;
	}
	// fill depth buffer
	if (isMultiSampled())
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, numMultiSamples, GL_DEPTH_COMPONENT, (int)m_vSize.x, (int)m_vSize.y);
	else
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (int)m_vSize.x, (int)m_vSize.y);

	// set depth buffer as depth attachment on the framebuffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_iDepthBuffer);

	// create texture
	glGenTextures(1, &m_iRenderTexture);
	glBindTexture(isMultiSampled() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, m_iRenderTexture);
	if (m_iRenderTexture == 0)
	{
		engine->showMessageError("RenderTarget Error", "Couldn't glGenTextures() or glBindTexture()!");
		return;
	}

	// fill texture
	if (isMultiSampled())
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, numMultiSamples, GL_RGBA8, (int)m_vSize.x, (int)m_vSize.y, true); // use fixed sample locations
	else
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (int)m_vSize.x, (int)m_vSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		// set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0); // no mipmapping atm
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // disable texture wrap
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	// set render texture as color attachment0 on the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, isMultiSampled() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, m_iRenderTexture, 0);

	// if multisampled, create resolve framebuffer/texture
	if (isMultiSampled())
	{
		if (m_iResolveFrameBuffer == 0)
		{
			// create resolve framebuffer
			glGenFramebuffers(1, &m_iResolveFrameBuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, m_iResolveFrameBuffer);
			if (m_iResolveFrameBuffer == 0)
			{
				engine->showMessageError("RenderTarget Error", "Couldn't glGenFramebuffers() or glBindFramebuffer() multisampled!");
				return;
			}

			// create resolve texture
			glGenTextures(1, &m_iResolveTexture);
			glBindTexture(GL_TEXTURE_2D, m_iResolveTexture);
			if (m_iResolveTexture == 0)
			{
				engine->showMessageError("RenderTarget Error", "Couldn't glGenTextures() or glBindTexture() multisampled!");
				return;
			}

			// set texture parameters
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0); // no mips

			// fill texture
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (int)m_vSize.x, (int)m_vSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

			// set resolve texture as color attachment0 on the resolve framebuffer
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_iResolveTexture, 0);
		}
	}

	// error checking
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		engine->showMessageError("RenderTarget Error", UString::format("!GL_FRAMEBUFFER_COMPLETE, size = (%ix%i), multisampled = %i", (int)m_vSize.x, (int)m_vSize.y, (int)isMultiSampled()));
		return;
	}

	// reset bound texture and framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_bReady = true;
}

void OpenGLRenderTarget::initAsync()
{
	m_bAsyncReady = true;
}

void OpenGLRenderTarget::destroy()
{
	if (m_iResolveTexture != 0)
		glDeleteTextures(1, &m_iResolveTexture);
	if (m_iResolveFrameBuffer != 0)
		glDeleteFramebuffers(1, &m_iResolveFrameBuffer);
	if (m_iRenderTexture != 0)
		glDeleteTextures(1, &m_iRenderTexture);
	if (m_iDepthBuffer != 0)
		glDeleteRenderbuffers(1, &m_iDepthBuffer);
	if (m_iFrameBuffer != 0)
		glDeleteFramebuffers(1, &m_iFrameBuffer);

	m_iDepthBuffer = m_iRenderTexture = m_iFrameBuffer = m_iResolveTexture = m_iResolveFrameBuffer = 0;
}

void OpenGLRenderTarget::enable()
{
	if (!m_bReady) return;

	// bind framebuffer
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_iFrameBufferBackup); // backup
	glBindFramebuffer(GL_FRAMEBUFFER, m_iFrameBuffer);

	// set new viewport
	glGetIntegerv(GL_VIEWPORT, m_iViewportBackup); // backup
	glViewport( -m_vPos.x, (m_vPos.y-engine->getGraphics()->getResolution().y)+m_vSize.y, engine->getGraphics()->getResolution().x, engine->getGraphics()->getResolution().y);

	// clear
	if (debug_rt->getBool())
		glClearColor(0.0f, 0.5f, 0.0f, 0.5f);
	else
		glClearColor(COLOR_GET_Rf(m_clearColor), COLOR_GET_Gf(m_clearColor), COLOR_GET_Bf(m_clearColor), COLOR_GET_Af(m_clearColor));

	if (m_bClearColorOnDraw || m_bClearDepthOnDraw)
		glClear((m_bClearColorOnDraw ? GL_COLOR_BUFFER_BIT : 0) | (m_bClearDepthOnDraw ? GL_DEPTH_BUFFER_BIT : 0));
}

void OpenGLRenderTarget::disable()
{
	if (!m_bReady) return;

	// if multisampled, blit content for multisampling into resolve texture
	if (isMultiSampled())
	{
		// HACKHACK: force disable antialiasing
		engine->getGraphics()->setAntialiasing(false);

	 	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_iFrameBuffer);
	    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_iResolveFrameBuffer);

	    // for multisampled, the sizes MUST be the same! you can't blit from multisampled into non-multisampled or different size
	    glBlitFramebuffer(0, 0, (int)m_vSize.x, (int)m_vSize.y, 0, 0, (int)m_vSize.x, (int)m_vSize.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	 	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

	// restore viewport
	glViewport(m_iViewportBackup[0], m_iViewportBackup[1], m_iViewportBackup[2], m_iViewportBackup[3]);

	// restore framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_iFrameBufferBackup);
}

void OpenGLRenderTarget::bind(unsigned int textureUnit)
{
	if (!m_bReady) return;

	m_iTextureUnitBackup = textureUnit;

	// switch texture units before enabling+binding
	glActiveTexture(GL_TEXTURE0 + textureUnit);

	// set texture
	glBindTexture(GL_TEXTURE_2D, isMultiSampled() ? m_iResolveTexture : m_iRenderTexture);

	// needed for legacy support (OpenGLLegacyInterface)
	// DEPRECATED LEGACY
	glEnable(GL_TEXTURE_2D);
	glGetError(); // clear gl error state
}

void OpenGLRenderTarget::unbind()
{
	if (!m_bReady) return;

	// restore texture unit (just in case) and set to no texture
	glActiveTexture(GL_TEXTURE0 + m_iTextureUnitBackup);
	glBindTexture(GL_TEXTURE_2D, 0);

	// restore default texture unit
	if (m_iTextureUnitBackup != 0)
		glActiveTexture(GL_TEXTURE0);
}

void OpenGLRenderTarget::blitResolveFrameBufferIntoFrameBuffer(OpenGLRenderTarget *rt)
{
	if (isMultiSampled())
	{
		// HACKHACK: force disable antialiasing
		engine->getGraphics()->setAntialiasing(false);

	 	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_iResolveFrameBuffer);
	    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rt->getFrameBuffer());

	    glBlitFramebuffer(0, 0, (int)m_vSize.x, (int)m_vSize.y, 0, 0, (int)rt->getWidth(), (int)rt->getHeight(), GL_COLOR_BUFFER_BIT, GL_LINEAR);

	 	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}
}

void OpenGLRenderTarget::blitFrameBufferIntoFrameBuffer(OpenGLRenderTarget *rt)
{
	// HACKHACK: force disable antialiasing
	engine->getGraphics()->setAntialiasing(false);

 	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_iFrameBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rt->getFrameBuffer());

    glBlitFramebuffer(0, 0, (int)m_vSize.x, (int)m_vSize.y, 0, 0, (int)rt->getWidth(), (int)rt->getHeight(), GL_COLOR_BUFFER_BIT, GL_LINEAR);

 	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

#endif
