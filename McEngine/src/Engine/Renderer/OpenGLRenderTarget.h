//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		OpenGL implementation of RenderTarget / render to texture
//
// $NoKeywords: $glrt
//===============================================================================//

#ifndef OPENGLRENDERTARGET_H
#define OPENGLRENDERTARGET_H

#include "RenderTarget.h"

#ifdef MCENGINE_FEATURE_OPENGL

class OpenGLRenderTarget : public RenderTarget
{
public:
	OpenGLRenderTarget(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType = Graphics::MULTISAMPLE_TYPE::MULTISAMPLE_0X);
	virtual ~OpenGLRenderTarget() {destroy();}

	virtual void enable();
	virtual void disable();

	virtual void bind(unsigned int textureUnit = 0);
	virtual void unbind();

	// ILLEGAL:
	void blitResolveFrameBufferIntoFrameBuffer(OpenGLRenderTarget *rt);
	void blitFrameBufferIntoFrameBuffer(OpenGLRenderTarget *rt);
	inline unsigned int getFrameBuffer() const {return m_iFrameBuffer;}
	inline unsigned int getRenderTexture() const {return m_iRenderTexture;}
	inline unsigned int getResolveFrameBuffer() const {return m_iResolveFrameBuffer;}
	inline unsigned int getResolveTexture() const {return m_iResolveTexture;}

private:
	virtual void init();
	virtual void initAsync();
	virtual void destroy();

	unsigned int m_iFrameBuffer;
	unsigned int m_iRenderTexture;
	unsigned int m_iDepthBuffer;
	unsigned int m_iResolveFrameBuffer;
	unsigned int m_iResolveTexture;

	int m_iFrameBufferBackup;
	unsigned int m_iTextureUnitBackup;
	int m_iViewportBackup[4];
};

#endif

#endif
