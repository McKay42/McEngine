//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		raw DirectX 11 graphics interface
//
// $NoKeywords: $dx11i
//===============================================================================//

#include "DirectX11Interface.h"

#ifdef MCENGINE_FEATURE_DIRECTX11

#include "Engine.h"
#include "ConVar.h"
#include "Camera.h"

#include "VisualProfiler.h"
#include "ResourceManager.h"

#include "DirectX11Image.h"
#include "DirectX11Shader.h"
#include "DirectX11RenderTarget.h"
#include "DirectX11VertexArrayObject.h"

#include "d3dx9math.h" // is there no modern version of this?

//#define MCENGINE_D3D11_CREATE_DEVICE_DEBUG

DirectX11Interface::DirectX11Interface(HWND hwnd, bool minimalistContext) : NullGraphicsInterface()
{
	m_bReady = false;

	// device context
	m_hwnd = hwnd;
	m_bMinimalistContext = minimalistContext;

	// d3d
	m_device = NULL;
	m_deviceContext = NULL;
	m_swapChain = NULL;
	m_frameBuffer = NULL;
	m_frameBufferDepthStencilTexture = NULL;
	m_frameBufferDepthStencilView = NULL;

	// renderer
	m_bIsFullscreen = false;
	m_bIsFullscreenBorderlessWindowed = false;
	m_vResolution = engine->getScreenSize(); // initial viewport size = window size
	m_rasterizerState = NULL;
	m_depthStencilState = NULL;
	m_blendState = NULL;
	m_shaderTexturedGeneric = NULL;
	m_vertexBuffer = NULL;
	m_iVertexBufferMaxNumVertices = 16384;
	m_iVertexBufferNumVertexOffsetCounter = 0;

	// persistent vars
	m_color = 0xffffffff;
	m_bVSync = false;
	m_activeShader = NULL;

	// stats
	m_iStatsNumDrawCalls = 0;
}

DirectX11Interface::~DirectX11Interface()
{
	if (!m_bMinimalistContext)
	{
		if (m_swapChain != NULL)
			m_swapChain->SetFullscreenState(FALSE, NULL);

		SAFE_DELETE(m_shaderTexturedGeneric);

		if (m_vertexBuffer != NULL)
			m_vertexBuffer->Release();

		if (m_rasterizerState != NULL)
			m_rasterizerState->Release();

		if (m_swapChain != NULL)
			m_swapChain->Release();

		if (m_frameBuffer != NULL)
			m_frameBuffer->Release();

		if (m_frameBufferDepthStencilView != NULL)
			m_frameBufferDepthStencilView->Release();

		if (m_frameBufferDepthStencilTexture != NULL)
			m_frameBufferDepthStencilTexture->Release();
	}

	if (m_device != NULL)
		m_device->Release();

	if (m_deviceContext != NULL)
		m_deviceContext->Release();
}

