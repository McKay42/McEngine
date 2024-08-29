//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		DirectX implementation of RenderTarget / render to texture
//
// $NoKeywords: $drt
//===============================================================================//

#include "DirectX11RenderTarget.h"

#ifdef MCENGINE_FEATURE_DIRECTX11

#include "Engine.h"
#include "ConVar.h"
#include "VertexArrayObject.h"

#include "DirectX11Interface.h"
#include "DirectX11Shader.h"

DirectX11RenderTarget::DirectX11RenderTarget(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType) : RenderTarget(x, y, width, height, multiSampleType)
{
	m_renderTexture = NULL;
	m_depthStencilTexture = NULL;
	m_renderTargetView = NULL;
	m_depthStencilView = NULL;
	m_shaderResourceView = NULL;

	m_prevRenderTargetView = NULL;
	m_prevDepthStencilView = NULL;

	m_iTextureUnitBackup = 0;
	m_prevShaderResourceView = NULL;

	m_interfaceOverrideHack = NULL;
}

void DirectX11RenderTarget::init()
{
	debugLog("Building RenderTarget (%ix%i) ...\n", (int)m_vSize.x, (int)m_vSize.y);

	HRESULT hr;

	DirectX11Interface *g = ((DirectX11Interface*)engine->getGraphics());
	if (m_interfaceOverrideHack != NULL)
		g = m_interfaceOverrideHack;

	// create color texture
	D3D11_TEXTURE2D_DESC colorTextureDesc;
	{
		colorTextureDesc.ArraySize = 1;
		colorTextureDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
		colorTextureDesc.CPUAccessFlags = 0;
		colorTextureDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
		colorTextureDesc.MipLevels = 1;
		colorTextureDesc.MiscFlags = 0;
		colorTextureDesc.SampleDesc.Count = 1;
		colorTextureDesc.SampleDesc.Quality = 0;
		colorTextureDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		colorTextureDesc.Width = (UINT)m_vSize.x;
		colorTextureDesc.Height = (UINT)m_vSize.y;
	}
	hr = g->getDevice()->CreateTexture2D(&colorTextureDesc, NULL, &m_renderTexture);
	if (FAILED(hr))
	{
		engine->showMessageErrorFatal("RenderTarget Error", UString::format("Couldn't color CreateTexture2D(%ld, %x, %x)!", hr, hr, MAKE_DXGI_HRESULT(hr)));
		return;
	}

	// create depthstencil texture
	D3D11_TEXTURE2D_DESC depthStencilTextureDesc;
	{
		depthStencilTextureDesc.ArraySize = 1;
		depthStencilTextureDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
		depthStencilTextureDesc.CPUAccessFlags = 0;
		depthStencilTextureDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilTextureDesc.MipLevels = 1;
		depthStencilTextureDesc.MiscFlags = 0;
		depthStencilTextureDesc.SampleDesc.Count = 1;
		depthStencilTextureDesc.SampleDesc.Quality = 0;
		depthStencilTextureDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		depthStencilTextureDesc.Width = (UINT)m_vSize.x;
		depthStencilTextureDesc.Height = (UINT)m_vSize.y;
	}
	hr = g->getDevice()->CreateTexture2D(&depthStencilTextureDesc, NULL, &m_depthStencilTexture);
	if (FAILED(hr))
	{
		engine->showMessageErrorFatal("RenderTarget Error", UString::format("Couldn't depthStencil CreateTexture2D(%ld, %x, %x)!", hr, hr, MAKE_DXGI_HRESULT(hr)));
		return;
	}

	// create rendertarget view
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	{
		renderTargetViewDesc.Format = colorTextureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;
	}
	hr = g->getDevice()->CreateRenderTargetView(m_renderTexture, &renderTargetViewDesc, &m_renderTargetView);
	if (FAILED(hr))
	{
		engine->showMessageErrorFatal("RenderTarget Error", UString::format("Couldn't CreateRenderTargetView(%ld, %x, %x)!", hr, hr, MAKE_DXGI_HRESULT(hr)));
		return;
	}

	// create depthstencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	{
		depthStencilViewDesc.Format = depthStencilTextureDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Flags = 0;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
	}
	hr = g->getDevice()->CreateDepthStencilView(m_depthStencilTexture, &depthStencilViewDesc, &m_depthStencilView);
	if (FAILED(hr))
	{
		engine->showMessageErrorFatal("RenderTarget Error", UString::format("Couldn't CreateDepthStencilView(%ld, %x, %x)!", hr, hr, MAKE_DXGI_HRESULT(hr)));
		return;
	}

	// create shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	{
		shaderResourceViewDesc.Format = colorTextureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
	}
	hr = g->getDevice()->CreateShaderResourceView(m_renderTexture, &shaderResourceViewDesc, &m_shaderResourceView);
	if (FAILED(hr))
	{
		engine->showMessageErrorFatal("RenderTarget Error", UString::format("Couldn't CreateShaderResourceView(%ld, %x, %x)!", hr, hr, MAKE_DXGI_HRESULT(hr)));
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

	if (m_depthStencilView != NULL)
		m_depthStencilView->Release();

	if (m_renderTargetView != NULL)
		m_renderTargetView->Release();

	if (m_depthStencilTexture != NULL)
		m_depthStencilTexture->Release();

	if (m_renderTexture != NULL)
		m_renderTexture->Release();

	m_shaderResourceView = NULL;
	m_depthStencilView = NULL;
	m_renderTargetView = NULL;
	m_depthStencilTexture = NULL;
	m_renderTexture = NULL;
}

