//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		raw DirectX 11 graphics interface
//
// $NoKeywords: $dx11i
//===============================================================================//

#ifndef DIRECTX11INTERFACE_H
#define DIRECTX11INTERFACE_H

#include "cbase.h"
#include "NullGraphicsInterface.h"

#ifdef MCENGINE_FEATURE_DIRECTX

#include "d3d11.h"

class DirectX11Shader;

class DirectX11Interface : public NullGraphicsInterface /*Graphics*/
{
public:
	DirectX11Interface(HWND hwnd, bool minimalistContext = false);
	virtual ~DirectX11Interface();

	// scene
	virtual void beginScene();
	virtual void endScene();

	// color
	virtual void setColor(Color color);
	virtual void setAlpha(float alpha);

	// 2d primitive drawing
	virtual void drawPixel(int x, int y);
	virtual void drawLine(int x1, int y1, int x2, int y2);
	virtual void drawLine(Vector2 pos1, Vector2 pos2);
	virtual void drawRect(int x, int y, int width, int height);
	virtual void drawRect(int x, int y, int width, int height, Color top, Color right, Color bottom, Color left);

	virtual void fillRect(int x, int y, int width, int height);
	virtual void fillGradient(int x, int y, int width, int height, Color topLeftColor, Color topRightColor, Color bottomLeftColor, Color bottomRightColor);

	virtual void drawQuad(int x, int y, int width, int height);
	virtual void drawQuad(Vector2 topLeft, Vector2 topRight, Vector2 bottomRight, Vector2 bottomLeft, Color topLeftColor, Color topRightColor, Color bottomRightColor, Color bottomLeftColor);

	// 2d resource drawing
	virtual void drawImage(Image *image);
	virtual void drawString(McFont *font, UString text);

	// 3d type drawing
	virtual void drawVAO(VertexArrayObject *vao);

	// DEPRECATED: 2d clipping
	virtual void setClipRect(McRect clipRect);
	virtual void pushClipRect(McRect clipRect);
	virtual void popClipRect();

	// renderer settings
	virtual void setClipping(bool enabled);
	virtual void setBlending(bool enabled);
	virtual void setDepthBuffer(bool enabled);
	virtual void setCulling(bool culling);
	virtual void setAntialiasing(bool aa);
	virtual void setWireframe(bool enabled);

	// renderer actions
	virtual void flush();
	virtual std::vector<unsigned char> getScreenshot();

	// device settings
	virtual void setVSync(bool vsync);

	// callbacks
	virtual void onResolutionChange(Vector2 newResolution);

	// factory
	virtual Image *createImage(UString filePath, bool mipmapped, bool keepInSystemMemory);
	virtual Image *createImage(int width, int height, bool mipmapped, bool keepInSystemMemory);
	virtual RenderTarget *createRenderTarget(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType);
	virtual Shader *createShaderFromFile(UString vertexShaderFilePath, UString fragmentShaderFilePath);
	virtual Shader *createShaderFromSource(UString vertexShader, UString fragmentShader);

	// ILLEGAL:
	inline bool isReady() const {return m_bReady;}
	ID3D11Device *getDevice() const {return m_device;}
	ID3D11DeviceContext *getDeviceContext() const {return m_deviceContext;}
	IDXGISwapChain *getSwapChain() const {return m_swapChain;}
	DirectX11Shader *getShaderGeneric() const {return m_shaderTexturedGeneric;}

protected:
	virtual void init();
	virtual void onTransformUpdate(Matrix4 &projectionMatrix, Matrix4 &worldMatrix);

private:
	static int primitiveToDirectX(Graphics::PRIMITIVE primitive);

	bool m_bReady;

	// device context
	HWND m_hwnd;
	bool m_bMinimalistContext;

	// d3d
	ID3D11Device *m_device;
	ID3D11DeviceContext *m_deviceContext;
	IDXGISwapChain *m_swapChain;
	ID3D11RenderTargetView *m_frameBuffer;

	// renderer
	Vector2 m_vResolution;

	ID3D11RasterizerState *m_rasterizerState;
	D3D11_RASTERIZER_DESC m_rasterizerDesc;

	ID3D11DepthStencilState *m_depthStencilState;
	D3D11_DEPTH_STENCIL_DESC m_depthStencilDesc;

	ID3D11BlendState *m_blendState;
	D3D11_BLEND_DESC m_blendDesc;

	Matrix4 m_projectionMatrix;
	Matrix4 m_worldMatrix;
	Matrix4 m_MP;

	DirectX11Shader *m_shaderTexturedGeneric;

	struct SimpleVertex
	{
		Vector3 pos;
		Vector4 col;
		Vector2 tex;
	};
	std::vector<SimpleVertex> m_vertices;
	ID3D11Buffer *m_vertexBuffer;

	// persistent vars
	bool m_bVSync;
	Color m_color;

	// clipping
	std::stack<McRect> m_clipRectStack;
};

#endif

#endif