void DirectX11Interface::init()
{
	// flags
	UINT createDeviceFlags = 0;
	createDeviceFlags = D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_SINGLETHREADED;

#ifdef MCENGINE_D3D11_CREATE_DEVICE_DEBUG

	createDeviceFlags |= D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;

#endif

	// feature levels
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	const UINT numFeatureLevels = _countof(featureLevels);

	// backbuffer descriptor
	ZeroMemory(&m_swapChainModeDesc, sizeof(DXGI_MODE_DESC));
	{
		m_swapChainModeDesc.Width = (UINT)m_vResolution.x;
		m_swapChainModeDesc.Height = (UINT)m_vResolution.y;
		m_swapChainModeDesc.RefreshRate.Numerator = 0;
		m_swapChainModeDesc.RefreshRate.Denominator = 1;
		m_swapChainModeDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM; // NOTE: DXGI_FORMAT_R8G8B8A8_UNORM has the broadest compatibility range
		m_swapChainModeDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		m_swapChainModeDesc.Scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED;
	}

	// swapchain descriptor
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	{
		swapChainDesc.BufferDesc = m_swapChainModeDesc;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.OutputWindow = m_hwnd;
		swapChainDesc.Windowed = TRUE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD; // TODO: Windows 10 and newer support flip to avoid the blit, but older versions don't
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	}

	// create device + context + swapchain
	HRESULT hr;
	D3D_FEATURE_LEVEL featureLevelOut;
	if (m_bMinimalistContext)
		hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &m_device, &featureLevelOut, &m_deviceContext);
	else
		hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, &featureLevelOut, &m_deviceContext);

	if (FAILED(hr))
	{
		UString errorTitle = "DirectX Error";
		UString errorMessage = UString::format("Couldn't D3D11CreateDevice[AndSwapChain](%ld, %x, %x)!", hr, hr, MAKE_DXGI_HRESULT(hr));

		if (!m_bMinimalistContext)
		{
			errorMessage.append("\nThe engine will quit now.");
			engine->showMessageErrorFatal("DirectX Error", errorMessage);
			engine->shutdown();
		}
		else
			engine->showMessageWarning("DirectX Error", errorMessage);

		return;
	}

	if (m_bMinimalistContext)
	{
		m_bReady = true;
		return;
	}

	// disable hardcoded DirectX ALT + ENTER fullscreen toggle functionality (this is instead handled by the engine internally)
	// disable dxgi interfering with mode changes and WndProc (again, handled by the engine internally)
	{
		IDXGIFactory1 *pFactory = NULL;
		if (SUCCEEDED(m_swapChain->GetParent(__uuidof(IDXGIFactory1), (void**)&pFactory)))
		{
			pFactory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER);
			pFactory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_WINDOW_CHANGES);
			pFactory->Release();
		}
		else
			engine->showMessageWarning("DirectX Error", "Couldn't m_swapChain->GetParent()!\nThe window may behave weirdly.");
	}

	// default rasterizer settings
	{
		m_rasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
		m_rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
		m_rasterizerDesc.FrontCounterClockwise = TRUE;
		m_rasterizerDesc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
		m_rasterizerDesc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
		m_rasterizerDesc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		m_rasterizerDesc.DepthClipEnable = TRUE; // (clipping, not depth buffer!)
		m_rasterizerDesc.ScissorEnable = FALSE;
		m_rasterizerDesc.MultisampleEnable = FALSE;
		m_rasterizerDesc.AntialiasedLineEnable = FALSE;
	}
	m_device->CreateRasterizerState(&m_rasterizerDesc, &m_rasterizerState);
	m_deviceContext->RSSetState(m_rasterizerState);

	// default depthStencil settings
	{
		m_depthStencilDesc.DepthEnable = FALSE;
		m_depthStencilDesc.StencilEnable = FALSE;
		m_depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ZERO;
		m_depthStencilDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS;
		m_depthStencilDesc.StencilReadMask = 0;		// see OMSetDepthStencilState()
		m_depthStencilDesc.StencilWriteMask = 0;	// see OMSetDepthStencilState()

		// stencil front
		D3D11_DEPTH_STENCILOP_DESC depthStencilFrontFaceOpDesc;
		depthStencilFrontFaceOpDesc.StencilFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_ZERO;
		depthStencilFrontFaceOpDesc.StencilDepthFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_ZERO;
		depthStencilFrontFaceOpDesc.StencilPassOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_ZERO;
		depthStencilFrontFaceOpDesc.StencilFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;
		m_depthStencilDesc.FrontFace = depthStencilFrontFaceOpDesc;

		// stencil back
		D3D11_DEPTH_STENCILOP_DESC depthStencilBackFaceOpDesc;
		depthStencilBackFaceOpDesc.StencilFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_ZERO;
		depthStencilBackFaceOpDesc.StencilDepthFailOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_ZERO;
		depthStencilBackFaceOpDesc.StencilPassOp = D3D11_STENCIL_OP::D3D11_STENCIL_OP_ZERO;
		depthStencilBackFaceOpDesc.StencilFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;
		m_depthStencilDesc.BackFace = depthStencilBackFaceOpDesc;
	}
	m_device->CreateDepthStencilState(&m_depthStencilDesc, &m_depthStencilState);
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 0); // for 0 see StencilReadMask, StencilWriteMask

	// default blending settings
	{
		ZeroMemory(&m_blendDesc, sizeof(D3D11_BLEND_DESC));
		m_blendDesc.AlphaToCoverageEnable = FALSE;
		m_blendDesc.IndependentBlendEnable = FALSE;

		m_blendDesc.RenderTarget[0].BlendEnable = true;
		m_blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_ALL;

		m_blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
		m_blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
		m_blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;

		m_blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
		m_blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
		m_blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
	}
	m_device->CreateBlendState(&m_blendDesc, &m_blendState);
	m_deviceContext->OMSetBlendState(m_blendState, NULL, D3D11_DEFAULT_SAMPLE_MASK);

	// create default shader
	UString vertexShader =
		"###DirectX11Interface::VertexShader#############################################################################################\n"
		"\n"
		"##D3D11_INPUT_ELEMENT_DESC::VS_INPUT::POSITION::DXGI_FORMAT_R32G32B32_FLOAT::D3D11_INPUT_PER_VERTEX_DATA\n"
		"##D3D11_INPUT_ELEMENT_DESC::VS_INPUT::COLOR0::DXGI_FORMAT_R32G32B32A32_FLOAT::D3D11_INPUT_PER_VERTEX_DATA\n"
		"##D3D11_INPUT_ELEMENT_DESC::VS_INPUT::TEXCOORD0::DXGI_FORMAT_R32G32_FLOAT::D3D11_INPUT_PER_VERTEX_DATA\n"
		"\n"
		"##D3D11_BUFFER_DESC::D3D11_BIND_CONSTANT_BUFFER::ModelViewProjectionConstantBuffer::mvp::float4x4\n"
		//"##D3D11_BUFFER_DESC::D3D11_BIND_CONSTANT_BUFFER::ModelViewProjectionConstantBuffer::world::float4x4\n"
		//"##D3D11_BUFFER_DESC::D3D11_BIND_CONSTANT_BUFFER::ModelViewProjectionConstantBuffer::view::float4x4\n"
		//"##D3D11_BUFFER_DESC::D3D11_BIND_CONSTANT_BUFFER::ModelViewProjectionConstantBuffer::projection::float4x4\n"
		"##D3D11_BUFFER_DESC::D3D11_BIND_CONSTANT_BUFFER::ModelViewProjectionConstantBuffer::col::float4\n"
		"##D3D11_BUFFER_DESC::D3D11_BIND_CONSTANT_BUFFER::ModelViewProjectionConstantBuffer::misc::float4\n"
		"\n"
		"cbuffer ModelViewProjectionConstantBuffer : register(b0)\n"
		"{\n"
		"	float4x4 mvp;		// world matrix for object\n"
		//"	matrix world;		// world matrix for object\n"
		//"	matrix view;		// view matrix\n"
		//"	matrix projection;	// projection matrix\n"
		"	float4 col;			// global color\n"
		"	float4 misc;		// misc params. [0] = textured or flat, [1] = vertex colors\n"
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
		"	float4 misc	: COLOR1;\n"
		"	float2 tex	: TEXCOORD0;\n"
		"};\n"
		"\n"
		"VS_OUTPUT vsmain(in VS_INPUT In)\n"
		"{\n"
		"	VS_OUTPUT Out;\n"
		"	In.pos.w = 1.0f;\n"
		"	Out.pos = mul(In.pos, mvp);\n"
		//"	Out.pos.z = (Out.pos.z + Out.pos.w)/2.0f;\n" // TODO: not sure if necessary to compensate clip space range here, no artifacts so far (OpenGL NDC z from -1 to 1, DirectX NDC z from 0 to 1)
		"	Out.col = In.col;\n"
		"	Out.misc = misc;\n"
		"	Out.tex = In.tex;\n"
		"	return Out;\n"
		"}\n";

	UString pixelShader =
		"###DirectX11Interface::PixelShader##############################################################################################\n"
		"\n"
		"Texture2D tex2D;\n"
		"SamplerState samplerState\n"
		"{\n"
		"	Filter = MIN_MAG_MIP_LINEAR;\n"
		"	AddressU = Clamp;\n"
		"	AddressV = Clamp;\n"
		"};\n"
		"\n"
		"struct PS_INPUT\n"
		"{\n"
		"	float4 pos	: SV_Position;\n"
		"	float4 col	: COLOR0;\n"
		"	float4 misc	: COLOR1;\n"
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
		"	PS_OUTPUT Out;\n"
			"if (In.misc.x < 0.5f)\n"
			"{\n"
			"	Out.col = In.col;\n"
			"}\n"
			"else\n"
			"{\n"
			"	Out.col = tex2D.Sample(samplerState, In.tex) * In.col;\n"
			"}\n"
		"	return Out;\n"
		"}\n";

	m_shaderTexturedGeneric = (DirectX11Shader*)createShaderFromSource(vertexShader, pixelShader);
	m_shaderTexturedGeneric->load();

	if (!m_shaderTexturedGeneric->isReady())
	{
		engine->showMessageErrorFatal("DirectX Error", "Failed to create default shader!\nThe engine will quit now.");
		engine->shutdown();
		return;
	}

	// default vertexbuffer
	{
		m_vertexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
		m_vertexBufferDesc.ByteWidth = sizeof(SimpleVertex) * m_iVertexBufferMaxNumVertices;
		m_vertexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
		m_vertexBufferDesc.CPUAccessFlags = (m_vertexBufferDesc.Usage == D3D11_USAGE::D3D11_USAGE_DYNAMIC ? D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE : 0);
		m_vertexBufferDesc.MiscFlags = 0;
		m_vertexBufferDesc.StructureByteStride = 0;
	}
	if (FAILED(m_device->CreateBuffer(&m_vertexBufferDesc, NULL, &m_vertexBuffer)))
	{
		engine->showMessageErrorFatal("DirectX Error", "Failed to create default vertex buffer!\nThe engine will quit now.");
		engine->shutdown();
		return;
	}

	onResolutionChange(m_vResolution); // NOTE: force build swapchain rendertarget view

	m_bReady = true;
}

