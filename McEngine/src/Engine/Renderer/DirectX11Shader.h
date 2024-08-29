//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		DirectX HLSL implementation of Shader
//
// $NoKeywords: $dxshader
//===============================================================================//

#ifndef DIRECTX11SHADER_H
#define DIRECTX11SHADER_H

#include "Shader.h"

#ifdef MCENGINE_FEATURE_DIRECTX11

#include "d3d11.h"

class DirectX11Shader : public Shader
{
public:
	DirectX11Shader(UString shader, bool source);
	DirectX11Shader(UString vertexShader, UString fragmentShader, bool source); // DEPRECATED
	virtual ~DirectX11Shader() {destroy();}

	virtual void enable();
	virtual void disable();

	virtual void setUniform1f(UString name, float value);
	virtual void setUniform1fv(UString name, int count, float *values);
	virtual void setUniform1i(UString name, int value);
	virtual void setUniform2f(UString name, float x, float y);
	virtual void setUniform2fv(UString name, int count, float *vectors);
	virtual void setUniform3f(UString name, float x, float y, float z);
	virtual void setUniform3fv(UString name, int count, float *vectors);
	virtual void setUniform4f(UString name, float x, float y, float z, float w);
	virtual void setUniformMatrix4fv(UString name, Matrix4 &matrix);
	virtual void setUniformMatrix4fv(UString name, float *v);

	// ILLEGAL:
	void onJustBeforeDraw();
	inline unsigned long getStatsNumConstantBufferUploadsPerFrame() const {return m_iStatsNumConstantBufferUploadsPerFrameCounter;}
	inline unsigned long getStatsNumConstantBufferUploadsPerFrameEngineFrameCount() const {return m_iStatsNumConstantBufferUploadsPerFrameCounterEngineFrameCount;}

private:
	struct INPUT_DESC_LINE
	{
		UString type;								// e.g. "VS_INPUT"
		UString dataType;							// e.g. "POSITION", "COLOR0", "TEXCOORD0", etc.
		DXGI_FORMAT dxgiFormat;						// e.g. DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, etc.
		int dxgiFormatBytes;						// e.g. "DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT" -> 12, etc.
		D3D11_INPUT_CLASSIFICATION classification;	// e.g. D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_INSTANCE_DATA
	};

	struct BIND_DESC_LINE
	{
		UString type;				// e.g. "D3D11_BUFFER_DESC"
		D3D11_BIND_FLAG bindFlag;	// e.g. D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER
		UString name;				// e.g. "ModelViewProjectionConstantBuffer"
		UString variableName;		// e.g. "mvp", "col", "misc", etc.
		UString variableType;		// e.g. "float4x4", "float4", "float3", "float2", "float", etc.
		int variableBytes;			// e.g. 16 -> "float4x4", 4 -> "float4", 3 -> "float3, 2 -> "float2", 1 -> "float", etc.
	};

	struct INPUT_DESC
	{
		UString type; // INPUT_DESC_LINE::type
		std::vector<INPUT_DESC_LINE> lines;
	};

	struct BIND_DESC
	{
		UString name; // BIND_DESC_LINE::name
		std::vector<BIND_DESC_LINE> lines;
		std::vector<float> floats;
	};

private:
	struct CACHE_ENTRY
	{
		int bindIndex; // into m_bindDescs[bindIndex] and m_constantBuffers[bindIndex]
		int offsetBytes;
	};

protected:
	virtual void init();
	virtual void initAsync();
	virtual void destroy();

	bool compile(UString vertexShader, UString fragmentShader);

	void setUniform(const UString &name, void *src, size_t numBytes);

	const CACHE_ENTRY getAndCacheUniformLocation(const UString &name);

private:
	static CACHE_ENTRY invalidCacheEntry;

	UString m_sShader;
	UString m_sVsh, m_sFsh;

	bool m_bSource;

	ID3D11VertexShader *m_vs;
	ID3D11PixelShader *m_ps;
	ID3D11InputLayout *m_inputLayout;
	std::vector<ID3D11Buffer*> m_constantBuffers;
	bool m_bConstantBuffersUpToDate;

	DirectX11Shader *m_prevShader;
	ID3D11VertexShader *m_prevVS;
	ID3D11PixelShader *m_prevPS;
	ID3D11InputLayout *m_prevInputLayout;
	std::vector<ID3D11Buffer*> m_prevConstantBuffers;

	std::vector<INPUT_DESC> m_inputDescs;
	std::vector<BIND_DESC> m_bindDescs;

	std::unordered_map<std::string, CACHE_ENTRY> m_uniformLocationCache;
	std::string m_sTempStringBuffer;

	// stats
	unsigned long m_iStatsNumConstantBufferUploadsPerFrameCounter;
	unsigned long m_iStatsNumConstantBufferUploadsPerFrameCounterEngineFrameCount;
};

#endif

#endif
