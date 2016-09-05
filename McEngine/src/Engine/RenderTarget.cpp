//================ Copyright (c) 2013, PG, All rights reserved. =================//
//
// Purpose:		offscreen rendering
//
// $NoKeywords: $rt
//===============================================================================//

#include "RenderTarget.h"
#include "Engine.h"
#include "ConVar.h"
#include "Camera.h"
#include "VertexArrayObject.h"

#include "OpenGLHeaders.h"

ConVar debug_rt("debug_rt", false, "draws all rendertargets with a translucent green background");

RenderTarget::RenderTarget(int x, int y, int width, int height)
{
	m_bReady = false;
	m_bClearColorOnDraw = true;
	m_bClearDepthOnDraw = true;

	m_vPos = Vector2(x,y);
	m_vSize = Vector2(width,height);
	m_color = 0xffffffff;

	build();
}

RenderTarget::~RenderTarget()
{
	release();
}

void RenderTarget::rebuild(int x, int y, int width, int height)
{
	m_bReady = false;

	m_vPos = Vector2(x,y);
	m_vSize = Vector2(width,height);

	release();
	build();
}

void RenderTarget::build()
{
	debugLog("Building RenderTarget (%ix%i) ...\n", (int)m_vSize.x, (int)m_vSize.y);

	m_bReady = false;

	m_iFrameBuffer = m_iRenderTexture = m_iDepthBuffer = 0;

	// create framebuffer
	glGenFramebuffers(1, &m_iFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_iFrameBuffer);

	if (m_iFrameBuffer == 0)
	{
		engine->showMessageError("RenderTarget Error", "Couldn't glGenFramebuffers() or glBindFramebuffer()");
		return;
	}

	// create texture & bind
	glGenTextures(1, &m_iRenderTexture);
	glBindTexture(GL_TEXTURE_2D, m_iRenderTexture);

	if (m_iRenderTexture == 0)
	{
		engine->showMessageError("RenderTarget Error", "Couldn't glGenTextures() or glBindTexture()");
		return;
	}

	// fill texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (int)m_vSize.x, (int)m_vSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	// set filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// set depth buffer
	glGenRenderbuffers(1, &m_iDepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_iDepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (int)m_vSize.x, (int)m_vSize.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_iDepthBuffer);

	// disable texture wrap
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// set the render texture as color attachment0 of the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_iRenderTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		engine->showMessageError("RenderTarget Error", "!GL_FRAMEBUFFER_COMPLETE");
		return;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_bReady = true;
}

void RenderTarget::release()
{
	m_bReady = false;

	glDeleteRenderbuffers(1, &m_iDepthBuffer);
	glDeleteFramebuffers(1, &m_iFrameBuffer);
	glDeleteTextures(1, &m_iRenderTexture);
}

void RenderTarget::enable()
{
	if (!m_bReady) return;

	// bind framebuffer
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_iFrameBufferBackup); // backup
	glBindFramebuffer(GL_FRAMEBUFFER, m_iFrameBuffer);

	/*
	// set projection matrix
	engine->getGraphics()->pushTransform();
	Matrix4 flippedProjectionMatrix = Camera::buildMatrixOrtho2D(0, engine->getScreenWidth(), 0, engine->getScreenHeight());
	engine->getGraphics()->setProjectionMatrix(flippedProjectionMatrix);
	*/

	// set new viewport
	glPushAttrib(GL_VIEWPORT_BIT); // backup
	glViewport( -m_vPos.x, (m_vPos.y-engine->getGraphics()->getResolution().y)+m_vSize.y, engine->getGraphics()->getResolution().x, engine->getGraphics()->getResolution().y);

	// clear
	if (debug_rt.getBool())
		glClearColor (0.0f, 0.5f, 0.0f, 0.5f);
	else
		glClearColor (0.0f, 0.0f, 0.0f, 0.0f);

	if (m_bClearColorOnDraw || m_bClearDepthOnDraw)
		glClear((m_bClearColorOnDraw ? GL_COLOR_BUFFER_BIT : 0) | (m_bClearDepthOnDraw ? GL_DEPTH_BUFFER_BIT : 0));
}

void RenderTarget::disable()
{
	if (!m_bReady) return;

	// restore viewport
	glPopAttrib();

	/*
	// restore projection matrix
	engine->getGraphics()->popTransform();
	*/

	// restore framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_iFrameBufferBackup);
}

