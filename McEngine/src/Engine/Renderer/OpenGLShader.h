//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		OpenGL GLSL implementation of Shader
//
// $NoKeywords: $glshader
//===============================================================================//

#ifndef OPENGLSHADER_H
#define OPENGLSHADER_H

#include "Shader.h"

#ifdef MCENGINE_FEATURE_OPENGL

class OpenGLShader : public Shader
{
public:
	OpenGLShader(UString shader, bool source);
	OpenGLShader(UString vertexShader, UString fragmentShader, bool source); // DEPRECATED
	virtual ~OpenGLShader() {destroy();}

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
	int getAttribLocation(UString name);
	int getAndCacheUniformLocation(const UString &name);

private:
	virtual void init();
	virtual void initAsync();
	virtual void destroy();

private:
	bool compile(UString vertexShader, UString fragmentShader, bool source);
	int createShaderFromString(UString shaderSource, int shaderType);
	int createShaderFromFile(UString fileName, int shaderType);

private:
	bool m_bIsShader2;

	UString m_sVsh;
	UString m_sFsh;

	bool m_bSource;
	int m_iVertexShader;
	int m_iFragmentShader;
	int m_iProgram;

	int m_iProgramBackup;

	std::unordered_map<std::string, int> m_uniformLocationCache;
	std::string m_sTempStringBuffer;
};

#endif

#endif
