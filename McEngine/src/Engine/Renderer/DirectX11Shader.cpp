//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		DirectX HLSL implementation of Shader
//
// $NoKeywords: $dxshader
//===============================================================================//

// TODO: prime full cache on load anyway
// TODO: individually remember m_bConstantBuffersUpToDate per constant buffer

#include "DirectX11Shader.h"

#ifdef MCENGINE_FEATURE_DIRECTX11

#include "Engine.h"
#include "ConVar.h"

#include "DirectX11Interface.h"

#include "d3dcompiler.h"

#include <sstream>

//#define MCENGINE_D3D11_CREATE_SHADER_DEBUG
#define MCENGINE_D3D11_SHADER_MAX_NUM_CONSTANT_BUFFERS 9

DirectX11Shader::CACHE_ENTRY DirectX11Shader::invalidCacheEntry {-1};

DirectX11Shader::DirectX11Shader(UString shader, bool source)
{
	m_sShader = shader;
	m_bSource = source;

	m_vs = NULL;
	m_ps = NULL;
	m_inputLayout = NULL;
	m_bConstantBuffersUpToDate = false;

	m_prevShader = NULL;
	m_prevVS = NULL;
	m_prevPS = NULL;
	m_prevInputLayout = NULL;
	for (int i=0; i<MCENGINE_D3D11_SHADER_MAX_NUM_CONSTANT_BUFFERS; i++)
	{
		m_prevConstantBuffers.push_back(NULL);
	}

	// stats
	m_iStatsNumConstantBufferUploadsPerFrameCounter = 0;
	m_iStatsNumConstantBufferUploadsPerFrameCounterEngineFrameCount = 0;
}

DirectX11Shader::DirectX11Shader(UString vertexShader, UString fragmentShader, bool source)
{
	m_sVsh = vertexShader;
	m_sFsh = fragmentShader;
	m_bSource = source;

	m_vs = NULL;
	m_ps = NULL;
	m_inputLayout = NULL;
	m_bConstantBuffersUpToDate = false;

	m_prevShader = NULL;
	m_prevVS = NULL;
	m_prevPS = NULL;
	m_prevInputLayout = NULL;
	for (int i=0; i<MCENGINE_D3D11_SHADER_MAX_NUM_CONSTANT_BUFFERS; i++)
	{
		m_prevConstantBuffers.push_back(NULL);
	}

	// stats
	m_iStatsNumConstantBufferUploadsPerFrameCounter = 0;
	m_iStatsNumConstantBufferUploadsPerFrameCounterEngineFrameCount = 0;
}

