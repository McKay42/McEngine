//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		DirectX implementation of Image
//
// $NoKeywords: $dximg
//===============================================================================//

#ifndef DIRECTX11IMAGE_H
#define DIRECTX11IMAGE_H

#include "Image.h"

#ifdef MCENGINE_FEATURE_DIRECTX

#include "d3d11.h"

class DirectX11Image : public Image
{
public:
	DirectX11Image(UString filepath, bool mipmapped = false);
	DirectX11Image(int width, int height, bool mipmapped = false);
	virtual ~DirectX11Image() {destroy();}

	virtual void bind(unsigned int textureUnit = 0);
	virtual void unbind();

	virtual void setFilterMode(Graphics::FILTER_MODE filterMode);
	virtual void setWrapMode(Graphics::WRAP_MODE wrapMode);

	// ILLEGAL:
	ID3D11Texture2D *getTexture() const {return m_texture;}
	ID3D11ShaderResourceView *getShaderResourceView() const {return m_shaderResourceView;}

private:
	virtual void init();
	virtual void initAsync();
	virtual void destroy();

	ID3D11Texture2D *m_texture;
	ID3D11ShaderResourceView *m_shaderResourceView;

	unsigned int m_iTextureUnitBackup;
	ID3D11ShaderResourceView *m_prevShaderResourceView;
};

#endif

#endif
