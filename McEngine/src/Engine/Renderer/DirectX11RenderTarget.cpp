//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		DirectX implementation of RenderTarget / render to texture
//
// $NoKeywords: $drt
//===============================================================================//

#include "DirectX11RenderTarget.h"

#ifdef MCENGINE_FEATURE_DIRECTX

#include "Engine.h"

#include "DirectX11Interface.h"

DirectX11RenderTarget::DirectX11RenderTarget(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType) : RenderTarget(x, y, width, height, multiSampleType)
{
	m_renderTexture = NULL;
	m_renderTargetView = NULL;
	m_shaderResourceView = NULL;
}

void DirectX11RenderTarget::init()
{
	debugLog("Building RenderTarget (%ix%i) ...\n", (int)m_vSize.x, (int)m_vSize.y);

	// create texture
	D3D11_TEXTURE2D_DESC bufferDesc;
	bufferDesc.ArraySize = 1;
	bufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	bufferDesc.MipLevels = 1;
	bufferDesc.MiscFlags = 0;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.Width = (UINT)m_vSize.x;
	bufferDesc.Height = (UINT)m_vSize.y;

	HRESULT hr = ((DirectX11Interface*)engine->getGraphics())->getDevice()->CreateTexture2D(&bufferDesc, NULL, &m_renderTexture);
	if (FAILED(hr))
	{
		engine->showMessageErrorFatal("RenderTarget Error", UString::format("Couldn't CreateTexture2D(%ld)!", hr));
		return;
	}

	// create rendertarget view
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = bufferDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	hr = ((DirectX11Interface*)engine->getGraphics())->getDevice()->CreateRenderTargetView(m_renderTexture, &renderTargetViewDesc, &m_renderTargetView);
	if (FAILED(hr))
	{
		engine->showMessageErrorFatal("RenderTarget Error", UString::format("Couldn't CreateRenderTargetView(%ld)!", hr));
		return;
	}

	// create shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = bufferDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	hr = ((DirectX11Interface*)engine->getGraphics())->getDevice()->CreateShaderResourceView(m_renderTexture, &shaderResourceViewDesc, &m_shaderResourceView);
	if (FAILED(hr))
	{
		engine->showMessageErrorFatal("RenderTarget Error", UString::format("Couldn't CreateShaderResourceView(%ld)!", hr));
		return;
	}

	m_bReady = true;
}

void DirectX11RenderTarget::initAsync()
{
	m_bAsyncReady = true;
}

void DirectX11RenderTarget::destroy()
{
	if (m_shaderResourceView != NULL)
		m_shaderResourceView->Release();

	if (m_renderTargetView != NULL)
		m_renderTargetView->Release();

	if (m_renderTexture != NULL)
		m_renderTexture->Release();
}

void DirectX11RenderTarget::enable()
{
	if (!m_bReady) return;

	// TODO:
}

void DirectX11RenderTarget::disable()
{
	if (!m_bReady) return;

	// TODO:
}

void DirectX11RenderTarget::bind(unsigned int textureUnit)
{
	if (!m_bReady) return;

	// TODO:
}

void DirectX11RenderTarget::unbind()
{
	if (!m_bReady) return;

	// TODO:
}

#endif