void DirectX11Shader::init()
{
	const SHADER_PARSE_RESULT parsedVertexShader = parseShaderFromFileOrString("DirectX11Interface::VertexShader", (m_sShader.length() > 0 ? m_sShader : m_sVsh), m_bSource);
	const SHADER_PARSE_RESULT parsedPixelShader = parseShaderFromFileOrString("DirectX11Interface::PixelShader", (m_sShader.length() > 0 ? m_sShader : m_sFsh), m_bSource);

	bool valid = (parsedVertexShader.descs.size() > 0);
	{
		// parse lines
		std::vector<INPUT_DESC_LINE> inputDescLines;
		std::vector<BIND_DESC_LINE> bindDescLines;
		for (size_t i=0; i<parsedVertexShader.descs.size(); i++)
		{
			const UString &desc = parsedVertexShader.descs[i];
			const std::vector<UString> tokens = desc.split("::");

			if (debug_shaders->getBool())
				debugLog("descs[%i] = %s\n", (int)i, desc.toUtf8());

			if (tokens.size() > 4)
			{
				const UString &descType = tokens[0];

				if (debug_shaders->getBool())
				{
					for (size_t t=0; t<tokens.size(); t++)
					{
						debugLog("descs[%i][%i] = %s\n", (int)i, (int)t, tokens[t].toUtf8());
					}
				}

				if (descType == "D3D11_INPUT_ELEMENT_DESC")
				{
					const UString &inputType = tokens[1];			// e.g. VS_INPUT
					const UString &inputDataType = tokens[2];		// e.g. POSITION or COLOR0 or TEXCOORD0 etc.
					const UString &inputFormat = tokens[3];			// e.g. DXGI_FORMAT_R32G32B32_FLOAT or DXGI_FORMAT_R32G32B32A32_FLOAT or DXGI_FORMAT_R32G32_FLOAT etc.
					const UString &inputClassification = tokens[4];	// e.g. D3D11_INPUT_PER_VERTEX_DATA

					INPUT_DESC_LINE inputDescLine;
					{
						inputDescLine.type = inputType;
						{
							inputDescLine.dataType = inputDataType;

							// NOTE: remove integer from end of datatype string (e.g. "COLOR0", "TEXCOORD0" etc., since this is implied by the order and only necessary in actual shader code as CreateInputLayout() would fail otherwise)
							if (inputDescLine.dataType.find("0") == inputDescLine.dataType.length() - 1
							 || inputDescLine.dataType.find("1") == inputDescLine.dataType.length() - 1
							 || inputDescLine.dataType.find("2") == inputDescLine.dataType.length() - 1
							 || inputDescLine.dataType.find("3") == inputDescLine.dataType.length() - 1
							 || inputDescLine.dataType.find("4") == inputDescLine.dataType.length() - 1
							 || inputDescLine.dataType.find("5") == inputDescLine.dataType.length() - 1
							 || inputDescLine.dataType.find("6") == inputDescLine.dataType.length() - 1
							 || inputDescLine.dataType.find("7") == inputDescLine.dataType.length() - 1
							 || inputDescLine.dataType.find("8") == inputDescLine.dataType.length() - 1
							 || inputDescLine.dataType.find("9") == inputDescLine.dataType.length() - 1)
							{
								inputDescLine.dataType.erase(inputDescLine.dataType.length() - 1, 1);
							}
						}
						{
							if (inputFormat == "DXGI_FORMAT_R32_FLOAT")
							{
								inputDescLine.dxgiFormat = DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT;
								inputDescLine.dxgiFormatBytes = 1*4;
							}
							else if (inputFormat == "DXGI_FORMAT_R32G32_FLOAT")
							{
								inputDescLine.dxgiFormat = DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT;
								inputDescLine.dxgiFormatBytes = 2*4;
							}
							else if (inputFormat == "DXGI_FORMAT_R32G32B32_FLOAT")
							{
								inputDescLine.dxgiFormat = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
								inputDescLine.dxgiFormatBytes = 3*4;
							}
							else if (inputFormat == "DXGI_FORMAT_R32G32B32A32_FLOAT")
							{
								inputDescLine.dxgiFormat = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
								inputDescLine.dxgiFormatBytes = 4*4;
							}
							else
							{
								valid = false;
								engine->showMessageError("DirectX11Shader Error", UString::format("Invalid/Unsupported inputFormat \"%s\"", inputFormat.toUtf8()));
								break;
							}
						}
						{
							if (inputClassification == "D3D11_INPUT_PER_VERTEX_DATA")
								inputDescLine.classification = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
							else
							{
								valid = false;
								engine->showMessageError("DirectX11Shader Error", UString::format("Invalid/Unsupported inputClassification \"%s\"", inputClassification.toUtf8()));
								break;
							}
						}
					}
					inputDescLines.push_back(inputDescLine);
				}
				else if (descType == "D3D11_BUFFER_DESC")
				{
					const UString &bufferBindType = tokens[1]; 			// e.g. D3D11_BIND_CONSTANT_BUFFER
					const UString &bufferName = tokens[2];				// e.g. ModelViewProjectionConstantBuffer
					const UString &bufferVariableName = tokens[3];		// e.g. mvp or col or misc etc.
					const UString &bufferVariableType = tokens[4];		// e.g. float4x4 or float4 etc.

					BIND_DESC_LINE bindDescLine;
					{
						bindDescLine.type = descType; // NOTE: not bufferBindType! since we want to be able to support more than just D3D11_BUFFER_DESC
						{
							if (bufferBindType == "D3D11_BIND_CONSTANT_BUFFER")
								bindDescLine.bindFlag = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
							else
							{
								valid = false;
								engine->showMessageError("DirectX11Shader Error", UString::format("Invalid/Unsupported bufferBindType \"%s\"", bufferBindType.toUtf8()));
								break;
							}
						}
						bindDescLine.name = bufferName;
						bindDescLine.variableName = bufferVariableName;
						{
							bindDescLine.variableType = bufferVariableType;
							if (bufferVariableType == "float")
								bindDescLine.variableBytes = 1*4;
							else if (bufferVariableType == "int")
								bindDescLine.variableBytes = 1*4;
							else if (bufferVariableType == "float2")
								bindDescLine.variableBytes = 2*4;
							else if (bufferVariableType == "float3")
								bindDescLine.variableBytes = 3*4;
							else if (bufferVariableType == "float4")
								bindDescLine.variableBytes = 4*4;
							else if (bufferVariableType == "float4x4")
								bindDescLine.variableBytes = 4*4*4;
							else
							{
								valid = false;
								engine->showMessageError("DirectX11Shader Error", UString::format("Invalid/Unsupported bufferVariableType \"%s\"", bufferVariableType.toUtf8()));
								break;
							}
						}
					}
					bindDescLines.push_back(bindDescLine);
				}
				else
				{
					valid = false;
					engine->showMessageError("DirectX11Shader Error", UString::format("Invalid/Unsupported descType \"%s\"", descType.toUtf8()));
					break;
				}
			}
			else
			{
				valid = false;
				engine->showMessageError("DirectX11Shader Error", UString::format("Invalid desc \"%s\"", desc.toUtf8()));
				break;
			}
		}

		// build m_inputDescs + m_bindDescs
		{
			// compound by INPUT_DESC_LINE::type
			for (size_t i=0; i<inputDescLines.size(); i++)
			{
				const INPUT_DESC_LINE &inputDescLine = inputDescLines[i];

				bool alreadyExists = false;
				for (INPUT_DESC &inputDesc : m_inputDescs)
				{
					if (inputDesc.type == inputDescLine.type)
					{
						alreadyExists = true;
						{
							inputDesc.lines.push_back(inputDescLine);
						}
						break;
					}
				}
				if (!alreadyExists)
				{
					INPUT_DESC inputDesc;
					{
						inputDesc.type = inputDescLine.type;
					}
					m_inputDescs.push_back(inputDesc);

					// (repeat to avoid code duplication)
					i--;
					continue;
				}
			}

			// compound by BIND_DESC_LINE::name
			for (size_t i=0; i<bindDescLines.size(); i++)
			{
				const BIND_DESC_LINE &bindDescLine = bindDescLines[i];

				bool alreadyExists = false;
				for (BIND_DESC &bindDesc : m_bindDescs)
				{
					if (bindDesc.name == bindDescLine.name)
					{
						alreadyExists = true;
						{
							bindDesc.lines.push_back(bindDescLine);
							bindDesc.floats.resize(bindDesc.floats.size() + (bindDescLine.variableBytes / sizeof(float)));

							if (debug_shaders->getBool())
								debugLog("bindDesc[%s].floats.size() = %i\n", bindDescLine.name.toUtf8(), (int)bindDesc.floats.size());
						}
						break;
					}
				}

				if (!alreadyExists)
				{
					BIND_DESC bindDesc;
					{
						bindDesc.name = bindDescLine.name;
					}
					m_bindDescs.push_back(bindDesc);

					// (repeat to avoid code duplication)
					i--;
					continue;
				}
			}

			// error checking
			if (m_inputDescs.size() < 1)
			{
				valid = false;
				engine->showMessageError("DirectX11Shader Error", "Missing at least one D3D11_INPUT_ELEMENT_DESC instance");
			}
			else if (m_bindDescs.size() < 1)
			{
				// (there could theoretically be a shader without any buffers bound, so this is not an error)
			}

			if (debug_shaders->getBool())
			{
				for (size_t i=0; i<m_inputDescs.size(); i++)
				{
					debugLog("m_inputDescs[%i] = \"%s\", has %i line(s)\n", (int)i, m_inputDescs[i].type.toUtf8(), (int)m_inputDescs[i].lines.size());
				}

				for (size_t i=0; i<m_bindDescs.size(); i++)
				{
					debugLog("m_bindDescs[%i] = \"%s\", has %i lines(s)\n", (int)i, m_bindDescs[i].name.toUtf8(), (int)m_bindDescs[i].lines.size());
				}
			}
		}
	}

	m_bReady = compile((valid ? parsedVertexShader.source : ""), (valid ? parsedPixelShader.source : ""));
}

