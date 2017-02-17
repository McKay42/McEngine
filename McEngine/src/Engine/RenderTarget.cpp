//================ Copyright (c) 2013, PG, All rights reserved. =================//
//
// Purpose:		offscreen rendering
//
// $NoKeywords: $rt
//===============================================================================//

#include "RenderTarget.h"
#include "Engine.h"
#include "ConVar.h"
#include "VertexArrayObject.h"

ConVar _debug_rt("debug_rt", false, "draws all rendertargets with a translucent green background");
ConVar *RenderTarget::debug_rt = &_debug_rt;

RenderTarget::RenderTarget(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType)
{
	m_vPos = Vector2(x, y);
	m_vSize = Vector2(width, height);
	m_multiSampleType = multiSampleType;

	m_bClearColorOnDraw = true;
	m_bClearDepthOnDraw = true;

	m_color = 0xffffffff;
	m_clearColor = 0x00000000;
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

void RenderTarget::rebuild(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType)
{
	m_vPos.x = x;
	m_vPos.y = y;
	m_vSize.x = width;
	m_vSize.y = height;
	m_multiSampleType = multiSampleType;

	reload();
}

void RenderTarget::rebuild(int x, int y, int width, int height)
{
	rebuild(x, y, width, height, m_multiSampleType);
}

void RenderTarget::rebuild(int width, int height)
{
	rebuild(m_vPos.x, m_vPos.y, width, height, m_multiSampleType);
}

void RenderTarget::rebuild(int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType)
{
	rebuild(m_vPos.x, m_vPos.y, width, height, multiSampleType);
}
