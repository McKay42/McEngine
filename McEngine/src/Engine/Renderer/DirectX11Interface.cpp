//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		raw DirectX 11 graphics interface
//
// $NoKeywords: $dx11i
//===============================================================================//

#include "DirectX11Interface.h"

#ifdef MCENGINE_FEATURE_DIRECTX

#include "Engine.h"
#include "ConVar.h"
#include "Camera.h"
#include "VertexArrayObject.h"

#include "Font.h"
#include "DirectX11Image.h"
#include "DirectX11Shader.h"
#include "DirectX11RenderTarget.h"

#include "d3dx9math.h" // is there no modern version of this?

DirectX11Interface::DirectX11Interface(HWND hwnd) : NullGraphicsInterface()
{
	m_hwnd = hwnd;

	m_device = NULL;
	m_deviceContext = NULL;
	m_swapChain = NULL;
	m_frameBuffer = NULL;

	// renderer
	m_vResolution = engine->getScreenSize(); // initial viewport size = window size
	m_rasterizerState = NULL;
	m_blendState = NULL;
	m_shaderTexturedGeneric = NULL;
	m_vertexBuffer = NULL;

	// persistent vars
	m_color = 0xffffffff;
	m_bVSync = false;
}

DirectX11Interface::~DirectX11Interface()
{
	SAFE_DELETE(m_shaderTexturedGeneric);

	if (m_vertexBuffer != NULL)
		m_vertexBuffer->Release();

	if (m_rasterizerState != NULL)
		m_rasterizerState->Release();

	if (m_swapChain != NULL)
		m_swapChain->Release();

	if (m_frameBuffer != NULL)
		m_frameBuffer->Release();

	if (m_device != NULL)
		m_device->Release();

	if (m_deviceContext != NULL)
		m_deviceContext->Release();
}