void DirectX11Shader::initAsync()
{
	m_bAsyncReady = true;
}

void DirectX11Shader::destroy()
{
	for (ID3D11Buffer *buffer : m_constantBuffers)
	{
		buffer->Release();
	}
	m_constantBuffers.clear();

	if (m_inputLayout != NULL)
	{
		m_inputLayout->Release();
		m_inputLayout = NULL;
	}

	if (m_vs != NULL)
	{
		m_vs->Release();
		m_vs = NULL;
	}

	if (m_ps != NULL)
	{
		m_ps->Release();
		m_ps = NULL;
	}

	m_inputDescs.clear();
	m_bindDescs.clear();

	m_uniformLocationCache.clear();
}

void DirectX11Shader::enable()
{
	DirectX11Interface *dx11 = (DirectX11Interface*)engine->getGraphics();
	if (!m_bReady || dx11->getActiveShader() == this) return;

	// backup
	// HACKHACK: slow af
	{
		m_prevShader = dx11->getActiveShader();

		dx11->getDeviceContext()->IAGetInputLayout(&m_prevInputLayout);
		dx11->getDeviceContext()->VSGetShader(&m_prevVS, NULL, NULL);
		dx11->getDeviceContext()->PSGetShader(&m_prevPS, NULL, NULL);
		dx11->getDeviceContext()->VSGetConstantBuffers(0, (UINT)m_prevConstantBuffers.size(), &m_prevConstantBuffers[0]);
	}

	dx11->getDeviceContext()->IASetInputLayout(m_inputLayout);
	dx11->getDeviceContext()->VSSetShader(m_vs, NULL, 0);
	dx11->getDeviceContext()->PSSetShader(m_ps, NULL, 0);

	dx11->getDeviceContext()->VSSetConstantBuffers(0, (UINT)m_constantBuffers.size(), &m_constantBuffers[0]);

	dx11->setActiveShader(this);
}

