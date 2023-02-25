//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		DirectX HLSL implementation of Shader
//
// $NoKeywords: $dxshader
//===============================================================================//

#include "DirectX11Shader.h"

#ifdef MCENGINE_FEATURE_DIRECTX11

#include "Engine.h"
#include "ConVar.h"

#include "DirectX11Interface.h"
#include "d3dcompiler.h"

#include <sstream>

//#define MCENGINE_D3D11_CREATE_SHADER_DEBUG

DirectX11Shader::DirectX11Shader(UString vertexShader, UString fragmentShader, bool source)
{
	m_sVsh = vertexShader;
	m_sFsh = fragmentShader;
	m_bSource = source;

	m_vs = NULL;
	m_ps = NULL;
	m_inputLayout = NULL;
	m_constantBuffer = NULL;

	m_prevVS = NULL;
	m_prevPS = NULL;
	m_prevInputLayout = NULL;
}

void DirectX11Shader::init()
{
	m_bReady = compile(m_sVsh, m_sFsh, m_bSource);
}

void DirectX11Shader::initAsync()
{
	m_bAsyncReady = true;
}

void DirectX11Shader::destroy()
{
	if (m_constantBuffer != NULL)
		m_constantBuffer->Release();

	if (m_inputLayout != NULL)
		m_inputLayout->Release();

	if (m_vs != NULL)
		m_vs->Release();

	if (m_ps != NULL)
		m_ps->Release();
}

void DirectX11Shader::enable()
{
	if (!m_bReady) return;

	DirectX11Interface *dx11 = (DirectX11Interface*)engine->getGraphics();

	dx11->getDeviceContext()->IAGetInputLayout(&m_prevInputLayout); // backup
	dx11->getDeviceContext()->VSGetShader(&m_prevVS, NULL, NULL); // backup
	dx11->getDeviceContext()->PSGetShader(&m_prevPS, NULL, NULL); // backup

	dx11->getDeviceContext()->IASetInputLayout(m_inputLayout);
	dx11->getDeviceContext()->VSSetShader(m_vs, NULL, 0);
	dx11->getDeviceContext()->PSSetShader(m_ps, NULL, 0);
}

void DirectX11Shader::disable()
{
	if (!m_bReady) return;

	DirectX11Interface *dx11 = (DirectX11Interface*)engine->getGraphics();

	dx11->getDeviceContext()->IASetInputLayout(m_prevInputLayout); // restore
	dx11->getDeviceContext()->VSSetShader(m_prevVS, NULL, 0); // restore
	dx11->getDeviceContext()->PSSetShader(m_prevPS, NULL, 0); // restore

	// refcount
	{
		if (m_prevInputLayout != NULL)
		{
			m_prevInputLayout->Release();
			m_prevInputLayout = NULL;
		}

		if (m_prevVS != NULL)
		{
			m_prevVS->Release();
			m_prevVS = NULL;
		}

		if (m_prevPS != NULL)
		{
			m_prevPS->Release();
			m_prevPS = NULL;
		}
	}
}

