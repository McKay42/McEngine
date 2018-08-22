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
#include "DirectX11Shader.h"

DirectX11Image::DirectX11Image(UString filepath, bool mipmapped, bool keepInSystemMemory) : Image(filepath, mipmapped, keepInSystemMemory)
{
	m_texture = NULL;
	m_shaderResourceView = NULL;

	m_iTextureUnitBackup = 0;
	m_prevShaderResourceView = NULL;

	m_interfaceOverrideHack = NULL;
	m_bShared = false;
}

DirectX11Image::DirectX11Image(int width, int height, bool mipmapped, bool keepInSystemMemory) : Image(width, height, mipmapped, keepInSystemMemory)
{
	m_texture = NULL;
	m_shaderResourceView = NULL;

	m_iTextureUnitBackup = 0;
	m_prevShaderResourceView = NULL;

	m_interfaceOverrideHack = NULL;
	m_bShared = false;
}

void DirectX11Image::init()
{
	if ((m_texture != NULL && !m_bKeepInSystemMemory) || !m_bAsyncReady) return; // only load if we are not already loaded

	HRESULT hr;

	DirectX11Interface *g = ((DirectX11Interface*)engine->getGraphics());
	if (m_interfaceOverrideHack != NULL)
		g = m_interfaceOverrideHack;

	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = (UINT)m_iWidth;
	textureDesc.Height = (UINT)m_iHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = m_iNumChannels == 4 ? DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM : (m_iNumChannels == 3 ? DXGI_FORMAT_R8_UNORM : (m_iNumChannels == 1 ? DXGI_FORMAT_R8_UNORM : DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM));
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = (m_bKeepInSystemMemory ? D3D11_USAGE::D3D11_USAGE_DYNAMIC : D3D11_USAGE::D3D11_USAGE_DEFAULT);
	textureDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = (m_bKeepInSystemMemory ? D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE : 0);
	textureDesc.MiscFlags = m_bShared ? D3D11_RESOURCE_MISC_FLAG::D3D11_RESOURCE_MISC_SHARED : 0;

	if (m_texture == NULL)
	{
		// create texture (with initial data)
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = (void*)&m_rawImage[0];
		initData.SysMemPitch = static_cast<UINT>(m_iWidth*m_iNumChannels);
		initData.SysMemSlicePitch = 0;

		hr = g->getDevice()->CreateTexture2D(&textureDesc, (m_rawImage.size() >= m_iWidth*m_iHeight*m_iNumChannels ? &initData : NULL), &m_texture);
		if (FAILED(hr) || m_texture == NULL)
		{
			debugLog("DirectX Image Error: Couldn't CreateTexture2D(%ld, %x, %x) on file %s!\n", hr, hr, MAKE_DXGI_HRESULT(hr), m_sFilePath.toUtf8());
			engine->showMessageError("Image Error", UString::format("DirectX Image error, couldn't CreateTexture2D(%ld, %x, %x) on file %s", hr, hr, MAKE_DXGI_HRESULT(hr), m_sFilePath.toUtf8()));
			return;
		}
	}
	else
	{
		// upload data to existing texture
		// TODO: Map(), upload m_rawImage, Unmap()
	}

	// free memory
	if (!m_bKeepInSystemMemory)
		m_rawImage = std::vector<unsigned char>();

	// create shader resource view
	if (m_shaderResourceView == NULL)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		memset(&shaderResourceViewDesc, 0, sizeof(shaderResourceViewDesc));
		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		hr = g->getDevice()->CreateShaderResourceView(m_texture, &shaderResourceViewDesc, &m_shaderResourceView);
		if (FAILED(hr) || m_shaderResourceView == NULL)
		{
			m_texture->Release(); m_texture = NULL;
			debugLog("DirectX Image Error: Couldn't CreateShaderResourceView(%ld, %x, %x) on file %s!\n", hr, hr, MAKE_DXGI_HRESULT(hr), m_sFilePath.toUtf8());
			engine->showMessageError("Image Error", UString::format("DirectX Image error, couldn't CreateShaderResourceView(%ld, %x, %x) on file %s", hr, hr, MAKE_DXGI_HRESULT(hr), m_sFilePath.toUtf8()));
			return;
		}
	}

	m_bReady = true;
}

void DirectX11Image::initAsync()
{
	if (!m_bCreatedImage)
	{
		printf("Resource Manager: Loading %s\n", m_sFilePath.toUtf8());
		m_bAsyncReady = loadRawImage();

		// rewrite all non-4-channels-per-pixel formats, because directx doesn't have any fucking 24bpp formats ffs
		if (m_bAsyncReady)
		{
			const int numTargetChannels = 4;
			const int numMissingChannels = numTargetChannels - m_iNumChannels;

			if (numMissingChannels > 0)
			{
				std::vector<unsigned char> newRawImage;
				newRawImage.reserve(m_iWidth*m_iHeight*numTargetChannels);

				for (size_t i=0; i<m_rawImage.size(); i+=m_iNumChannels) // for every pixel
				{
					// add original data
					for (int p=0; p<m_iNumChannels; p++)
					{
						newRawImage.push_back(m_rawImage[i + p]);
					}

					// add padded data
					if (m_iNumChannels == 1)
					{
						newRawImage.push_back(m_rawImage[i + 0]);	// G
						newRawImage.push_back(m_rawImage[i + 0]);	// B
						newRawImage.push_back(0xff);				// A
					}
					else if (m_iNumChannels == 3)
					{
						newRawImage.push_back(0xff);				// A
					}
					else
					{
						for (int m=0; m<numMissingChannels; m++)
						{
							newRawImage.push_back(0xff);			// B, A
						}
					}
				}

				m_rawImage = std::move(newRawImage);
				m_iNumChannels = numTargetChannels;
			}
		}
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

	// HACKHACK: TEMP:
	((DirectX11Interface*)engine->getGraphics())->getShaderGeneric()->setUniform1f("misc", 1.0f); // enable texturing
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