void DirectX11Shader::disable()
{
	DirectX11Interface *dx11 = (DirectX11Interface*)engine->getGraphics();
	if (!m_bReady || dx11->getActiveShader() != this) return;

	// restore
	// HACKHACK: slow af
	{
		UINT numPrevConstantBuffers = 0;
		for (size_t i=0; i<m_prevConstantBuffers.size(); i++)
		{
			if (m_prevConstantBuffers[i] != NULL)
				numPrevConstantBuffers++;
			else
				break;
		}

		dx11->getDeviceContext()->IASetInputLayout(m_prevInputLayout);
		dx11->getDeviceContext()->VSSetShader(m_prevVS, NULL, 0);
		dx11->getDeviceContext()->PSSetShader(m_prevPS, NULL, 0);
		dx11->getDeviceContext()->VSSetConstantBuffers(0, numPrevConstantBuffers, &m_prevConstantBuffers[0]);

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

			for (size_t i=0; i<m_prevConstantBuffers.size(); i++)
			{
				if (m_prevConstantBuffers[i] != NULL)
				{
					m_prevConstantBuffers[i]->Release();
					m_prevConstantBuffers[i] = NULL;
				}
			}
		}

		dx11->setActiveShader(m_prevShader);
	}
}

void DirectX11Shader::setUniform1f(UString name, float value)
{
	setUniform(name, &value, sizeof(float) * 1);
}