void DirectX11Interface::init()
{
	// backbuffer descriptor
	DXGI_MODE_DESC swapChainBufferDesc;
	ZeroMemory(&swapChainBufferDesc, sizeof(DXGI_MODE_DESC));
	swapChainBufferDesc.Width = (UINT)m_vResolution.x; // TODO:!!!
	swapChainBufferDesc.Height = (UINT)m_vResolution.y; // TODO:!!!
	swapChainBufferDesc.RefreshRate.Numerator = 144; // TODO:!!!
	swapChainBufferDesc.RefreshRate.Denominator = 1;
	swapChainBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainBufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainBufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// swapchain descriptor
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferDesc = swapChainBufferDesc;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = m_hwnd;
	swapChainDesc.Windowed = TRUE; // TODO:!!!
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// feature levels
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = _countof(featureLevels);
    D3D_FEATURE_LEVEL featureLevelOut;

    // flags
    UINT createDeviceFlags = 0;
    createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;

	// create swapchain
	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, &featureLevelOut, &m_deviceContext);
	if (FAILED(hr))
	{
		engine->showMessageErrorFatal("DirectX Error", UString::format("Couldn't D3D11CreateDeviceAndSwapChain(%ld)!\nThe engine will quit now.", hr));
		engine->shutdown();
		return;
	}

	// default rasterizer settings
	m_rasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	m_rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
	m_rasterizerDesc.FrontCounterClockwise = FALSE;
	m_rasterizerDesc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
	m_rasterizerDesc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
	m_rasterizerDesc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	m_rasterizerDesc.DepthClipEnable = FALSE;
	m_rasterizerDesc.ScissorEnable = FALSE;
	m_rasterizerDesc.MultisampleEnable = FALSE;
	m_rasterizerDesc.AntialiasedLineEnable = FALSE;
	m_device->CreateRasterizerState(&m_rasterizerDesc, &m_rasterizerState);
	m_deviceContext->RSSetState(m_rasterizerState);

	// TODO: default depthStencil settings D3D11_DEPTH_STENCIL_DESC

	// default blending settings
	ZeroMemory(&m_blendDesc, sizeof(D3D11_BLEND_DESC));
	m_blendDesc.AlphaToCoverageEnable = FALSE;
	m_blendDesc.RenderTarget[0].BlendEnable = true;
	m_blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	m_blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	m_blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	m_blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
	m_blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	m_blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	m_blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_device->CreateBlendState(&m_blendDesc, &m_blendState);
	m_deviceContext->OMSetBlendState(m_blendState, NULL, 0xffffffff);

	// create default shader
	UString vertexShader =
		"cbuffer ModelViewProjectionConstantBuffer : register(b0)\n"
		"{\n"
		"	float4x4 mvp;		// world matrix for object\n"
		//"	matrix world;		// world matrix for object\n"
		//"	matrix view;		// view matrix\n"
		//"	matrix projection;	// projection matrix\n"
		"	float4 col;			// global color\n"
		"};\n"
		"\n"
		"struct VS_INPUT\n"
		"{\n"
		"	float4 pos	: POSITION;\n"
		"	float4 col	: COLOR0;\n"
		"	float2 tex	: TEXCOORD0;\n"
		"};\n"
		"\n"
		"struct VS_OUTPUT\n"
		"{\n"
		"	float4 pos	: SV_Position;\n"
		"	float4 col	: COLOR0;\n"
		"	float2 tex	: TEXCOORD0;\n"
		"};\n"
		"\n"
		"VS_OUTPUT vsmain(in VS_INPUT In)\n"
		"{\n"
		"	VS_OUTPUT Out;"
		"	Out.pos = mul(In.pos, mvp);\n"
		"	Out.col = In.col;\n"
		"	Out.tex = In.tex;\n"
		"	return Out;\n"
		"}\n";


	UString pixelShader =
		"Texture2D tex2D;\n"
		"SamplerState samplerState\n"
		"{\n"
		"	Filter = MIN_MAG_MIP_LINEAR;"
		"	AddressU = Clamp;"
		"	AddressV = Clamp;"
		"};\n"
		"\n"
		"struct PS_INPUT\n"
		"{\n"
		"	float4 pos	: SV_Position;\n"
		"	float4 col	: COLOR0;\n"
		"	float2 tex	: TEXCOORD0;\n"
		"};\n"
		"\n"
		"struct PS_OUTPUT\n"
		"{\n"
		"	float4 col	: SV_Target;\n"
		"};\n"
		"\n"
		"PS_OUTPUT psmain(in PS_INPUT In)\n"
		"{\n"
		"	PS_OUTPUT Out;"
		//"	Out.col = float4(0.2f, 1.0f, 1.0f, 0.2f);"
		//"	Out.col = In.col;"
		"	Out.col = tex2D.Sample(samplerState, In.tex) * In.col;"
		"	return Out;\n"
		"}\n";

	m_shaderTexturedGeneric = (DirectX11Shader*)createShaderFromSource(vertexShader, pixelShader);
	m_shaderTexturedGeneric->load();

	// dynamic vertexbuffer
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(SimpleVertex) * 512;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	if (FAILED(m_device->CreateBuffer(&bufferDesc, NULL, &m_vertexBuffer)))
		engine->showMessageError("DirectX Error", "Couldn't CreateBuffer()!");

    onResolutionChange(m_vResolution); // force build swapchain rendertarget view
}

void DirectX11Interface::beginScene()
{
	Matrix4 defaultProjectionMatrix = Camera::buildMatrixOrtho2D(0, m_vResolution.x, m_vResolution.y, 0);

	// push main transforms
	pushTransform();
	setProjectionMatrix(defaultProjectionMatrix);
	translate(r_globaloffset_x->getFloat(), r_globaloffset_y->getFloat());

	// and apply them
	updateTransform();

	// clear
	D3DXCOLOR clearColor(0.0f, 0.0f, 0.0f, 0.0f);
	m_deviceContext->ClearRenderTargetView(m_frameBuffer, clearColor);

	// enable default shader
	m_shaderTexturedGeneric->enable();
}

void DirectX11Interface::endScene()
{
	popTransform();

	checkStackLeaks();

	m_swapChain->Present(m_bVSync ? 1 : 0, 0);
}

void DirectX11Interface::setColor(Color color)
{
	m_color = color;
	// TODO: wtf is happening there
	///m_shaderTexturedGeneric->setUniform4f("col", COLOR_GET_Af(m_color), COLOR_GET_Rf(m_color), COLOR_GET_Gf(m_color), COLOR_GET_Bf(m_color));
}

void DirectX11Interface::setAlpha(float alpha)
{
	m_color &= 0x00ffffff;
	m_color |= ((int)(255.0f * alpha)) << 24;
	setColor(m_color);
}

void DirectX11Interface::drawLine(int x1, int y1, int x2, int y2)
{
	updateTransform();

	VertexArrayObject vao(Graphics::PRIMITIVE::PRIMITIVE_LINES);
	vao.addVertex(x1 + 0.5f, y1 + 0.5f);
	vao.addVertex(x2 + 0.5f, y2 + 0.5f);
	drawVAO(&vao);
}