void DirectX11Interface::beginScene()
{
	Matrix4 defaultProjectionMatrix = Camera::buildMatrixOrtho2DDXLH(0, m_vResolution.x, m_vResolution.y, 0, -1.0f, 1.0f);

	// push main transforms
	pushTransform();
	setProjectionMatrix(defaultProjectionMatrix);
	translate(r_globaloffset_x->getFloat(), r_globaloffset_y->getFloat());

	// and apply them
	updateTransform();

	// clear
	D3DXCOLOR clearColor(0.0f, 0.0f, 0.0f, 0.0f);
	if (m_frameBuffer != NULL)
		m_deviceContext->ClearRenderTargetView(m_frameBuffer, clearColor);
	if (m_frameBufferDepthStencilView != NULL)
		m_deviceContext->ClearDepthStencilView(m_frameBufferDepthStencilView, D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH | D3D11_CLEAR_FLAG::D3D11_CLEAR_STENCIL, 1.0f, 0); // yes, the 1.0f is correct

	// enable default shader
	m_shaderTexturedGeneric->enable();

	// prev frame render stats
	const int numDrawCallsPrevFrame = m_iStatsNumDrawCalls;
	m_iStatsNumDrawCalls = 0;
	if (vprof != NULL && vprof->isEnabled())
	{
		int numActiveShaders = 1;
		for (const Resource *resource : engine->getResourceManager()->getResources())
		{
			const DirectX11Shader *dx11Shader = dynamic_cast<const DirectX11Shader*>(resource);
			if (dx11Shader != NULL)
			{
				if (dx11Shader->getStatsNumConstantBufferUploadsPerFrameEngineFrameCount() == (engine->getFrameCount() - 1))
					numActiveShaders++;
			}
		}

		int shaderCounter = 0;
		vprof->addInfoBladeEngineTextLine(UString::format("Draw Calls: %i", numDrawCallsPrevFrame));
		vprof->addInfoBladeEngineTextLine(UString::format("Active Shaders: %i", numActiveShaders));
		vprof->addInfoBladeEngineTextLine(UString::format("shader[%i]: shaderTexturedGeneric: %ic", shaderCounter++, (int)m_shaderTexturedGeneric->getStatsNumConstantBufferUploadsPerFrame()));
		for (const Resource *resource : engine->getResourceManager()->getResources())
		{
			const DirectX11Shader *dx11Shader = dynamic_cast<const DirectX11Shader*>(resource);
			if (dx11Shader != NULL)
			{
				if (dx11Shader->getStatsNumConstantBufferUploadsPerFrameEngineFrameCount() == (engine->getFrameCount() - 1))
					vprof->addInfoBladeEngineTextLine(UString::format("shader[%i]: %s: %ic", shaderCounter++, resource->getName().toUtf8(), (int)dx11Shader->getStatsNumConstantBufferUploadsPerFrame()));
			}
		}
	}
}

void DirectX11Interface::endScene()
{
	popTransform();

	checkStackLeaks();

	if (m_clipRectStack.size() > 0)
	{
		engine->showMessageErrorFatal("ClipRect Stack Leak", "Make sure all push*() have a pop*()!");
		engine->shutdown();
	}

	m_swapChain->Present(m_bVSync ? 1 : 0, 0);

	// aka checkErrors()
#ifdef MCENGINE_D3D11_CREATE_DEVICE_DEBUG
	{
		ID3D11InfoQueue *debugInfoQueue;
		if (SUCCEEDED(m_device->QueryInterface(__uuidof(ID3D11InfoQueue), (void **)&debugInfoQueue)))
		{
			UINT64 message_count = debugInfoQueue->GetNumStoredMessages();

			for (UINT64 i=0; i<message_count; i++)
			{
				SIZE_T message_size = 0;
				debugInfoQueue->GetMessage(i, NULL, &message_size);

				D3D11_MESSAGE *message = (D3D11_MESSAGE*)malloc(message_size + 1);
				memset((void*)message, '\0', message_size + 1);
				if (SUCCEEDED(debugInfoQueue->GetMessage(i, message, &message_size)))
					debugLog("DirectX11Debug: %s\n", message->pDescription);
				else
					debugLog("DirectX Error: Couldn't debugInfoQueue->GetMessage()\n");

				free(message);
			}

			debugInfoQueue->ClearStoredMessages();
		}
		else
			debugLog("DirectX Error: Couldn't m_device->QueryInterface( ID3D11InfoQueue )\n");
	}
#endif
}

void DirectX11Interface::clearDepthBuffer()
{
	if (m_frameBufferDepthStencilView != NULL)
		m_deviceContext->ClearDepthStencilView(m_frameBufferDepthStencilView, D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH, 1.0f, 0); // yes, the 1.0f is correct
}

void DirectX11Interface::setColor(Color color)
{
	if (m_color == color) return;

	m_color = color;
	m_shaderTexturedGeneric->setUniform4f("col", COLOR_GET_Af(m_color), COLOR_GET_Rf(m_color), COLOR_GET_Gf(m_color), COLOR_GET_Bf(m_color));
}

void DirectX11Interface::setAlpha(float alpha)
{
	m_color &= 0x00ffffff;
	m_color |= ((int)(255.0f * alpha)) << 24;

	setColor(m_color);
}

void DirectX11Interface::drawPixel(int x, int y)
{
	updateTransform();

	m_shaderTexturedGeneric->setUniform1f("misc", 0.0f); // disable texturing

	// build directx vertices
	m_vertices.clear();
	{
		SimpleVertex v;
		{
			v.pos.x = x;
			v.pos.y = y;
			v.pos.z = 0.0f;

			v.col = Vector4(COLOR_GET_Rf(m_color), COLOR_GET_Gf(m_color), COLOR_GET_Bf(m_color), COLOR_GET_Af(m_color));

			v.tex.x = 0.0f;
			v.tex.y = 0.0f;
		}
		m_vertices.push_back(v);
	}

	// upload everything to gpu
	size_t numVertexOffset = 0;
	bool uploadedSuccessfully = true;
	{
		if (m_vertexBufferDesc.Usage == D3D11_USAGE::D3D11_USAGE_DEFAULT)
		{
			D3D11_BOX box;
			{
				box.left = sizeof(DirectX11Interface::SimpleVertex) * 0;
				box.right = box.left + (sizeof(DirectX11Interface::SimpleVertex) * m_vertices.size());
				box.top = 0;
				box.bottom = 1;
				box.front = 0;
				box.back = 1;
			}
			m_deviceContext->UpdateSubresource(m_vertexBuffer, 0, &box, &m_vertices[0], 0, 0);
		}
		else
		{
			const bool needsDiscardEntireBuffer = (m_iVertexBufferNumVertexOffsetCounter + m_vertices.size() > m_iVertexBufferMaxNumVertices);
			const size_t writeOffsetNumVertices = (needsDiscardEntireBuffer ? 0 : m_iVertexBufferNumVertexOffsetCounter);
			numVertexOffset = writeOffsetNumVertices;
			{
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
				if (SUCCEEDED(m_deviceContext->Map(m_vertexBuffer, 0, (needsDiscardEntireBuffer ? D3D11_MAP::D3D11_MAP_WRITE_DISCARD : D3D11_MAP::D3D11_MAP_WRITE_NO_OVERWRITE), 0, &mappedResource)))
				{
					memcpy((void*)(((SimpleVertex*)mappedResource.pData) + writeOffsetNumVertices), &m_vertices[0], sizeof(DirectX11Interface::SimpleVertex) * m_vertices.size());
					m_deviceContext->Unmap(m_vertexBuffer, 0);
				}
				else
					uploadedSuccessfully = false;
			}
			m_iVertexBufferNumVertexOffsetCounter = writeOffsetNumVertices + m_vertices.size();
		}

		// shader update
		if (uploadedSuccessfully)
		{
			if (m_activeShader != NULL)
				m_activeShader->onJustBeforeDraw();
		}
	}

	// draw it
	if (uploadedSuccessfully)
	{
		const UINT stride = sizeof(SimpleVertex);
		const UINT offset = 0;

		m_deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
		m_deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		m_deviceContext->Draw(m_vertices.size(), numVertexOffset);
		m_iStatsNumDrawCalls++;
	}
}