void DirectX11Shader::setUniform1fv(UString name, int count, float *values)
{
	setUniform(name, values, sizeof(float) * 1 * count);
}

void DirectX11Shader::setUniform1i(UString name, int value)
{
	setUniform(name, &value, sizeof(int32_t) * 1);
}

void DirectX11Shader::setUniform2f(UString name, float x, float y)
{
	Vector2 f(x, y);
	setUniform(name, &f[0], sizeof(float) * 2);
}

void DirectX11Shader::setUniform2fv(UString name, int count, float *vectors)
{
	setUniform(name, vectors, sizeof(float) * 2 * count);
}

void DirectX11Shader::setUniform3f(UString name, float x, float y, float z)
{
	Vector3 f(x, y, z);
	setUniform(name, &f[0], sizeof(float) * 3);
}

void DirectX11Shader::setUniform3fv(UString name, int count, float *vectors)
{
	setUniform(name, vectors, sizeof(float) * 3 * count);
}

void DirectX11Shader::setUniform4f(UString name, float x, float y, float z, float w)
{
	Vector4 f(x, y, z, w);
	setUniform(name, &f[0], sizeof(float) * 4);
}

void DirectX11Shader::setUniformMatrix4fv(UString name, Matrix4 &matrix)
{
	setUniformMatrix4fv(name, (float*)matrix.getTranspose());
}

void DirectX11Shader::setUniformMatrix4fv(UString name, float *v)
{
	setUniform(name, v, sizeof(float) * 4 * 4);
}

void DirectX11Shader::setUniform(const UString &name, void *src, size_t numBytes)
{
	if (!m_bReady) return;

	const CACHE_ENTRY cacheEntry = getAndCacheUniformLocation(name);
	if (cacheEntry.bindIndex > -1)
	{
		BIND_DESC &bindDesc = m_bindDescs[cacheEntry.bindIndex];

		if (memcmp(src, &bindDesc.floats[cacheEntry.offsetBytes / sizeof(float)], numBytes) != 0) // NOTE: ignore redundant updates
		{
			memcpy(&bindDesc.floats[cacheEntry.offsetBytes / sizeof(float)], src, numBytes);

			// NOTE: uniforms will be lazy updated later in onJustBeforeDraw() below
			// NOTE: this way we concatenate multiple uniform updates into one single gpu memory transfer
			m_bConstantBuffersUpToDate = false;
		}
	}
}