void DirectX11Interface::drawLine(Vector2 pos1, Vector2 pos2)
{
	drawLine(pos1.x, pos1.y, pos2.x, pos2.y);
}

void DirectX11Interface::drawRect(int x, int y, int width, int height)
{
	drawLine(x, y, x+width, y);
	drawLine(x, y, x, y+height);
	drawLine(x, y+height, x+width+1, y+height);
	drawLine(x+width, y, x+width, y+height);
}

void DirectX11Interface::drawRect(int x, int y, int width, int height, Color top, Color right, Color bottom, Color left)
{
	setColor(top);
	drawLine(x, y, x+width, y);
	setColor(left);
	drawLine(x, y, x, y+height);
	setColor(bottom);
	drawLine(x, y+height, x+width+1, y+height);
	setColor(right);
	drawLine(x+width, y, x+width, y+height);
}

void DirectX11Interface::fillRect(int x, int y, int width, int height)
{
	updateTransform();

	VertexArrayObject vao(Graphics::PRIMITIVE::PRIMITIVE_QUADS);
	vao.addVertex(x, y);
	vao.addVertex(x, y + height);
	vao.addVertex(x + width, y + height);
	vao.addVertex(x + width, y);
	drawVAO(&vao);
}

void DirectX11Interface::drawImage(Image *image)
{
	if (image == NULL)
	{
		debugLog("WARNING: Tried to draw image with NULL texture!\n");
		return;
	}
	if (!image->isReady())
		return;

	updateTransform();

	float width = image->getWidth();
	float height = image->getHeight();

	float x = -width/2;
	float y = -height/2;

	VertexArrayObject vao(Graphics::PRIMITIVE::PRIMITIVE_QUADS);
	vao.addVertex(x, y);
	vao.addTexcoord(0, 0);
	vao.addVertex(x, y + height);
	vao.addTexcoord(0, 1);
	vao.addVertex(x + width, y + height);
	vao.addTexcoord(1, 1);
	vao.addVertex(x + width, y);
	vao.addTexcoord(1, 0);

	image->bind();
	drawVAO(&vao);
	image->unbind();

	if (r_debug_drawimage->getBool())
	{
		setColor(0xbbff00ff);
		drawRect(x, y, width, height);
	}
}

void DirectX11Interface::drawString(McFont *font, UString text)
{
	if (font == NULL || text.length() < 1 || !font->isReady())
		return;

	updateTransform();

	font->drawString(this, text);
}

