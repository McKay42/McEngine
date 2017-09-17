//================ Copyright (c) 2013, PG, All rights reserved. =================//
//
// Purpose:		box shadows
//
// $NoKeywords: $bshad
//===============================================================================//

// TODO: fix this

#include "CBaseUIBoxShadow.h"

#include "Engine.h"
#include "ConVar.h"
#include "ResourceManager.h"

#include "Shader.h"
#include "RenderTarget.h"
#include "GaussianBlurKernel.h"

/*
// HACKHACK: renderer dependent
#include "OpenGLHeaders.h"
*/

ConVar debug_box_shadows("debug_box_shadows", false);

CBaseUIBoxShadow::CBaseUIBoxShadow(Color color, float radius, float xPos, float yPos, float xSize, float ySize, UString name) : CBaseUIElement(xPos,yPos,xSize,ySize,name)
{
	m_shadowColor = color;
	m_color = color;
	m_fRadius = radius;
	m_bNeedsRedraw = true;
	m_bColoredContent = false;

	m_blur = new GaussianBlur(0, 0, m_vSize.x+m_fRadius*2,m_vSize.y+m_fRadius*2, 91, m_fRadius);
}

CBaseUIBoxShadow::~CBaseUIBoxShadow()
{
	SAFE_DELETE(m_blur);
}

void CBaseUIBoxShadow::draw(Graphics *g)
{
	if (m_bNeedsRedraw)
	{
		render(g);
		m_bNeedsRedraw = false;
	}

	if (debug_box_shadows.getBool())
	{
		g->setColor(0xff00ff00);
		g->drawRect(m_vPos.x-m_fRadius,m_vPos.y-m_fRadius, m_blur->getSize().x, m_blur->getSize().y);
	}

	if (!m_bVisible) return;

	/*
	// HACKHACK: switching blend funcs
	if (m_bColoredContent)
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	*/

	g->setColor(m_color);
		m_blur->draw(g, m_vPos.x-m_fRadius, m_vPos.y-m_fRadius);

	/*
	if (m_bColoredContent)
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	*/
}

void CBaseUIBoxShadow::render(Graphics *g)
{
	/*
	// HACKHACK: switching blend funcs
	if (m_bColoredContent)
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	*/

	g->setClipping(false);
		m_blur->enable();
			g->setColor(m_shadowColor);
			g->fillRect(m_fRadius+2, m_blur->getSize().y/2.0f - m_vSize.y/2.0f, m_vSize.x-4, m_vSize.y);
		m_blur->disable(g);
	g->setClipping(true);

	/*
	if (m_bColoredContent)
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	*/
}

void CBaseUIBoxShadow::renderOffscreen(Graphics *g)
{
	if (m_bNeedsRedraw)
	{
		render(g);
		m_bNeedsRedraw = false;
	}
}

CBaseUIBoxShadow *CBaseUIBoxShadow::setColor(Color color)
{
	m_blur->setColor(color);
	m_color = color;
	return this;
}

CBaseUIBoxShadow *CBaseUIBoxShadow::setShadowColor(Color color)
{
	m_bNeedsRedraw = true;
	m_shadowColor = color;
	return this;
}

CBaseUIBoxShadow *CBaseUIBoxShadow::setColoredContent(bool coloredContent)
{
	m_bColoredContent = coloredContent;

	if (m_bColoredContent)
		m_blur->setSize(Vector2(m_vSize.x+m_fRadius*5, m_vSize.y+m_fRadius*5));

	m_bNeedsRedraw = true;

	return this;
}

void CBaseUIBoxShadow::onResized()
{
	if (m_bColoredContent)
		m_blur->setSize(Vector2(m_vSize.x+m_fRadius*5, m_vSize.y+m_fRadius*5));
	else
		m_blur->setSize(Vector2(m_vSize.x+m_fRadius*2, m_vSize.y+m_fRadius*2));

	m_bNeedsRedraw = true;
}


//*******************************************************************************************************************************************************************************************

//**********************************//
//	Implementation of Gaussianblur  //
//**********************************//

GaussianBlur::GaussianBlur(int x, int y, int width, int height, int kernelSize, float radius)
{
	m_vPos = Vector2(x,y);
	m_vSize = Vector2(width,height);
	m_iKernelSize = kernelSize;
	m_fRadius = radius;

	m_kernel = new GaussianBlurKernel(kernelSize, radius, width, height);
	m_rt = engine->getResourceManager()->createRenderTarget(x, y, width, height);
	m_rt2 = engine->getResourceManager()->createRenderTarget(x, y, width, height);
	m_blurShader = engine->getResourceManager()->loadShader("blur.vsh", "blur.fsh", "gblur");
}

GaussianBlur::~GaussianBlur()
{
	engine->getResourceManager()->destroyResource(m_rt);
	m_rt = NULL;
	engine->getResourceManager()->destroyResource(m_rt2);
	m_rt2 = NULL;
	SAFE_DELETE(m_kernel);
}

void GaussianBlur::draw(Graphics *g, int x, int y)
{
	m_rt->draw(g, x, y);

	//g->setColor(0xffff0000);
	//g->fillRect(x,y,m_vSize.x, m_vSize.y);
}

void GaussianBlur::enable()
{
	m_rt->enable();
}

void GaussianBlur::disable(Graphics *g)
{
	m_rt->disable();

	Shader *blur = m_blurShader;

	m_rt2->enable();
		blur->enable();
			blur->setUniform1i("kernelSize", m_kernel->getKernelSize());
			blur->setUniform1fv("weights", m_kernel->getKernelSize(), m_kernel->getKernel());
			blur->setUniform1fv("offsets", m_kernel->getKernelSize(), m_kernel->getOffsetsVertical());
			blur->setUniform1i("orientation", 1);
			m_rt->draw(g, 0, 0);
		blur->disable();
	m_rt2->disable();

	m_rt->enable();
		blur->enable();
			blur->setUniform1i("kernelSize", m_kernel->getKernelSize());
			blur->setUniform1fv("weights", m_kernel->getKernelSize(), m_kernel->getKernel());
			blur->setUniform1fv("offsets", m_kernel->getKernelSize(), m_kernel->getOffsetsHorizontal());
			blur->setUniform1i("orientation", 0);
			m_rt2->draw(g, m_vPos.x, m_vPos.y);
		blur->disable();
	m_rt->disable();
}

void GaussianBlur::setColor(Color color)
{
	m_rt->setColor(color);
	m_rt2->setColor(color);
}

void GaussianBlur::setSize(Vector2 size)
{
	m_vSize = size;

	SAFE_DELETE(m_kernel);
	m_kernel = new GaussianBlurKernel(m_iKernelSize, m_fRadius, m_vSize.x, m_vSize.y);

	m_rt->rebuild(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y);
	m_rt2->rebuild(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y);
}
