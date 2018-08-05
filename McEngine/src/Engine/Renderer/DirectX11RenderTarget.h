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

class DirectX11Interface;

class DirectX11RenderTarget : public RenderTarget
{
public:
	DirectX11RenderTarget(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType = Graphics::MULTISAMPLE_TYPE::MULTISAMPLE_0X);
	virtual ~DirectX11RenderTarget() {destroy();}

	virtual void draw(Graphics *g, int x, int y);
	virtual void draw(Graphics *g, int x, int y, int width, int height);
	virtual void drawRect(Graphics *g, int x, int y, int width, int height);

	virtual void enable();
	virtual void disable();

	virtual void bind(unsigned int textureUnit = 0);
	virtual void unbind();

	// ILLEGAL:
	void setDirectX11InterfaceHack(DirectX11Interface *dxi) {m_interfaceOverrideHack = dxi;}
	inline ID3D11Texture2D *getRenderTexture() const {return m_renderTexture;}

private:
	virtual void init();
	virtual void initAsync();
	virtual void destroy();

	ID3D11Texture2D *m_renderTexture;
	ID3D11Texture2D *m_depthStencilTexture;
	ID3D11RenderTargetView *m_renderTargetView;
	ID3D11DepthStencilView *m_depthStencilView;
	ID3D11ShaderResourceView *m_shaderResourceView;

	ID3D11RenderTargetView *m_prevRenderTargetView;
	ID3D11DepthStencilView *m_prevDepthStencilView;

	unsigned int m_iTextureUnitBackup;
	ID3D11ShaderResourceView *m_prevShaderResourceView;

	DirectX11Interface *m_interfaceOverrideHack;
};

#endif

#endif