void DirectX11Interface::drawPixels(int x, int y, int width, int height, Graphics::DRAWPIXELS_TYPE type, const void *pixels)
{
	// TODO: implement
}

void DirectX11Interface::drawLine(int x1, int y1, int x2, int y2)
{
	updateTransform();

	m_shaderTexturedGeneric->setUniform1f("misc", 0.0f); // disable texturing

	static VertexArrayObject vao(Graphics::PRIMITIVE::PRIMITIVE_LINES);
	{
		vao.empty();

		vao.addVertex(x1 + 0.5f, y1 + 0.5f);
		vao.addVertex(x2 + 0.5f, y2 + 0.5f);
	}
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

	m_shaderTexturedGeneric->setUniform1f("misc", 0.0f); // disable texturing

	static VertexArrayObject vao(Graphics::PRIMITIVE::PRIMITIVE_QUADS);
	{
		vao.empty();

		vao.addVertex(x, y);
		vao.addVertex(x, y + height);
		vao.addVertex(x + width, y + height);
		vao.addVertex(x + width, y);
	}
	drawVAO(&vao);
}

void DirectX11Interface::fillGradient(int x, int y, int width, int height, Color topLeftColor, Color topRightColor, Color bottomLeftColor, Color bottomRightColor)
{
	updateTransform();

	m_shaderTexturedGeneric->setUniform1f("misc", 0.0f); // disable texturing

	static VertexArrayObject vao(Graphics::PRIMITIVE::PRIMITIVE_QUADS);
	{
		vao.empty();

		vao.addVertex(x, y);
		vao.addColor(topLeftColor);
		vao.addVertex(x + width, y);
		vao.addColor(topRightColor);
		vao.addVertex(x + width, y + height);
		vao.addColor(bottomRightColor);
		vao.addVertex(x, y + height);
		vao.addColor(bottomLeftColor);
	}
	drawVAO(&vao);
}

void DirectX11Interface::drawQuad(int x, int y, int width, int height)
{
	updateTransform();

	m_shaderTexturedGeneric->setUniform1f("misc", 1.0f); // enable texturing

	static VertexArrayObject vao(Graphics::PRIMITIVE::PRIMITIVE_QUADS);
	{
		vao.empty();

		vao.addVertex(x, y);
		vao.addTexcoord(0, 0);
		vao.addVertex(x, y + height);
		vao.addTexcoord(0, 1);
		vao.addVertex(x + width, y + height);
		vao.addTexcoord(1, 1);
		vao.addVertex(x + width, y);
		vao.addTexcoord(1, 0);
	}
	drawVAO(&vao);
}

void DirectX11Interface::drawQuad(Vector2 topLeft, Vector2 topRight, Vector2 bottomRight, Vector2 bottomLeft, Color topLeftColor, Color topRightColor, Color bottomRightColor, Color bottomLeftColor)
{
	updateTransform();

	m_shaderTexturedGeneric->setUniform1f("misc", 0.0f); // disable texturing

	static VertexArrayObject vao(Graphics::PRIMITIVE::PRIMITIVE_QUADS);
	{
		vao.empty();

		vao.addVertex(topLeft.x, topLeft.y);
		vao.addColor(topLeftColor);
		//vao.addTexcoord(0, 0);
		vao.addVertex(bottomLeft.x, bottomLeft.y);
		vao.addColor(bottomLeftColor);
		//vao.addTexcoord(0, 1);
		vao.addVertex(bottomRight.x, bottomRight.y);
		vao.addColor(bottomRightColor);
		//vao.addTexcoord(1, 1);
		vao.addVertex(topRight.x, topRight.y);
		vao.addColor(topRightColor);
		//vao.addTexcoord(1, 0);
	}
	drawVAO(&vao);
}

void DirectX11Interface::drawImage(Image *image)
{
	if (image == NULL)
	{
		debugLog("WARNING: Tried to draw image with NULL texture!\n");
		return;
	}
	if (!image->isReady()) return;

	updateTransform();

	m_shaderTexturedGeneric->setUniform1f("misc", 1.0f); // enable texturing

	float width = image->getWidth();
	float height = image->getHeight();

	float x = -width/2;
	float y = -height/2;

	static VertexArrayObject vao(Graphics::PRIMITIVE::PRIMITIVE_QUADS);
	{
		vao.empty();

		vao.addVertex(x, y);
		vao.addTexcoord(0, 0);
		vao.addVertex(x, y + height);
		vao.addTexcoord(0, 1);
		vao.addVertex(x + width, y + height);
		vao.addTexcoord(1, 1);
		vao.addVertex(x + width, y);
		vao.addTexcoord(1, 0);
	}

	image->bind();
	{
		drawVAO(&vao);
	}
	image->unbind();

	if (r_debug_drawimage->getBool())
	{
		setColor(0xbbff00ff);
		drawRect(x, y, width, height);
	}
}

void DirectX11Interface::drawString(McFont *font, UString text)
{
	if (font == NULL || text.length() < 1 || !font->isReady()) return;

	updateTransform();

	m_shaderTexturedGeneric->setUniform1f("misc", 1.0f); // enable texturing

	font->drawString(this, text);
}