void DirectX11Interface::drawVAO(VertexArrayObject *vao)
{
	if (vao == NULL) return;

	updateTransform();

	// if baked, then we can directly draw the buffer
	if (vao->isReady())
	{
		// TODO:!!!
		return;
	}

	const std::vector<Vector3> &vertices = vao->getVertices();
	const std::vector<Vector3> &normals = vao->getNormals();
	const std::vector<std::vector<Vector2>> &texcoords = vao->getTexcoords();
	const std::vector<Color> &vcolors = vao->getColors();

	if (vertices.size() < 2) return;

	// no support for quads, because fuck you
	// rewrite all quads into triangles
	std::vector<Vector3> finalVertices = vertices;
	std::vector<std::vector<Vector2>> finalTexcoords = texcoords;
	std::vector<Vector4> colors;
	std::vector<Vector4> finalColors;

	for (int i=0; i<vcolors.size(); i++)
	{
		Vector4 color = Vector4(COLOR_GET_Rf(vcolors[i]), COLOR_GET_Gf(vcolors[i]), COLOR_GET_Bf(vcolors[i]), COLOR_GET_Af(vcolors[i]));
		colors.push_back(color);
		finalColors.push_back(color);
	}
	int maxColorIndex = colors.size() - 1;

	Graphics::PRIMITIVE primitive = vao->getPrimitive();
	if (primitive == Graphics::PRIMITIVE::PRIMITIVE_QUADS)
	{
		finalVertices.clear();
		for (int t=0; t<finalTexcoords.size(); t++)
		{
			finalTexcoords[t].clear();
		}
		finalColors.clear();
		primitive = Graphics::PRIMITIVE::PRIMITIVE_TRIANGLES;

		if (vertices.size() > 3)
		{
			for (int i=0; i<vertices.size(); i+=4)
			{
				finalVertices.push_back(vertices[i + 0]);
				finalVertices.push_back(vertices[i + 1]);
				finalVertices.push_back(vertices[i + 2]);

				for (int t=0; t<texcoords.size(); t++)
				{
					finalTexcoords[t].push_back(texcoords[t][i + 0]);
					finalTexcoords[t].push_back(texcoords[t][i + 1]);
					finalTexcoords[t].push_back(texcoords[t][i + 2]);
				}

				if (colors.size() > 0)
				{
					finalColors.push_back(colors[clamp<int>(i + 0, 0, maxColorIndex)]);
					finalColors.push_back(colors[clamp<int>(i + 1, 0, maxColorIndex)]);
					finalColors.push_back(colors[clamp<int>(i + 2, 0, maxColorIndex)]);
				}

				finalVertices.push_back(vertices[i + 0]);
				finalVertices.push_back(vertices[i + 2]);
				finalVertices.push_back(vertices[i + 3]);

				for (int t=0; t<texcoords.size(); t++)
				{
					finalTexcoords[t].push_back(texcoords[t][i + 0]);
					finalTexcoords[t].push_back(texcoords[t][i + 2]);
					finalTexcoords[t].push_back(texcoords[t][i + 3]);
				}

				if (colors.size() > 0)
				{
					finalColors.push_back(colors[clamp<int>(i + 0, 0, maxColorIndex)]);
					finalColors.push_back(colors[clamp<int>(i + 2, 0, maxColorIndex)]);
					finalColors.push_back(colors[clamp<int>(i + 3, 0, maxColorIndex)]);
				}
			}
		}
	}

	// build directx vertices
	m_vertices.clear();
	for (int i=0; i<finalVertices.size(); i++)
	{
		SimpleVertex v;
		v.pos = finalVertices[i];
		v.col = Vector4(COLOR_GET_Rf(m_color), COLOR_GET_Gf(m_color), COLOR_GET_Bf(m_color), COLOR_GET_Af(m_color));

		if (finalTexcoords.size() > 0 && finalTexcoords[0].size() > 0 && i < finalTexcoords[0].size())
			v.tex = finalTexcoords[0][i];
		else
			v.tex = Vector2(0, 0);

		m_vertices.push_back(v);
	}

	// upload everything to gpu
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	if (SUCCEEDED(m_deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		memcpy(mappedResource.pData, &m_vertices[0], std::min(sizeof(SimpleVertex) * 512, sizeof(SimpleVertex) * m_vertices.size()));
		m_deviceContext->Unmap(m_vertexBuffer, 0);
	}
	else
	{
		debugLog("DirectX Error: Couldn't Map() vertexbuffer!\n");
		return;
	}

	// draw it
	const UINT stride = sizeof(SimpleVertex);
	const UINT offset = 0;
	m_deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	m_deviceContext->IASetPrimitiveTopology((D3D_PRIMITIVE_TOPOLOGY)primitiveToDirectX(primitive));
	m_deviceContext->Draw(m_vertices.size(), 0);
}

void DirectX11Interface::setClipping(bool enabled)
{
	m_rasterizerState->Release();
	m_rasterizerDesc.ScissorEnable = enabled ? TRUE : FALSE;
	m_device->CreateRasterizerState(&m_rasterizerDesc, &m_rasterizerState);
	m_deviceContext->RSSetState(m_rasterizerState);
}

void DirectX11Interface::setBlending(bool enabled)
{
	m_blendState->Release();
	m_blendDesc.RenderTarget[0].BlendEnable = enabled ? TRUE : FALSE;
	m_device->CreateBlendState(&m_blendDesc, &m_blendState);
	m_deviceContext->OMSetBlendState(m_blendState, NULL, 0xffffffff);
}

void DirectX11Interface::setDepthBuffer(bool enabled)
{
	m_rasterizerState->Release();
	m_rasterizerDesc.DepthClipEnable = enabled ? TRUE : FALSE;
	m_device->CreateRasterizerState(&m_rasterizerDesc, &m_rasterizerState);
	m_deviceContext->RSSetState(m_rasterizerState);
}

void DirectX11Interface::setCulling(bool culling)
{
	m_rasterizerState->Release();
	m_rasterizerDesc.CullMode = culling ? D3D11_CULL_MODE::D3D11_CULL_BACK : D3D11_CULL_MODE::D3D11_CULL_NONE;
	m_device->CreateRasterizerState(&m_rasterizerDesc, &m_rasterizerState);
	m_deviceContext->RSSetState(m_rasterizerState);
}

void DirectX11Interface::setAntialiasing(bool aa)
{
	m_rasterizerState->Release();
	m_rasterizerDesc.MultisampleEnable = aa ? TRUE : FALSE;
	m_device->CreateRasterizerState(&m_rasterizerDesc, &m_rasterizerState);
	m_deviceContext->RSSetState(m_rasterizerState);
}

void DirectX11Interface::setWireframe(bool enabled)
{
	m_rasterizerState->Release();
	m_rasterizerDesc.FillMode = enabled ? D3D11_FILL_MODE::D3D11_FILL_WIREFRAME : D3D11_FILL_MODE::D3D11_FILL_SOLID;
	m_device->CreateRasterizerState(&m_rasterizerDesc, &m_rasterizerState);
	m_deviceContext->RSSetState(m_rasterizerState);
}

void DirectX11Interface::setVSync(bool vsync)
{
	m_bVSync = vsync;
}

void DirectX11Interface::onResolutionChange(Vector2 newResolution)
{
	m_vResolution = newResolution;

	// rebuild swapchain rendertarget + view
	{
		HRESULT hr;

		// unset + release
		m_deviceContext->OMSetRenderTargets(0, NULL, NULL);
		if (m_frameBuffer != NULL)
			m_frameBuffer->Release();

		// resize
		hr = m_swapChain->ResizeBuffers(0, (UINT)newResolution.x, (UINT)newResolution.y, DXGI_FORMAT_UNKNOWN, 0); // this preserves the existing format
		if (FAILED(hr))
		{
			debugLog("FATAL ERROR: DirectX11Interface::onResolutionChange() couldn't ResizeBuffers(%ld)!!!\n", hr);
			return;
		}

		// rebuild
		ID3D11Texture2D *backBuffer;
		hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
		if (FAILED(hr))
		{
			debugLog("FATAL ERROR: DirectX11Interface::onResolutionChange() couldn't GetBuffer(%ld)!!!\n", hr);
			return;
		}

		hr = m_device->CreateRenderTargetView(backBuffer, NULL, &m_frameBuffer);
		if (FAILED(hr))
		{
			debugLog("FATAL ERROR: DirectX11Interface::onResolutionChange() couldn't CreateRenderTargetView(%ld)!!!\n", hr);
			backBuffer->Release();
			return;
		}
		backBuffer->Release();

		m_deviceContext->OMSetRenderTargets(1, &m_frameBuffer, NULL);
	}

	// rebuild viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = m_vResolution.x;
	viewport.Height = m_vResolution.y;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_deviceContext->RSSetViewports(1, &viewport);
}

Image *DirectX11Interface::createImage(UString filePath, bool mipmapped)
{
	return new DirectX11Image(filePath, mipmapped);
}

Image *DirectX11Interface::createImage(int width, int height, bool mipmapped)
{
	return new DirectX11Image(width, height, mipmapped);
}

RenderTarget *DirectX11Interface::createRenderTarget(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType)
{
	return new DirectX11RenderTarget(x, y, width, height, multiSampleType);
}

Shader *DirectX11Interface::createShaderFromFile(UString vertexShaderFilePath, UString fragmentShaderFilePath)
{
	return new DirectX11Shader(vertexShaderFilePath, fragmentShaderFilePath, false);
}

Shader *DirectX11Interface::createShaderFromSource(UString vertexShader, UString fragmentShader)
{
	return new DirectX11Shader(vertexShader, fragmentShader, true);
}

void DirectX11Interface::onTransformUpdate(Matrix4 &projectionMatrix, Matrix4 &worldMatrix)
{
	m_projectionMatrix = projectionMatrix;
	m_worldMatrix = worldMatrix;

	m_MP = m_projectionMatrix * m_worldMatrix;
	m_shaderTexturedGeneric->setUniformMatrix4fv("mvp", m_MP);
}

int DirectX11Interface::primitiveToDirectX(Graphics::PRIMITIVE primitive)
{
	switch (primitive)
	{
	case Graphics::PRIMITIVE::PRIMITIVE_LINES:
		return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	case Graphics::PRIMITIVE::PRIMITIVE_LINE_STRIP:
		return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
	case Graphics::PRIMITIVE::PRIMITIVE_TRIANGLES:
		return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case Graphics::PRIMITIVE::PRIMITIVE_TRIANGLE_FAN: // not available!
		return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
	case Graphics::PRIMITIVE::PRIMITIVE_TRIANGLE_STRIP:
		return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	case Graphics::PRIMITIVE::PRIMITIVE_QUADS: // not available!
		return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
	}

	return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

#endif