void DirectX11RenderTarget::draw(Graphics *g, int x, int y)
{
	if (!m_bReady)
	{
		debugLog("WARNING: RenderTarget is not ready!\n");
		return;
	}

	bind();
	{
		g->setColor(m_color);
		g->drawQuad(x, y, m_vSize.x, m_vSize.y);
	}
	unbind();
}

void DirectX11RenderTarget::draw(Graphics *g, int x, int y, int width, int height)
{
	if (!m_bReady)
	{
		debugLog("WARNING: RenderTarget is not ready!\n");
		return;
	}

	bind();
	{
		g->setColor(m_color);
		g->drawQuad(x, y, width, height);
	}
	unbind();
}

void DirectX11RenderTarget::drawRect(Graphics *g, int x, int y, int width, int height)
{
	if (!m_bReady)
	{
		debugLog("WARNING: RenderTarget is not ready!\n");
		return;
	}

	const float texCoordWidth0 = x / m_vSize.x;
	const float texCoordWidth1 = (x+width) / m_vSize.x;
	const float texCoordHeight1 = y / m_vSize.y;
	const float texCoordHeight0 = (y+height) / m_vSize.y;

	bind();
	{
		g->setColor(m_color);

		static VertexArrayObject vao;

		vao.empty();

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
	}
	unbind();
}

void DirectX11RenderTarget::enable()
{
	if (!m_bReady) return;

	DirectX11Interface *dx11 = (DirectX11Interface*)engine->getGraphics();

	// backup
	// HACKHACK: slow af
	{
		dx11->getDeviceContext()->OMGetRenderTargets(1, &m_prevRenderTargetView, &m_prevDepthStencilView);
	}

	dx11->getDeviceContext()->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	// clear
	Color clearColor = m_clearColor;
	if (debug_rt->getBool())
		clearColor = COLORf(0.5f, 0.0f, 0.5f, 0.0f);

	float fClearColor[4] = {COLOR_GET_Rf(clearColor), COLOR_GET_Gf(clearColor), COLOR_GET_Bf(clearColor), COLOR_GET_Af(clearColor)};

	if (m_bClearColorOnDraw)
		dx11->getDeviceContext()->ClearRenderTargetView(m_renderTargetView, fClearColor);

	if (m_bClearDepthOnDraw)
		dx11->getDeviceContext()->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0); // yes, the 1.0f is correct
}

void DirectX11RenderTarget::disable()
{
	if (!m_bReady) return;

	// restore
	// HACKHACK: slow af
	{
		((DirectX11Interface*)engine->getGraphics())->getDeviceContext()->OMSetRenderTargets(1, &m_prevRenderTargetView, m_prevDepthStencilView);

		// refcount
		{
			if (m_prevRenderTargetView != NULL)
			{
				m_prevRenderTargetView->Release();
				m_prevRenderTargetView = NULL;
			}

			if (m_prevDepthStencilView != NULL)
			{
				m_prevDepthStencilView->Release();
				m_prevDepthStencilView = NULL;
			}
		}
	}
}

void DirectX11RenderTarget::bind(unsigned int textureUnit)
{
	if (!m_bReady) return;

	DirectX11Interface *dx11 = (DirectX11Interface*)engine->getGraphics();

	m_iTextureUnitBackup = textureUnit;

	// backup
	// HACKHACK: slow af
	{
		dx11->getDeviceContext()->PSGetShaderResources(textureUnit, 1, &m_prevShaderResourceView);
	}

	dx11->getDeviceContext()->PSSetShaderResources(textureUnit, 1, &m_shaderResourceView);

	dx11->getShaderGeneric()->setUniform1f("misc", 1.0f); // enable texturing
}

void DirectX11RenderTarget::unbind()
{
	if (!m_bReady) return;

	// restore
	// HACKHACK: slow af
	{
		((DirectX11Interface*)engine->getGraphics())->getDeviceContext()->PSSetShaderResources(m_iTextureUnitBackup, 1, &m_prevShaderResourceView);

		// refcount
		{
			if (m_prevShaderResourceView != NULL)
			{
				m_prevShaderResourceView->Release();
				m_prevShaderResourceView = NULL;
			}
		}
	}
}

#endif