void DirectX11Interface::drawVAO(VertexArrayObject *vao)
{
	if (vao == NULL) return;

	updateTransform();

	// if baked, then we can directly draw the buffer
	if (vao->isReady())
	{
		// shader update
		{
			if (m_activeShader != NULL)
				m_activeShader->onJustBeforeDraw();
		}

		((DirectX11VertexArrayObject*)vao)->draw();
		return;
	}

	const std::vector<Vector3> &vertices = vao->getVertices();
	///const std::vector<Vector3> &normals = vao->getNormals();
	const std::vector<std::vector<Vector2>> &texcoords = vao->getTexcoords();
	const std::vector<Color> &vcolors = vao->getColors();

	if (vertices.size() < 2) return;

	// TODO: optimize this piece of shit

	// no support for quads, because fuck you
	// no support for triangle fans, because fuck youuu
	// rewrite all quads into triangles
	// rewrite all triangle fans into triangles
	static std::vector<Vector3> finalVertices;
	finalVertices = vertices;
	static std::vector<std::vector<Vector2>> finalTexcoords;
	finalTexcoords = texcoords;
	static std::vector<Vector4> colors;
	colors.clear();
	static std::vector<Vector4> finalColors;
	finalColors.clear();

	for (size_t i=0; i<vcolors.size(); i++)
	{
		const Vector4 color = Vector4(COLOR_GET_Rf(vcolors[i]), COLOR_GET_Gf(vcolors[i]), COLOR_GET_Bf(vcolors[i]), COLOR_GET_Af(vcolors[i]));
		colors.push_back(color);
		finalColors.push_back(color);
	}
	const size_t maxColorIndex = (colors.size() > 0 ? colors.size() - 1 : 0);

	Graphics::PRIMITIVE primitive = vao->getPrimitive();
	if (primitive == Graphics::PRIMITIVE::PRIMITIVE_QUADS)
	{
		finalVertices.clear();
		for (size_t t=0; t<finalTexcoords.size(); t++)
		{
			finalTexcoords[t].clear();
		}
		finalColors.clear();
		primitive = Graphics::PRIMITIVE::PRIMITIVE_TRIANGLES;

		if (vertices.size() > 3)
		{
			for (size_t i=0; i<vertices.size(); i+=4)
			{
				finalVertices.push_back(vertices[i + 0]);
				finalVertices.push_back(vertices[i + 1]);
				finalVertices.push_back(vertices[i + 2]);

				for (size_t t=0; t<texcoords.size(); t++)
				{
					finalTexcoords[t].push_back(texcoords[t][i + 0]);
					finalTexcoords[t].push_back(texcoords[t][i + 1]);
					finalTexcoords[t].push_back(texcoords[t][i + 2]);
				}

				if (colors.size() > 0)
				{
					finalColors.push_back(colors[clamp<size_t>(i + 0, 0, maxColorIndex)]);
					finalColors.push_back(colors[clamp<size_t>(i + 1, 0, maxColorIndex)]);
					finalColors.push_back(colors[clamp<size_t>(i + 2, 0, maxColorIndex)]);
				}

				finalVertices.push_back(vertices[i + 0]);
				finalVertices.push_back(vertices[i + 2]);
				finalVertices.push_back(vertices[i + 3]);

				for (size_t t=0; t<texcoords.size(); t++)
				{
					finalTexcoords[t].push_back(texcoords[t][i + 0]);
					finalTexcoords[t].push_back(texcoords[t][i + 2]);
					finalTexcoords[t].push_back(texcoords[t][i + 3]);
				}

				if (colors.size() > 0)
				{
					finalColors.push_back(colors[clamp<size_t>(i + 0, 0, maxColorIndex)]);
					finalColors.push_back(colors[clamp<size_t>(i + 2, 0, maxColorIndex)]);
					finalColors.push_back(colors[clamp<size_t>(i + 3, 0, maxColorIndex)]);
				}
			}
		}
	}
	else if (primitive == Graphics::PRIMITIVE::PRIMITIVE_TRIANGLE_FAN)
	{
		finalVertices.clear();
		for (size_t t=0; t<finalTexcoords.size(); t++)
		{
			finalTexcoords[t].clear();
		}
		finalColors.clear();
		primitive = Graphics::PRIMITIVE::PRIMITIVE_TRIANGLES;

		if (vertices.size() > 2)
		{
			for (size_t i=2; i<vertices.size(); i++)
			{
				finalVertices.push_back(vertices[0]);

				finalVertices.push_back(vertices[i]);
				finalVertices.push_back(vertices[i - 1]);

				for (size_t t=0; t<texcoords.size(); t++)
				{
					finalTexcoords[t].push_back(texcoords[t][0]);
					finalTexcoords[t].push_back(texcoords[t][i]);
					finalTexcoords[t].push_back(texcoords[t][i - 1]);
				}

				if (colors.size() > 0)
				{
					finalColors.push_back(colors[clamp<size_t>(0, 0, maxColorIndex)]);
					finalColors.push_back(colors[clamp<size_t>(i, 0, maxColorIndex)]);
					finalColors.push_back(colors[clamp<size_t>(i - 1, 0, maxColorIndex)]);
				}
			}
		}
	}

	// build directx vertices
	const bool hasTexcoords0 = (finalTexcoords.size() > 0 && finalTexcoords[0].size() > 0);
	m_vertices.resize(finalVertices.size());
	{
		const bool hasColors = (finalColors.size() > 0);

		const size_t maxColorIndex = (hasColors ? finalColors.size() - 1 : 0);
		const size_t maxTexcoords0Index = (hasTexcoords0 ? finalTexcoords[0].size() - 1 : 0);

		const Vector4 color = Vector4(COLOR_GET_Rf(m_color), COLOR_GET_Gf(m_color), COLOR_GET_Bf(m_color), COLOR_GET_Af(m_color));

		for (size_t i=0; i<finalVertices.size(); i++)
		{
			m_vertices[i].pos = finalVertices[i];

			if (hasColors)
				m_vertices[i].col = finalColors[clamp<size_t>(i, 0, maxColorIndex)];
			else
				m_vertices[i].col = color;

			// TODO: multitexturing
			if (hasTexcoords0)
				m_vertices[i].tex = finalTexcoords[0][clamp<size_t>(i, 0, maxTexcoords0Index)];
		}
	}

	// upload everything to gpu
	size_t numVertexOffset = 0;
	bool uploadedSuccessfully = true;
	{
		if (m_vertexBufferDesc.Usage == D3D11_USAGE::D3D11_USAGE_DEFAULT)
		{
			D3D11_BOX box;
			{
				box.left = sizeof(DirectX11Interface::SimpleVertex) * 0;
				box.right = box.left + (sizeof(DirectX11Interface::SimpleVertex) * m_vertices.size());
				box.top = 0;
				box.bottom = 1;
				box.front = 0;
				box.back = 1;
			}
			m_deviceContext->UpdateSubresource(m_vertexBuffer, 0, &box, &m_vertices[0], 0, 0);
		}
		else
		{
			const bool needsDiscardEntireBuffer = (m_iVertexBufferNumVertexOffsetCounter + m_vertices.size() > m_iVertexBufferMaxNumVertices);
			const size_t writeOffsetNumVertices = (needsDiscardEntireBuffer ? 0 : m_iVertexBufferNumVertexOffsetCounter);
			numVertexOffset = writeOffsetNumVertices;
			{
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
				if (SUCCEEDED(m_deviceContext->Map(m_vertexBuffer, 0, (needsDiscardEntireBuffer ? D3D11_MAP::D3D11_MAP_WRITE_DISCARD : D3D11_MAP::D3D11_MAP_WRITE_NO_OVERWRITE), 0, &mappedResource)))
				{
					memcpy((void*)(((SimpleVertex*)mappedResource.pData) + writeOffsetNumVertices), &m_vertices[0], sizeof(DirectX11Interface::SimpleVertex) * m_vertices.size());
					m_deviceContext->Unmap(m_vertexBuffer, 0);
				}
				else
					uploadedSuccessfully = false;
			}
			m_iVertexBufferNumVertexOffsetCounter = writeOffsetNumVertices + m_vertices.size();
		}

		// shader update
		if (uploadedSuccessfully)
		{
			m_shaderTexturedGeneric->setUniform1f("misc", (hasTexcoords0 ? 1.0f : 0.0f));

			if (m_activeShader != NULL)
				m_activeShader->onJustBeforeDraw();
		}
	}

	// draw it
	if (uploadedSuccessfully)
	{
		const UINT stride = sizeof(SimpleVertex);
		const UINT offset = 0;

		m_deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
		m_deviceContext->IASetPrimitiveTopology((D3D_PRIMITIVE_TOPOLOGY)primitiveToDirectX(primitive));
		m_deviceContext->Draw(m_vertices.size(), numVertexOffset);
		m_iStatsNumDrawCalls++;
	}
}