void DirectX11Shader::setUniform1f(UString name, float value)
{
	if (!m_bReady || m_constantBuffer == NULL) return;

	DirectX11Interface *dx11 = (DirectX11Interface*)engine->getGraphics();

	// lock
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(dx11->getDeviceContext()->Map(m_constantBuffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		if (debug_shaders->getBool())
			debugLog("DirectX11Shader::setUniformMatrix4fv() couldn't Map()!\n");

		return;
	}

	// write data
	// TODO: hardcoded pData->mvp!!!
	for (int i=0; i<4; i++)
	{
		m_constants.misc[i] = value;
	}
	memcpy(mappedResource.pData, &m_constants, sizeof(MatrixBufferType));

	// unlock
	dx11->getDeviceContext()->Unmap(m_constantBuffer, 0);
}

void DirectX11Shader::setUniform1fv(UString name, int count, float *values)
{
	// TODO: implement
}

void DirectX11Shader::setUniform1i(UString name, int value)
{
	// TODO: implement
}

void DirectX11Shader::setUniform2f(UString name, float x, float y)
{
	// TODO: implement
}

void DirectX11Shader::setUniform2fv(UString name, int count, float *vectors)
{
	// TODO: implement
}

void DirectX11Shader::setUniform3f(UString name, float x, float y, float z)
{
	// TODO: implement
}

void DirectX11Shader::setUniform3fv(UString name, int count, float *vectors)
{
	// TODO: implement
}

void DirectX11Shader::setUniform4f(UString name, float x, float y, float z, float w)
{
	if (!m_bReady || m_constantBuffer == NULL) return;

	DirectX11Interface *dx11 = (DirectX11Interface*)engine->getGraphics();

	// lock
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(dx11->getDeviceContext()->Map(m_constantBuffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		if (debug_shaders->getBool())
			debugLog("DirectX11Shader::setUniformMatrix4fv() couldn't Map()!\n");

		return;
	}

	// write data
	// TODO: hardcoded pData->mvp!!!
	m_constants.col[0] = x;
	m_constants.col[1] = y;
	m_constants.col[2] = z;
	m_constants.col[3] = w;
	memcpy(mappedResource.pData, &m_constants, sizeof(MatrixBufferType));

	// unlock
	dx11->getDeviceContext()->Unmap(m_constantBuffer, 0);
}

void DirectX11Shader::setUniformMatrix4fv(UString name, Matrix4 &matrix)
{
	setUniformMatrix4fv(name, (float*)matrix.getTranspose());
}

void DirectX11Shader::setUniformMatrix4fv(UString name, float *v)
{
	if (!m_bReady || m_constantBuffer == NULL || v == NULL) return;

	DirectX11Interface *dx11 = (DirectX11Interface*)engine->getGraphics();

	// lock
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(dx11->getDeviceContext()->Map(m_constantBuffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		if (debug_shaders->getBool())
			debugLog("DirectX11Shader::setUniformMatrix4fv() couldn't Map()!\n");

		return;
	}

	// write data
	// TODO: hardcoded pData->mvp!!!
	for (int i=0; i<4*4; i++)
	{
		m_constants.mvp[i] = v[i];
	}
	memcpy(mappedResource.pData, &m_constants, sizeof(MatrixBufferType));

	// unlock
	dx11->getDeviceContext()->Unmap(m_constantBuffer, 0);
}

bool DirectX11Shader::compile(UString vertexShader, UString fragmentShader, bool source)
{
	if (!source)
	{
		// vs
		{
			std::ifstream inFile(vertexShader.toUtf8());
			if (!inFile)
			{
				engine->showMessageError("DirectX11Shader Error", vertexShader);
				return false;
			}
			std::stringstream buffer;
			buffer << inFile.rdbuf();
			const std::string str = buffer.str();
			vertexShader = UString(str.c_str(), str.length());
		}

		// ps
		{
			std::ifstream inFile(fragmentShader.toUtf8());
			if (!inFile)
			{
				engine->showMessageError("DirectX11Shader Error", fragmentShader);
				return false;
			}
			std::stringstream buffer;
			buffer << inFile.rdbuf();
			const std::string str = buffer.str();
			fragmentShader = UString(str.c_str(), str.length());
		}
	}

	DirectX11Interface *dx11 = (DirectX11Interface*)engine->getGraphics();

	const char *vsProfile = (dx11->getDevice()->GetFeatureLevel() >= D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0 ? "vs_5_0" : "vs_4_0");
	const char *psProfile = (dx11->getDevice()->GetFeatureLevel() >= D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0 ? "ps_5_0" : "ps_4_0");
	const char *vsEntryPoint = "vsmain";
	const char *psEntryPoint = "psmain";

	UINT flags = 0;

#ifdef MCENGINE_D3D11_CREATE_SHADER_DEBUG

	flags |= D3DCOMPILE_DEBUG;

#endif

	const D3D_SHADER_MACRO defines[] =
	{
		"EXAMPLE_DEFINE", "1",
		NULL, NULL // sentinel
	};

	ID3D10Blob *vs = NULL, *ps = NULL;
	ID3D10Blob *vsError = NULL, *psError = NULL;
	HRESULT hr1, hr2;

	// compile
	debugLog("DirectX11Shader: Compiling %s ...\n", (source ? "vertex source" : vertexShader.toUtf8()));
	hr1 = D3DCompile(vertexShader.toUtf8(), vertexShader.length(), "VS", defines, NULL, vsEntryPoint, vsProfile, flags, 0, &vs, &vsError);

	debugLog("DirectX11Shader: Compiling %s ...\n", (source ? "pixel source" : fragmentShader.toUtf8()));
	hr2 = D3DCompile(fragmentShader.toUtf8(), fragmentShader.length(), "PS", defines, NULL, psEntryPoint, psProfile, flags, 0, &ps, &psError);

	if (FAILED(hr1) || FAILED(hr2) || vs == NULL || ps == NULL)
	{
		if (vsError != NULL)
		{
			debugLog("DirectX11Shader Vertex Shader Error: \n%s\n", (const char*)vsError->GetBufferPointer());
			vsError->Release();
			debugLog("\n");
		}

		if (psError != NULL)
		{
			debugLog("DirectX11Shader Pixel Shader Error: \n%s\n", (const char*)psError->GetBufferPointer());
			psError->Release();
			debugLog("\n");
		}

		engine->showMessageError("DirectX11Shader Error", "Couldn't D3DCompile()!");
		return false;
	}

	// encapsulate
	debugLog("DirectX11Shader: CreateVertexShader(%i) ...\n", vs->GetBufferSize());
	hr1 = dx11->getDevice()->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), NULL, &m_vs);
	// (note how vs is not released here, since it's still needed for the input layout)

	debugLog("DirectX11Shader: CreatePixelShader(%i) ...\n", ps->GetBufferSize());
	hr2 = dx11->getDevice()->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), NULL, &m_ps);
	ps->Release();

	if (FAILED(hr1) || FAILED(hr2))
	{
		vs->Release();
		engine->showMessageError("DirectX11Shader Error", "Couldn't CreateVertexShader()/CreatePixelShader()!");
		return false;
	}

	// create the input layout
	// TODO: extract this into public functions
	D3D11_INPUT_ELEMENT_DESC elements[] =
	{
		{ "POSITION",	0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,	0,	0,	D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 },	// + 3*4
		{ "COLOR",		0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	12,	D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 },	// + 4*4
		{ "TEXCOORD",	0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT,		0,	28,	D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 },	// + 2*4
		//{ "NORMAL",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,				0,	36,	D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 },	// + 3*4
	};
	UINT numElements = _countof(elements);

	hr1 = dx11->getDevice()->CreateInputLayout(elements, numElements, vs->GetBufferPointer(), vs->GetBufferSize(), &m_inputLayout);
	vs->Release(); // (now we can release it)

	if (FAILED(hr1))
	{
		engine->showMessageError("DirectX11Shader Error", "Couldn't CreateInputLayout()!");
		return false;
	}

	// TODO: extract this into public functions
	// create contant buffer
	D3D11_BUFFER_DESC matrixBufferDesc;
	{
		matrixBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
		matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType); // NOTE: "If the bind flag is D3D11_BIND_CONSTANT_BUFFER, you must set the ByteWidth value in multiples of 16, and less than or equal to D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT."
		matrixBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
		matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
		matrixBufferDesc.MiscFlags = 0;
		matrixBufferDesc.StructureByteStride = 0;
	}
	hr1 = dx11->getDevice()->CreateBuffer(&matrixBufferDesc, NULL, &m_constantBuffer);
	if (FAILED(hr1))
	{
		engine->showMessageError("DirectX11Shader Error", UString::format("Couldn't CreateBuffer(%ld, %x, %x)!", hr1, hr1, MAKE_DXGI_HRESULT(hr1)));
		return false;
	}

	// TODO: set buffer number?
	// TODO: this is a global number across all shaders, so needs to be specified in the shader via descs probably
	UINT bufferNumber = 0;
	dx11->getDeviceContext()->VSSetConstantBuffers(bufferNumber, 1, &m_constantBuffer);

	return true;
}

#endif

