//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		DirectX implementation of Image
//
// $NoKeywords: $dximg
//===============================================================================//

#include "DirectX11Image.h"

#ifdef MCENGINE_FEATURE_DIRECTX

#include "Engine.h"

#include "DirectX11Interface.h"

DirectX11Image::DirectX11Image(UString filepath, bool mipmapped) : Image(filepath, mipmapped)
{
	m_texture = NULL;
	m_shaderResourceView = NULL;

	m_iTextureUnitBackup = 0;
	m_prevShaderResourceView = NULL;
}

DirectX11Image::DirectX11Image(int width, int height, bool mipmapped) : Image(width, height, mipmapped)
{
	m_texture = NULL;
	m_shaderResourceView = NULL;

	m_iTextureUnitBackup = 0;
	m_prevShaderResourceView = NULL;
}

void DirectX11Image::init()
{
	if (m_texture != NULL || !m_bAsyncReady) return; // only load if we are not already loaded

	// TODO: directx doesn't have any fucking 24bpp formats ffs
	if (m_iNumChannels == 3)
		return;

	HRESULT hr;

	// create texture
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = (UINT)m_iWidth;
	textureDesc.Height = (UINT)m_iHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = m_iNumChannels == 4 ? DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM : (m_iNumChannels == 3 ? DXGI_FORMAT_R8_UNORM : (m_iNumChannels == 1 ? DXGI_FORMAT_R8_UNORM : DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM));
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = (void*)&m_rawImage[0];
	initData.SysMemPitch = static_cast<UINT>(m_iWidth*m_iNumChannels);
	initData.SysMemSlicePitch = static_cast<UINT>(m_iNumChannels*m_iWidth*m_iHeight);

	hr = ((DirectX11Interface*)engine->getGraphics())->getDevice()->CreateTexture2D(&textureDesc, &initData, &m_texture);
	if (FAILED(hr) || m_texture == NULL)
	{
		debugLog("DirectX Image Error: Couldn't CreateTexture2D(%ld) on file %s!\n", hr, m_sFilePath.toUtf8());
		engine->showMessageError("Image Error", UString::format("DirectX Image error, couldn't CreateTexture2D(%ld) on file %s", hr, m_sFilePath.toUtf8()));
		return;
	}

	// create shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	memset(&shaderResourceViewDesc, 0, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	hr = ((DirectX11Interface*)engine->getGraphics())->getDevice()->CreateShaderResourceView(m_texture, &shaderResourceViewDesc, &m_shaderResourceView);
	if (FAILED(hr) || m_shaderResourceView == NULL)
	{
		m_texture->Release(); m_texture = NULL;
		debugLog("DirectX Image Error: Couldn't CreateShaderResourceView(%ld) on file %s!\n", hr, m_sFilePath.toUtf8());
		engine->showMessageError("Image Error", UString::format("DirectX Image error, couldn't CreateShaderResourceView(%ld) on file %s", hr, m_sFilePath.toUtf8()));
		return;
	}

	m_bReady = true;
}

void DirectX11Image::initAsync()
{
	if (!m_bCreatedImage)
	{
		printf("Resource Manager: Loading %s\n", m_sFilePath.toUtf8());
		m_bAsyncReady = loadRawImage();
	}
}

void DirectX11Image::destroy()
{
	if (m_shaderResourceView != NULL)
	{
		m_shaderResourceView->Release();
		m_shaderResourceView = NULL;
	}

	if (m_texture != NULL)
	{
		m_texture->Release();
		m_texture = NULL;
	}

	m_rawImage = std::vector<unsigned char>();
}

void DirectX11Image::bind(unsigned int textureUnit)
{
	if (!m_bReady) return;

	m_iTextureUnitBackup = textureUnit;

	((DirectX11Interface*)engine->getGraphics())->getDeviceContext()->PSGetShaderResources(textureUnit, 1, &m_prevShaderResourceView); // backup

	((DirectX11Interface*)engine->getGraphics())->getDeviceContext()->PSSetShaderResources(textureUnit, 1, &m_shaderResourceView);
}

void DirectX11Image::unbind()
{
	if (!m_bReady) return;

	((DirectX11Interface*)engine->getGraphics())->getDeviceContext()->PSSetShaderResources(m_iTextureUnitBackup, 1, &m_prevShaderResourceView); // restore
}

void DirectX11Image::setFilterMode(Graphics::FILTER_MODE filterMode)
{
	if (!m_bReady) return;
	// TODO:
}

void DirectX11Image::setWrapMode(Graphics::WRAP_MODE wrapMode)
{
	if (!m_bReady) return;
	// TODO:
}

#endif