void DirectX11Interface::setClipRect(McRect clipRect)
{
	if (r_debug_disable_cliprect->getBool()) return;
	//if (m_bIs3DScene) return; // HACKHACK: TODO:

	setClipping(true);

	D3D11_RECT rect;
	{
		rect.left = clipRect.getMinX();
		rect.top = clipRect.getMinY() - 1;
		rect.right = clipRect.getMaxX();
		rect.bottom = clipRect.getMaxY() - 1;
	}
	m_deviceContext->RSSetScissorRects(1, &rect);
}

void DirectX11Interface::pushClipRect(McRect clipRect)
{
	if (m_clipRectStack.size() > 0)
		m_clipRectStack.push(m_clipRectStack.top().intersect(clipRect));
	else
		m_clipRectStack.push(clipRect);

	setClipRect(m_clipRectStack.top());
}

void DirectX11Interface::popClipRect()
{
	m_clipRectStack.pop();

	if (m_clipRectStack.size() > 0)
		setClipRect(m_clipRectStack.top());
	else
		setClipping(false);
}

void DirectX11Interface::setClipping(bool enabled)
{
	if (enabled)
	{
		if (m_clipRectStack.size() < 1)
			enabled = false;
	}

	m_rasterizerState->Release();
	m_rasterizerDesc.ScissorEnable = (enabled ? TRUE : FALSE);
	m_device->CreateRasterizerState(&m_rasterizerDesc, &m_rasterizerState);
	m_deviceContext->RSSetState(m_rasterizerState);
}

void DirectX11Interface::setAlphaTesting(bool enabled)
{
	// TODO: implement in default shader
}

void DirectX11Interface::setAlphaTestFunc(COMPARE_FUNC alphaFunc, float ref)
{
	// TODO: implement in default shader
}

void DirectX11Interface::setBlending(bool enabled)
{
	m_blendState->Release();
	m_blendDesc.RenderTarget[0].BlendEnable = (enabled ? TRUE : FALSE);
	m_device->CreateBlendState(&m_blendDesc, &m_blendState);
	m_deviceContext->OMSetBlendState(m_blendState, NULL, D3D11_DEFAULT_SAMPLE_MASK);
}

void DirectX11Interface::setBlendMode(BLEND_MODE blendMode)
{
	m_blendState->Release();
	{
		switch (blendMode)
		{
		case BLEND_MODE::BLEND_MODE_ALPHA:
			{
				m_blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
				m_blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
				m_blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;

				m_blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
				m_blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
				m_blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
			}
			break;

		case BLEND_MODE::BLEND_MODE_ADDITIVE:
			{
				m_blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
				m_blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND::D3D11_BLEND_ONE;
				m_blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;

				m_blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
				m_blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
				m_blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
			}
			break;

		case BLEND_MODE::BLEND_MODE_PREMUL_ALPHA:
			{
				m_blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
				m_blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
				m_blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;

				m_blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
				m_blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
				m_blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
			}
			break;

		case BLEND_MODE::BLEND_MODE_PREMUL_COLOR:
			{
				m_blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND::D3D11_BLEND_ONE;
				m_blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
				m_blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;

				m_blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
				m_blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
				m_blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
			}
			break;
		}
	}
	m_device->CreateBlendState(&m_blendDesc, &m_blendState);
	m_deviceContext->OMSetBlendState(m_blendState, NULL, D3D11_DEFAULT_SAMPLE_MASK);
}

void DirectX11Interface::setDepthBuffer(bool enabled)
{
	m_depthStencilState->Release();
	m_depthStencilDesc.DepthEnable = (enabled ? TRUE : FALSE);
	m_depthStencilDesc.DepthWriteMask = (enabled ? D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ZERO);
	m_device->CreateDepthStencilState(&m_depthStencilDesc, &m_depthStencilState);
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 0); // for 0 see StencilReadMask, StencilWriteMask
}

void DirectX11Interface::setCulling(bool culling)
{
	m_rasterizerState->Release();
	m_rasterizerDesc.CullMode = (culling ? D3D11_CULL_MODE::D3D11_CULL_BACK : D3D11_CULL_MODE::D3D11_CULL_NONE);
	m_device->CreateRasterizerState(&m_rasterizerDesc, &m_rasterizerState);
	m_deviceContext->RSSetState(m_rasterizerState);
}

void DirectX11Interface::setAntialiasing(bool aa)
{
	m_rasterizerState->Release();
	m_rasterizerDesc.MultisampleEnable = (aa ? TRUE : FALSE);
	m_device->CreateRasterizerState(&m_rasterizerDesc, &m_rasterizerState);
	m_deviceContext->RSSetState(m_rasterizerState);
}

void DirectX11Interface::setWireframe(bool enabled)
{
	m_rasterizerState->Release();
	m_rasterizerDesc.FillMode = (enabled ? D3D11_FILL_MODE::D3D11_FILL_WIREFRAME : D3D11_FILL_MODE::D3D11_FILL_SOLID);
	m_device->CreateRasterizerState(&m_rasterizerDesc, &m_rasterizerState);
	m_deviceContext->RSSetState(m_rasterizerState);
}

void DirectX11Interface::flush()
{
	m_deviceContext->Flush();
}

