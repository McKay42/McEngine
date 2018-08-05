//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		DirectX HLSL implementation of Shader
//
// $NoKeywords: $dxshader
//===============================================================================//

#ifndef DIRECTX11SHADER_H
#define DIRECTX11SHADER_H

#include "Shader.h"

#ifdef MCENGINE_FEATURE_DIRECTX

#include "d3d11.h"

class DirectX11Shader : public Shader
{
public:
	DirectX11Shader(UString vertexShader, UString fragmentShader, bool source);
	virtual ~DirectX11Shader() {destroy();}

	virtual void enable();
	virtual void disable();

	// TODO:
	virtual void setUniform1f(UString name, float value);
	virtual void setUniform1fv(UString name, int count, float *values) {;}
	virtual void setUniform1i(UString name, int value) {;}
	virtual void setUniform2f(UString name, float x, float y) {;}
	virtual void setUniform2fv(UString name, int count, float *vectors) {;}
	virtual void setUniform3f(UString name, float x, float y, float z) {;}
	virtual void setUniform3fv(UString name, int count, float *vectors) {;}

	virtual void setUniform4f(UString name, float x, float y, float z, float w);
	virtual void setUniformMatrix4fv(UString name, Matrix4 &matrix);
	virtual void setUniformMatrix4fv(UString name, float *v);

	ID3D11VertexShader *getVertexShader() const {return m_vs;}
	ID3D11PixelShader *getPixelShader() const {return m_ps;}
	ID3D11InputLayout *getInputLayout() const {return m_inputLayout;}

private:
	// TODO: extract this into public functions
	// NOTE: "If the bind flag is D3D11_BIND_CONSTANT_BUFFER, you must set the ByteWidth value in multiples of 16, and less than or equal to D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT."
	struct MatrixBufferType
	{
		float mvp[4*4];
		float col[4];
		float misc[4]; // [0] = textured
	};

	virtual void init();
	virtual void initAsync();
	virtual void destroy();

	bool compile(UString vertexShader, UString fragmentShader, bool source);

	UString m_sVsh, m_sFsh;

	bool m_bSource;

	ID3D11VertexShader *m_vs;
	ID3D11PixelShader *m_ps;
	ID3D11InputLayout *m_inputLayout;
	ID3D11Buffer *m_constantBuffer;

	ID3D11VertexShader *m_prevVS;
	ID3D11PixelShader *m_prevPS;
	ID3D11InputLayout *m_prevInputLayout;

	MatrixBufferType m_constants;
};

#endif

#endif
