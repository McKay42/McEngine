//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		DirectX HLSL implementation of Shader
//
// $NoKeywords: $dxshader
//===============================================================================//

#include "DirectX11Shader.h"

#ifdef MCENGINE_FEATURE_DIRECTX

#include "Engine.h"
#include "ConVar.h"

#include "DirectX11Interface.h"
#include "d3dcompiler.h"

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

	((DirectX11Interface*)engine->getGraphics())->getDeviceContext()->IAGetInputLayout(&m_prevInputLayout); // backup
	((DirectX11Interface*)engine->getGraphics())->getDeviceContext()->VSGetShader(&m_prevVS, NULL, NULL); // backup
	((DirectX11Interface*)engine->getGraphics())->getDeviceContext()->PSGetShader(&m_prevPS, NULL, NULL); // backup

	((DirectX11Interface*)engine->getGraphics())->getDeviceContext()->IASetInputLayout(m_inputLayout);
	((DirectX11Interface*)engine->getGraphics())->getDeviceContext()->VSSetShader(m_vs, NULL, 0);
	((DirectX11Interface*)engine->getGraphics())->getDeviceContext()->PSSetShader(m_ps, NULL, 0);
}

void DirectX11Shader::disable()
{
	if (!m_bReady) return;

	((DirectX11Interface*)engine->getGraphics())->getDeviceContext()->IASetInputLayout(m_prevInputLayout); // restore
	((DirectX11Interface*)engine->getGraphics())->getDeviceContext()->VSSetShader(m_prevVS, NULL, 0); // restore
	((DirectX11Interface*)engine->getGraphics())->getDeviceContext()->PSSetShader(m_prevPS, NULL, 0); // restore
}

void DirectX11Shader::setUniform1f(UString name, float value)
{
	if (!m_bReady || m_constantBuffer == NULL) return;

	// lock
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(((DirectX11Interface*)engine->getGraphics())->getDeviceContext()->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
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
	((DirectX11Interface*)engine->getGraphics())->getDeviceContext()->Unmap(m_constantBuffer, 0);
}

void DirectX11Shader::setUniform4f(UString name, float x, float y, float z, float w)
{
	if (!m_bReady || m_constantBuffer == NULL) return;

	// lock
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(((DirectX11Interface*)engine->getGraphics())->getDeviceContext()->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
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
	((DirectX11Interface*)engine->getGraphics())->getDeviceContext()->Unmap(m_constantBuffer, 0);
}

void DirectX11Shader::setUniformMatrix4fv(UString name, Matrix4 &matrix)
{
	setUniformMatrix4fv(name, (float*)matrix.getTranspose());
}

void DirectX11Shader::setUniformMatrix4fv(UString name, float *v)
{
	if (!m_bReady || m_constantBuffer == NULL || v == NULL) return;

	// lock
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(((DirectX11Interface*)engine->getGraphics())->getDeviceContext()->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
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
	((DirectX11Interface*)engine->getGraphics())->getDeviceContext()->Unmap(m_constantBuffer, 0);
}

bool DirectX11Shader::compile(UString vertexShader, UString fragmentShader, bool source)
{
	// TODO: file support!

	const char *vsProfile = (((DirectX11Interface*)engine->getGraphics())->getDevice()->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "vs_5_0" : "vs_4_0";
	const char *psProfile = (((DirectX11Interface*)engine->getGraphics())->getDevice()->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "ps_5_0" : "ps_4_0";
	const char *vsEntryPoint = "vsmain";
	const char *psEntryPoint = "psmain";

	UINT flags = D3DCOMPILE_DEBUG;

	const D3D_SHADER_MACRO defines[] =
	{
		"EXAMPLE_DEFINE", "1",
		NULL, NULL
	};

	ID3D10Blob *vs = NULL, *ps = NULL;
	ID3D10Blob *vsError = NULL, *psError = NULL;
	HRESULT hr1, hr2;

	// compile
	debugLog("Shader: Compiling %s ...\n", vertexShader.toUtf8());
	hr1 = D3DCompile(vertexShader.toUtf8(), vertexShader.length(), "VS", defines, NULL, vsEntryPoint, vsProfile, flags, 0, &vs, &vsError);

	debugLog("Shader: Compiling %s ...\n", fragmentShader.toUtf8());
	hr2 = D3DCompile(fragmentShader.toUtf8(), fragmentShader.length(), "PS", defines, NULL, psEntryPoint, psProfile, flags, 0, &ps, &psError);

	if (FAILED(hr1) || FAILED(hr2) || vs == NULL || ps == NULL)
	{
		if (vsError != NULL)
		{
			debugLog("Vertex Shader Error: \n%s\n", (const char *)vsError->GetBufferPointer());
			vsError->Release();
			debugLog("\n");
		}

		if (psError != NULL)
		{
			debugLog("Pixel Shader Error: \n%s\n", (const char *)psError->GetBufferPointer());
			psError->Release();
			debugLog("\n");
		}

		engine->showMessageError("Shader Error", "Couldn't D3DCompile()!");
		return false;
	}

	// encapsulate
	debugLog("Shader: CreateVertexShader(%i) ...\n", vs->GetBufferSize());
	hr1 = ((DirectX11Interface*)engine->getGraphics())->getDevice()->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), NULL, &m_vs);
	// note how vs is not released here, since it's still needed for the input layout

	debugLog("Shader: CreatePixelShader(%i) ...\n", ps->GetBufferSize());
	hr2 = ((DirectX11Interface*)engine->getGraphics())->getDevice()->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), NULL, &m_ps);
	ps->Release();

	if (FAILED(hr1) || FAILED(hr2))
	{
		vs->Release();
		engine->showMessageError("Shader Error", "Couldn't CreateVertexShader()/CreatePixelShader()!");
		return false;
	}

	// create the input layout
	// TODO: extract this into public functions
	D3D11_INPUT_ELEMENT_DESC elements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },	// + 3*4
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },	// + 4*4
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },		// + 2*4
		//{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // + 3*4
	};
	UINT numElements = _countof(elements);

	hr1 = ((DirectX11Interface*)engine->getGraphics())->getDevice()->CreateInputLayout(elements, numElements, vs->GetBufferPointer(), vs->GetBufferSize(), &m_inputLayout);
	vs->Release();

	if (FAILED(hr1))
	{
		engine->showMessageError("Shader Error", "Couldn't CreateInputLayout()!");
		return false;
	}

	// TODO: extract this into public functions
	// create contant buffer
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	hr1 = ((DirectX11Interface*)engine->getGraphics())->getDevice()->CreateBuffer(&matrixBufferDesc, NULL, &m_constantBuffer);
	if (FAILED(hr1))
	{
		engine->showMessageError("Shader Error", UString::format("Couldn't CreateBuffer(%ld, %x, %x)!", hr1, hr1, MAKE_DXGI_HRESULT(hr1)));
		return false;
	}

	// TODO: set buffer number?
	UINT bufferNumber = 0;
	((DirectX11Interface*)engine->getGraphics())->getDeviceContext()->VSSetConstantBuffers(bufferNumber, 1, &m_constantBuffer);

	return true;
}

#endif