std::vector<unsigned char> DirectX11Interface::getScreenshot()
{
	std::vector<unsigned char> result;
	{
		bool success = false;
		{
			ID3D11Texture2D *backBuffer = NULL;
			if (SUCCEEDED(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer)) && backBuffer != NULL)
			{
				D3D11_TEXTURE2D_DESC backBufferDesc;
				backBuffer->GetDesc(&backBufferDesc);
				{
					backBufferDesc.Usage = D3D11_USAGE_STAGING;
					backBufferDesc.BindFlags = 0;
					backBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
				}
				ID3D11Texture2D *tempTexture2D = NULL;
				if (SUCCEEDED(m_device->CreateTexture2D(&backBufferDesc, NULL, &tempTexture2D)) && tempTexture2D != NULL)
				{
					D3D11_TEXTURE2D_DESC tempTexture2DDesc;
					tempTexture2D->GetDesc(&tempTexture2DDesc);
					m_deviceContext->CopyResource(tempTexture2D, backBuffer);
					{
						D3D11_MAPPED_SUBRESOURCE mappedResource;
						ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
						if (SUCCEEDED(m_deviceContext->Map(tempTexture2D, 0, D3D11_MAP_READ, 0, &mappedResource)))
						{
							success = true;
							result.reserve(tempTexture2DDesc.Width * tempTexture2DDesc.Height * 3); // RGB
							{
								const UINT numPixelBytes = 4; // RGBA
								const UINT numRowBytes = mappedResource.RowPitch / sizeof(unsigned char);
								for (UINT y=0; y<tempTexture2DDesc.Height; y++)
								{
									for (UINT x=0; x<tempTexture2DDesc.Width; x++)
									{
										unsigned char r = (unsigned char)(((unsigned char*)mappedResource.pData)[y*numRowBytes + x*numPixelBytes + 0]); // RGBA
										unsigned char g = (unsigned char)(((unsigned char*)mappedResource.pData)[y*numRowBytes + x*numPixelBytes + 1]);
										unsigned char b = (unsigned char)(((unsigned char*)mappedResource.pData)[y*numRowBytes + x*numPixelBytes + 2]);
										//unsigned char a = (unsigned char)(((unsigned char*)mappedResource.pData)[y*numRowBytes + x*numPixelBytes + 3]);

										result.push_back(r);
										result.push_back(g);
										result.push_back(b);
									}
								}
							}
							m_deviceContext->Unmap(tempTexture2D, 0);
						}
					}
					tempTexture2D->Release();
				}
				backBuffer->Release();
			}
		}

		if (!success)
		{
			const int numExpectedPixels = (int)(m_vResolution.x) * (int)(m_vResolution.y);
			for (int i=0; i<numExpectedPixels; i++)
			{
				result.push_back(0);
				result.push_back(0);
				result.push_back(0);
			}
		}
	}
	return result;
}

UString DirectX11Interface::getVendor()
{
	IDXGIFactory1 *pFactory = NULL;
	if (SUCCEEDED(m_swapChain->GetParent(__uuidof(IDXGIFactory1), (void**)&pFactory)) && pFactory != NULL)
	{
		IDXGIAdapter *adapter = NULL;
		if (SUCCEEDED(pFactory->EnumAdapters(0, &adapter)) && adapter != NULL)
		{
			DXGI_ADAPTER_DESC desc;
			if (SUCCEEDED(adapter->GetDesc(&desc)))
			{
				return UString::format("0x%x", desc.VendorId);
			}
			adapter->Release();
		}
		pFactory->Release();
	}

	return "<UNKNOWN>";
}

UString DirectX11Interface::getModel()
{
	IDXGIFactory1 *pFactory = NULL;
	if (SUCCEEDED(m_swapChain->GetParent(__uuidof(IDXGIFactory1), (void**)&pFactory)) && pFactory != NULL)
	{
		IDXGIAdapter *adapter = NULL;
		if (SUCCEEDED(pFactory->EnumAdapters(0, &adapter)) && adapter != NULL)
		{
			DXGI_ADAPTER_DESC desc;
			if (SUCCEEDED(adapter->GetDesc(&desc)))
			{
				const std::wstring description = std::wstring(desc.Description, 128);
				return UString(description.c_str());
			}
			adapter->Release();
		}
		pFactory->Release();
	}

	return "<UNKNOWN>";
}

UString DirectX11Interface::getVersion()
{
	IDXGIFactory1 *pFactory = NULL;
	if (SUCCEEDED(m_swapChain->GetParent(__uuidof(IDXGIFactory1), (void**)&pFactory)) && pFactory != NULL)
	{
		IDXGIAdapter *adapter = NULL;
		if (SUCCEEDED(pFactory->EnumAdapters(0, &adapter)) && adapter != NULL)
		{
			DXGI_ADAPTER_DESC desc;
			if (SUCCEEDED(adapter->GetDesc(&desc)))
			{
				return UString::format("0x%x/%x/%x", desc.DeviceId, desc.SubSysId, desc.Revision);
			}
			adapter->Release();
		}
		pFactory->Release();
	}

	return "<UNKNOWN>";
}

int DirectX11Interface::getVRAMTotal()
{
	IDXGIFactory1 *pFactory = NULL;
	if (SUCCEEDED(m_swapChain->GetParent(__uuidof(IDXGIFactory1), (void**)&pFactory)) && pFactory != NULL)
	{
		IDXGIAdapter *adapter = NULL;
		if (SUCCEEDED(pFactory->EnumAdapters(0, &adapter)) && adapter != NULL)
		{
			DXGI_ADAPTER_DESC desc;
			if (SUCCEEDED(adapter->GetDesc(&desc)))
			{
				// NOTE: this value is affected by 32-bit limits, meaning it will cap out at ~3071 MB (or ~3072 MB depending on rounding), which makes sense since we can't address more video memory in a 32-bit process anyway
				return (desc.DedicatedVideoMemory / 1024); // (from bytes to kb)
			}
			adapter->Release();
		}
		pFactory->Release();
	}

	return -1;
}

int DirectX11Interface::getVRAMRemaining()
{
	// TODO: https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_4/nf-dxgi1_4-idxgiadapter3-queryvideomemoryinfo

	return -1;
}

void DirectX11Interface::setVSync(bool vsync)
{
	m_bVSync = vsync;
}