void RenderTarget::bind(unsigned int textureUnit)
{
	if (!m_bReady) return;

	// switch texture units before enabling+binding
	glActiveTexture(GL_TEXTURE0 + textureUnit);

	// set texture
	glBindTexture(GL_TEXTURE_2D, m_iRenderTexture);

	// needed for legacy support (OpenGLLegacyInterface)
	glEnable(GL_TEXTURE_2D);
}

void RenderTarget::unbind()
{
	if (!m_bReady) return;

	// restore default texture unit
	glActiveTexture(GL_TEXTURE0);

	// restore to no texture (0)
	glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderTarget::draw(Graphics *g, int x, int y)
{
	if (!m_bReady)
	{
		debugLog("WARNING: RenderTarget is not ready!\n");
		return;
	}

	bind();
		g->setColor(m_color);

		// can't use drawQuad here because opengl fucks shit up and flips framebuffers vertically (due to bottom left opengl origin and top left engine origin)
		///g->drawQuad(x, y, m_vSize.x, m_vSize.y);

		// compromise: all draw*() functions of the RenderTarget class guarantee correctly flipped images.
		//             if bind() is used, no guarantee can be made about the texture orientation (assuming an anonymous Renderer)

		VertexArrayObject vao;

		vao.addTexcoord(0, 1);
		vao.addVertex(x, y);

		vao.addTexcoord(0, 0);
		vao.addVertex(x, y+m_vSize.y);

		vao.addTexcoord(1, 0);
		vao.addVertex(x+m_vSize.x, y+m_vSize.y);

		vao.addTexcoord(1, 0);
		vao.addVertex(x+m_vSize.x, y+m_vSize.y);

		vao.addTexcoord(1, 1);
		vao.addVertex(x+m_vSize.x, y);

		vao.addTexcoord(0, 1);
		vao.addVertex(x, y);

		g->drawVAO(&vao);
	unbind();
}

void RenderTarget::draw(Graphics *g, int x, int y, int width, int height)
{
	if (!m_bReady)
	{
		debugLog("WARNING: RenderTarget is not ready!\n");
		return;
	}

	bind();
		g->setColor(m_color);

		VertexArrayObject vao;

		vao.addTexcoord(0, 1);
		vao.addVertex(x, y);

		vao.addTexcoord(0, 0);
		vao.addVertex(x, y+height);

		vao.addTexcoord(1, 0);
		vao.addVertex(x+width, y+height);

		vao.addTexcoord(1, 0);
		vao.addVertex(x+width, y+height);

		vao.addTexcoord(1, 1);
		vao.addVertex(x+width, y);

		vao.addTexcoord(0, 1);
		vao.addVertex(x, y);

		g->drawVAO(&vao);
	unbind();
}

void RenderTarget::drawRect(Graphics *g, int x, int y, int width, int height)
{
	if (!m_bReady)
	{
		debugLog("WARNING: RenderTarget is not ready!\n");
		return;
	}

	const float texCoordWidth0 = x / m_vSize.x;
	const float texCoordWidth1 = (x+width) / m_vSize.x;
	const float texCoordHeight1 = 1.0f - y / m_vSize.y;
	const float texCoordHeight0 = 1.0f - (y+height) / m_vSize.y;

	bind();
		g->setColor(m_color);

		// can't use drawQuad here because opengl fucks shit up and flips framebuffers vertically (due to bottom left opengl origin and top left engine origin)
		///g->drawQuad(x, y, m_vSize.x, m_vSize.y);

		// compromise: all draw*() functions of the RenderTarget class guarantee correctly flipped images.
		//             if bind() is used, no guarantee can be made about the texture orientation (assuming an anonymous Renderer)

		VertexArrayObject vao;

		vao.addTexcoord(texCoordWidth0, texCoordHeight1);
		vao.addVertex(x, y);

		vao.addTexcoord(texCoordWidth0, texCoordHeight0);
		vao.addVertex(x, y+height);

		vao.addTexcoord(texCoordWidth1, texCoordHeight0);
		vao.addVertex(x+width, y+height);

		vao.addTexcoord(texCoordWidth1, texCoordHeight0);
		vao.addVertex(x+width, y+height);

		vao.addTexcoord(texCoordWidth1, texCoordHeight1);
		vao.addVertex(x+width, y);

		vao.addTexcoord(texCoordWidth0, texCoordHeight1);
		vao.addVertex(x, y);

		g->drawVAO(&vao);
	unbind();
}
