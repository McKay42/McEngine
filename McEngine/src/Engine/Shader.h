//================ Copyright (c) 2012, PG, All rights reserved. =================//
//
// Purpose:		GLSL shader wrapper
//
// $NoKeywords: $shader
//===============================================================================//

#ifndef SHADER_H
#define SHADER_H

#include "cbase.h"

class Shader
{
public:
	Shader(UString vertexShaderFileName, UString fragmentShaderFileName);
	Shader(UString vertexShaderSource, UString fragmentShaderSource, bool source);
	~Shader();

	void free();

	void enable();
	void disable();

	void setUniform1f(UString name, float value);
	void setUniform1fv(UString name, int count, float *values);
	void setUniform1i(UString name, int value);
	void setUniform2f(UString name, float x, float y);
	void setUniform3f(UString name, float x, float y, float z);
	void setUniform3fv(UString name, int count, float *vectors);
	void setUniformMatrix4fv(UString name, Matrix4 &matrix);
	void setUniformMatrix4fv(UString name, float *v);

	void recompile();

	inline bool isReady() const {return m_bReady;}

private:
	void compile(UString vertexShader, UString fragmentShader, bool source);
	int createShaderFromFile(UString fileName, int shaderType);
	int createShaderFromString(UString shaderSource, int shaderType);

	UString m_sVsh, m_sFsh;

	bool m_bReady;

	bool m_bSource;
	int m_iVertexShader;
	int m_iFragmentShader;
	int m_iProgram;

	std::unordered_map<std::string, int> m_vUniformCache;
};

#endif