void DirectX11Interface::onResolutionChange(Vector2 newResolution)
{
	m_vResolution = newResolution;

	if (!engine->isDrawing()) // HACKHACK: to allow viewport changes for rendertarget rendering OpenGL style
	{
		// rebuild swapchain rendertarget + view
		HRESULT hr;

		// unset + release
		{
			m_deviceContext->OMSetRenderTargets(0, NULL, NULL);

			if (m_frameBuffer != NULL)
			{
				m_frameBuffer->Release();
				m_frameBuffer = NULL;
			}

			if (m_frameBufferDepthStencilView != NULL)
			{
				m_frameBufferDepthStencilView->Release();
				m_frameBufferDepthStencilView = NULL;
			}

			if (m_frameBufferDepthStencilTexture != NULL)
			{
				m_frameBufferDepthStencilTexture->Release();
				m_frameBufferDepthStencilTexture = NULL;
			}
		}

		// resize
		// NOTE: when in fullscreen mode, use 0 as width/height (because they were set internally by SetFullscreenState())
		// NOTE: DXGI_FORMAT_UNKNOWN preserves the existing format
		hr = m_swapChain->ResizeBuffers(0, (m_bIsFullscreen && !m_bIsFullscreenBorderlessWindowed ? 0 : (UINT)newResolution.x), (m_bIsFullscreen && !m_bIsFullscreenBorderlessWindowed ? 0 : (UINT)newResolution.y), DXGI_FORMAT::DXGI_FORMAT_UNKNOWN, /*DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH*/0);
		if (FAILED(hr))
			debugLog("FATAL ERROR: DirectX11Interface::onResolutionChange() couldn't ResizeBuffers(%ld, %x, %x)!!!\n", hr, hr, MAKE_DXGI_HRESULT(hr));

		// get new (automatically generated) backbuffer
		ID3D11Texture2D *backBuffer;
		hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
		if (FAILED(hr))
		{
			debugLog("FATAL ERROR: DirectX11Interface::onResolutionChange() couldn't GetBuffer(%ld, %x, %x)!!!\n", hr, hr, MAKE_DXGI_HRESULT(hr));
			return;
		}

		// read actual new width/height of backbuffer
		{
			D3D11_TEXTURE2D_DESC backBufferTextureDesc;
			backBuffer->GetDesc(&backBufferTextureDesc);

			// NOTE: force overwrite local resolution (even though it was just set at the start of onResolutionChange() here)
			m_vResolution.x = (float)backBufferTextureDesc.Width;
			m_vResolution.y = (float)backBufferTextureDesc.Height;
		}

		// and create new framebuffer from it
		hr = m_device->CreateRenderTargetView(backBuffer, NULL, &m_frameBuffer);
		backBuffer->Release(); // (release temp buffer)
		if (FAILED(hr))
		{
			debugLog("FATAL ERROR: DirectX11Interface::onResolutionChange() couldn't CreateRenderTargetView(%ld, %x, %x)!!!\n", hr, hr, MAKE_DXGI_HRESULT(hr));
			m_frameBuffer = NULL;
			return;
		}

		// add new depth buffer
		{
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
				depthStencilTextureDesc.Width = (UINT)m_vResolution.x;
				depthStencilTextureDesc.Height = (UINT)m_vResolution.y;
			}

			hr = m_device->CreateTexture2D(&depthStencilTextureDesc, NULL, &m_frameBufferDepthStencilTexture);
			if (FAILED(hr))
			{
				debugLog("FATAL ERROR: DirectX11Interface::onResolutionChange() couldn't CreateTexture2D(%ld, %x, %x)!!!\n", hr, hr, MAKE_DXGI_HRESULT(hr));
				m_frameBufferDepthStencilTexture = NULL;
			}
			else
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
				{
					depthStencilViewDesc.Format = depthStencilTextureDesc.Format;
					depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2D;
					depthStencilViewDesc.Flags = 0;
					depthStencilViewDesc.Texture2D.MipSlice = 0;
				}

				hr = m_device->CreateDepthStencilView(m_frameBufferDepthStencilTexture, &depthStencilViewDesc, &m_frameBufferDepthStencilView);
				if (FAILED(hr))
				{
					debugLog("FATAL ERROR: DirectX11Interface::onResolutionChange() couldn't CreateDepthStencilView(%ld, %x, %x)!!!\n", hr, hr, MAKE_DXGI_HRESULT(hr));
					m_frameBufferDepthStencilView = NULL;
				}
			}
		}

		// use new framebuffer
		m_deviceContext->OMSetRenderTargets(1, &m_frameBuffer, m_frameBufferDepthStencilView);
	}

	// rebuild viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	{
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = m_vResolution.x;
		viewport.Height = m_vResolution.y;
		viewport.MinDepth = 0.0f; // NOTE: between 0 and 1
		viewport.MaxDepth = 1.0f; // NOTE: between 0 and 1
	}
	m_deviceContext->RSSetViewports(1, &viewport);
}

void DirectX11Interface::resizeTarget(Vector2 newResolution)
{
	m_swapChainModeDesc.Width = (UINT)newResolution.x;
	m_swapChainModeDesc.Height = (UINT)newResolution.y;

	m_swapChain->ResizeTarget(&m_swapChainModeDesc); // NOTE: this will resize the actual window and send WM_SIZE which will in turn call onResolutionChange() here
}

bool DirectX11Interface::enableFullscreen(bool borderlessWindowedFullscreen)
{
	m_bIsFullscreenBorderlessWindowed = borderlessWindowedFullscreen;

	if (!m_bIsFullscreenBorderlessWindowed)
	{
		HRESULT hr = m_swapChain->SetFullscreenState((WINBOOL)true, NULL);
		m_bIsFullscreen = !FAILED(hr);
	}
	else
		m_bIsFullscreen = true; // ("fake" fullscreen)

	return m_bIsFullscreen;
}

void DirectX11Interface::disableFullscreen()
{
	if (!m_bIsFullscreen) return;

	if (!m_bIsFullscreenBorderlessWindowed)
		m_swapChain->SetFullscreenState((WINBOOL)false, NULL);

	m_bIsFullscreen = false;
	m_bIsFullscreenBorderlessWindowed = false;
}

Image *DirectX11Interface::createImage(UString filePath, bool mipmapped, bool keepInSystemMemory)
{
	return new DirectX11Image(filePath, mipmapped, keepInSystemMemory);
}

Image *DirectX11Interface::createImage(int width, int height, bool mipmapped, bool keepInSystemMemory)
{
	return new DirectX11Image(width, height, mipmapped, keepInSystemMemory);
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

Shader *DirectX11Interface::createShaderFromFile(UString shaderFilePath)
{
	return new DirectX11Shader(shaderFilePath, false);
}

Shader *DirectX11Interface::createShaderFromSource(UString shaderSource)
{
	return new DirectX11Shader(shaderSource, true);
}

VertexArrayObject *DirectX11Interface::createVertexArrayObject(Graphics::PRIMITIVE primitive, Graphics::USAGE_TYPE usage, bool keepInSystemMemory)
{
	return new DirectX11VertexArrayObject(primitive, usage, keepInSystemMemory);
}

void DirectX11Interface::onTransformUpdate(Matrix4 &projectionMatrix, Matrix4 &worldMatrix)
{
	// NOTE: convert from OpenGL coordinate space
	static Matrix4 zflip = Matrix4().scale(1, 1, -1);

	Matrix4 mvp = m_MP * zflip;
	m_shaderTexturedGeneric->setUniformMatrix4fv("mvp", mvp);
}

int DirectX11Interface::primitiveToDirectX(Graphics::PRIMITIVE primitive)
{
	switch (primitive)
	{
	case Graphics::PRIMITIVE::PRIMITIVE_LINES:
		return D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	case Graphics::PRIMITIVE::PRIMITIVE_LINE_STRIP:
		return D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
	case Graphics::PRIMITIVE::PRIMITIVE_TRIANGLES:
		return D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case Graphics::PRIMITIVE::PRIMITIVE_TRIANGLE_FAN:	// NOTE: not available! -------------------
		return D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
	case Graphics::PRIMITIVE::PRIMITIVE_TRIANGLE_STRIP:
		return D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	case Graphics::PRIMITIVE::PRIMITIVE_QUADS:			// NOTE: not available! -------------------
		return D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
	}

	return D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

int DirectX11Interface::compareFuncToDirectX(Graphics::COMPARE_FUNC compareFunc)
{
	// TODO: implement

	return 0;
}

#endif