void DirectX11Shader::onJustBeforeDraw()
{
	if (!m_bReady) return;

	// lazy update uniforms
	if (!m_bConstantBuffersUpToDate)
	{
		DirectX11Interface *dx11 = (DirectX11Interface*)engine->getGraphics();

		for (size_t i=0; i<m_constantBuffers.size(); i++)
		{
			ID3D11Buffer *constantBuffer = m_constantBuffers[i];
			BIND_DESC &bindDesc = m_bindDescs[i];

			// lock
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			if (FAILED(dx11->getDeviceContext()->Map(constantBuffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
			{
				debugLog("ERROR: DirectX11Shader::setUniform1f() failed to Map()!\n");
				continue;
			}

			// write
			memcpy(mappedResource.pData, &bindDesc.floats[0], bindDesc.floats.size() * sizeof(float));

			// unlock
			dx11->getDeviceContext()->Unmap(constantBuffer, 0);

			// stats
			{
				if (engine->getFrameCount() == m_iStatsNumConstantBufferUploadsPerFrameCounterEngineFrameCount)
					m_iStatsNumConstantBufferUploadsPerFrameCounter++;
				else
				{
					m_iStatsNumConstantBufferUploadsPerFrameCounterEngineFrameCount = engine->getFrameCount();
					m_iStatsNumConstantBufferUploadsPerFrameCounter = 1;
				}
			}
		}

		m_bConstantBuffersUpToDate = true;
	}
}

const DirectX11Shader::CACHE_ENTRY DirectX11Shader::getAndCacheUniformLocation(const UString &name)
{
	if (!m_bReady) return invalidCacheEntry;

	m_sTempStringBuffer.reserve(name.lengthUtf8());
	m_sTempStringBuffer.assign(name.toUtf8(), name.lengthUtf8());

	const auto cachedValue = m_uniformLocationCache.find(m_sTempStringBuffer);
	const bool isCached = (cachedValue != m_uniformLocationCache.end());

	if (isCached)
		return cachedValue->second;
	else
	{
		CACHE_ENTRY newCacheEntry;
		{
			newCacheEntry.bindIndex = -1;
			newCacheEntry.offsetBytes = -1;

			for (size_t i=0; i<m_bindDescs.size(); i++)
			{
				const BIND_DESC &bindDesc = m_bindDescs[i];
				int offsetBytesCounter = 0;
				for (size_t j=0; j<bindDesc.lines.size(); j++)
				{
					const BIND_DESC_LINE &bindDescLine = bindDesc.lines[j];
					if (bindDescLine.variableName == name)
					{
						newCacheEntry.bindIndex = (int)i;
						newCacheEntry.offsetBytes = offsetBytesCounter;
						break;
					}
					else
						offsetBytesCounter += bindDescLine.variableBytes;
				}
			}

			if (newCacheEntry.bindIndex > -1 && newCacheEntry.offsetBytes > -1)
			{
				m_uniformLocationCache[m_sTempStringBuffer] = newCacheEntry;
				return newCacheEntry;
			}
			else if (debug_shaders->getBool())
				debugLog("DirectX11Shader Warning: Can't find uniform %s\n", name.toUtf8());
		}
	}

	return invalidCacheEntry;
}

bool DirectX11Shader::compile(UString vertexShader, UString fragmentShader)
{
	if (vertexShader.length() < 1 || fragmentShader.length() < 1) return false;

	DirectX11Interface *dx11 = (DirectX11Interface*)engine->getGraphics();

	const char *vsProfile = (dx11->getDevice()->GetFeatureLevel() >= D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0 ? "vs_5_0" : "vs_4_0");
	const char *psProfile = (dx11->getDevice()->GetFeatureLevel() >= D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0 ? "ps_5_0" : "ps_4_0");
	const char *vsEntryPoint = "vsmain";
	const char *psEntryPoint = "psmain";

	UINT flags = 0;

#ifdef MCENGINE_D3D11_CREATE_SHADER_DEBUG

	flags |= D3DCOMPILE_DEBUG;

#endif

	if (debug_shaders->getBool())
		flags |= D3DCOMPILE_DEBUG;

	const D3D_SHADER_MACRO defines[] =
	{
		"EXAMPLE_DEFINE", "1",
		NULL, NULL // sentinel
	};

	ID3D10Blob *vs = NULL;
	ID3D10Blob *ps = NULL;
	ID3D10Blob *vsError = NULL;
	ID3D10Blob *psError = NULL;

	// compile
	debugLog("DirectX11Shader: D3DCompile()-ing vertex shader ...\n");
	HRESULT hr1 = D3DCompile(vertexShader.toUtf8(), vertexShader.length(), "VS", defines, NULL, vsEntryPoint, vsProfile, flags, 0, &vs, &vsError);

	debugLog("DirectX11Shader: D3DCompile()-ing pixel shader ...\n");
	HRESULT hr2 = D3DCompile(fragmentShader.toUtf8(), fragmentShader.length(), "PS", defines, NULL, psEntryPoint, psProfile, flags, 0, &ps, &psError);

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

		engine->showMessageError("DirectX11Shader Error", "Couldn't D3DCompile()! Check the log for details.");

		return false;
	}

	// encapsulate
	debugLog("DirectX11Shader: CreateVertexShader(%i) ...\n", vs->GetBufferSize());
	hr1 = dx11->getDevice()->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), NULL, &m_vs);
	// NOTE: vs is not released here, since it's still needed for the input layout (1/2)

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
	std::vector<D3D11_INPUT_ELEMENT_DESC> elements;
	{
		const INPUT_DESC &inputDesc = m_inputDescs[0];
		UINT alignedByteOffsetCounter = 0;
		for (const INPUT_DESC_LINE &inputDescLine : inputDesc.lines)
		{
			D3D11_INPUT_ELEMENT_DESC element;
			{
				element.SemanticName = inputDescLine.dataType.toUtf8(); // WARNING: this is only safe because the UString is directly referenced (and kept alive) from the persistent m_inputDescs vector
				element.SemanticIndex = 0;
				element.Format = inputDescLine.dxgiFormat;
				element.InputSlot = 0;
				element.AlignedByteOffset = alignedByteOffsetCounter;
				element.InputSlotClass = inputDescLine.classification;
				element.InstanceDataStepRate = 0;

				if (debug_shaders->getBool())
					debugLog("%s, %i, %i, %i, %i, %i, %i\n", element.SemanticName, (int)element.SemanticIndex, (int)element.Format, (int)element.InputSlot, (int)element.AlignedByteOffset, (int)element.InputSlotClass, (int)element.InstanceDataStepRate);
			}
			elements.push_back(element);

			alignedByteOffsetCounter += inputDescLine.dxgiFormatBytes;
		}
	}
	hr1 = dx11->getDevice()->CreateInputLayout(&elements[0], elements.size(), vs->GetBufferPointer(), vs->GetBufferSize(), &m_inputLayout);
	vs->Release(); // NOTE: now we can release it (2/2)

	if (FAILED(hr1))
	{
		engine->showMessageError("DirectX11Shader Error", "Couldn't CreateInputLayout()!");
		return false;
	}

	// create binds/buffers
	for (const BIND_DESC &bindDesc : m_bindDescs)
	{
		const UString &descType = bindDesc.lines[0].type;

		if (descType == "D3D11_BUFFER_DESC")
		{
			const D3D11_BIND_FLAG bindFlag = bindDesc.lines[0].bindFlag;
			const UString &name = bindDesc.lines[0].name;

			UINT byteWidth = 0;
			for (const BIND_DESC_LINE &bindDescLine : bindDesc.lines)
			{
				byteWidth += bindDescLine.variableBytes;
			}

			// NOTE: "If the bind flag is D3D11_BIND_CONSTANT_BUFFER, you must set the ByteWidth value in multiples of 16, and less than or equal to D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT."
			if (byteWidth % 16 != 0)
			{
				engine->showMessageError("DirectX11Shader Error", UString::format("Invalid byteWidth %i for \"%s\" (must be a multiple of 16)", (int)byteWidth, name.toUtf8()));
				return false;
			}

			D3D11_BUFFER_DESC bufferDesc;
			{
				bufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
				bufferDesc.ByteWidth = byteWidth;
				bufferDesc.BindFlags = bindFlag;
				bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
				bufferDesc.MiscFlags = 0;
				bufferDesc.StructureByteStride = 0;
			}
			ID3D11Buffer *buffer = NULL;
			hr1 = dx11->getDevice()->CreateBuffer(&bufferDesc, NULL, &buffer);
			if (FAILED(hr1) || buffer == NULL)
			{
				engine->showMessageError("DirectX11Shader Error", UString::format("Couldn't CreateBuffer(%ld, %x, %x)!", hr1, hr1, MAKE_DXGI_HRESULT(hr1)));
				return false;
			}

			m_constantBuffers.push_back(buffer);
		}
		else
		{
			engine->showMessageError("DirectX11Shader Error", UString::format("Invalid descType \"%s\"", descType.toUtf8()));
			return false; // (unsupported desc/type)
		}
	}

	return true;
}

#endif

