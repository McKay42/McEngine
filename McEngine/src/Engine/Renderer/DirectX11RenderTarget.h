//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		DirectX implementation of RenderTarget / render to texture
//
// $NoKeywords: $drt
//===============================================================================//

#ifndef DIRECTX11RENDERTARGET_H
#define DIRECTX11RENDERTARGET_H

#include "RenderTarget.h"

#ifdef MCENGINE_FEATURE_DIRECTX

#include "d3d11.h"

class DirectX11RenderTarget : public RenderTarget
{
public:
	DirectX11RenderTarget(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType = Graphics::MULTISAMPLE_TYPE::MULTISAMPLE_0X);
	virtual ~DirectX11RenderTarget() {destroy();}

	virtual void enable();
	virtual void disable();

	virtual void bind(unsigned int textureUnit = 0);
	virtual void unbind();

private:
	virtual void init();
	virtual void initAsync();
	virtual void destroy();

	ID3D11Texture2D *m_renderTexture;
	ID3D11RenderTargetView *m_renderTargetView;
	ID3D11ShaderResourceView *m_shaderResourceView;
};

#endif

#endif
