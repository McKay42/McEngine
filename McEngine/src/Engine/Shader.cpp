//================ Copyright (c) 2012, PG, All rights reserved. =================//
//
// Purpose:		GLSL shader wrapper
//
// $NoKeywords: $shader
//===============================================================================//

#include "Shader.h"
#include "Engine.h"
#include "ConVar.h"

#include "OpenGLHeaders.h"

#define SHADER_FOLDER "shaders/"

ConVar debug_shaders("debug_shaders", false);

Shader::Shader(UString vertexShaderFileName, UString fragmentShaderFileName)
{
	m_sVsh = vertexShaderFileName;
	m_sFsh = fragmentShaderFileName;
	m_bSource = false;

	m_iProgram = 0;
	m_iVertexShader = 0;
	m_iFragmentShader = 0;

	compile(m_sVsh, m_sFsh, false);
}

Shader::Shader(UString vertexShaderSource, UString fragmentShaderSource, bool source)
{
	m_sVsh = vertexShaderSource;
	m_sFsh = fragmentShaderSource;
	m_bSource = true;

	m_iProgram = 0;
	m_iVertexShader = 0;
	m_iFragmentShader = 0;

	compile(m_sVsh, m_sFsh, true);
}

Shader::~Shader()
{
	free();
}

void Shader::compile(UString vertexShader, UString fragmentShader, bool source)
{
	m_bReady = false;
	m_vUniformCache.clear();

	// load & compile shaders
	m_iVertexShader = source ? createShaderFromString(vertexShader, GL_VERTEX_SHADER_ARB) : createShaderFromFile(vertexShader, GL_VERTEX_SHADER_ARB);
	m_iFragmentShader = source ? createShaderFromString(fragmentShader, GL_FRAGMENT_SHADER_ARB) : createShaderFromFile(fragmentShader, GL_FRAGMENT_SHADER_ARB);

	if (m_iVertexShader == 0 || m_iFragmentShader == 0)
	{
		engine->showMessageError("Shader Error", "Couldn't createShader()");
		return;
	}

	// create program
	m_iProgram = glCreateProgramObjectARB();

	if (m_iProgram == 0)
	{
		engine->showMessageError("Shader Error", "Couldn't glCreateProgramObjectARB()");
		return;
	}

	// attach
	glAttachObjectARB(m_iProgram, m_iVertexShader);
	glAttachObjectARB(m_iProgram, m_iFragmentShader);

	// link
	glLinkProgramARB(m_iProgram);

	int returnValue = GL_TRUE;
	glGetObjectParameterivARB(m_iProgram, GL_OBJECT_LINK_STATUS_ARB, &returnValue);
	if (returnValue == GL_FALSE)
	{
		engine->showMessageError("Shader Error", "Couldn't glLinkProgramARB()");
		return;
	}

	// validate
	glValidateProgramARB(m_iProgram);
	returnValue = GL_TRUE;
	glGetObjectParameterivARB(m_iProgram, GL_OBJECT_VALIDATE_STATUS_ARB, &returnValue);
	if (returnValue == GL_FALSE)
	{
		engine->showMessageError("Shader Error", "Couldn't glValidateProgramARB()");
		return;
	}

	m_bReady = true;
}

void Shader::enable()
{
	if (!m_bReady) return;
	glUseProgramObjectARB(m_iProgram);
}

void Shader::disable()
{
	if (!m_bReady) return;
	glUseProgramObjectARB(0);
}

int Shader::createShaderFromFile(UString fileName, int shaderType)
{
	debugLog("Shader: Loading file %s\n", fileName.toUtf8());

	// load file
	fileName.insert(0,SHADER_FOLDER);
	std::ifstream inFile(fileName.toUtf8());
	if (!inFile)
	{
		engine->showMessageError("Shader Error", fileName);
		return 0;
	}
	std::string line;
	std::string shaderSource;
	int linecount = 0;
	while (inFile.good())
	{
		std::getline(inFile, line);
		shaderSource += line + "\n\0";
		linecount++;
	}
	shaderSource += "\n\0";
	inFile.close();

	UString shaderSourcePtr = UString(shaderSource.c_str());

	return createShaderFromString(shaderSourcePtr, shaderType);
}

int Shader::createShaderFromString(UString shaderSource, int shaderType)
{
	int shader = glCreateShaderObjectARB(shaderType);

	if (shader == 0)
	{
		engine->showMessageError("Shader Compile Error", "Couldn't glCreateShaderObjectARB()");
		return 0;
	}

	// compile shader
	const char* shaderSourceChar = shaderSource.toUtf8();
	glShaderSourceARB(shader, 1, &shaderSourceChar, NULL);
	glCompileShaderARB(shader);

	int returnValue = GL_TRUE;
	glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &returnValue);

	if (returnValue == GL_FALSE)
	{
		debugLog("------------------Shader Compile Error------------------\n");

		glGetObjectParameterivARB(shader, GL_OBJECT_INFO_LOG_LENGTH_ARB, &returnValue);
		char *errorLog = new char[returnValue];
		glGetInfoLogARB(shader, returnValue, &returnValue, errorLog);

		debugLog(errorLog);
		delete[] errorLog;

		debugLog("--------------------------------------------------------\n");

		engine->showMessageError("Shader Error", "Couldn't glShaderSourceARB() or glCompileShaderARB()");
		return 0;
	}

	return shader;
}

void Shader::free()
{
	m_bReady = false;

	if (m_iProgram != 0)
		glDeleteObjectARB(m_iProgram);
	if (m_iFragmentShader != 0)
		glDeleteObjectARB(m_iFragmentShader);
	if (m_iVertexShader != 0)
		glDeleteObjectARB(m_iVertexShader);
}

void Shader::recompile()
{
	free();
	compile(m_sVsh, m_sFsh, m_bSource);
}

void Shader::setUniform1f(UString name, float value)
{
	if (!m_bReady) return;

	// check cache first
	auto it = m_vUniformCache.find(name.toUtf8());
	if (it != m_vUniformCache.end())
	{
		glUniform1fARB(it->second, value);
		return;
	}

	// couldn't find in cache. get and store
	int id = glGetUniformLocationARB(m_iProgram, name.toUtf8());
	if (id != -1)
	{
		m_vUniformCache[name.toUtf8()] = id;
		glUniform1fARB(id, value);
	}
	else if (debug_shaders.getBool())
		debugLog("Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void Shader::setUniform1fv(UString name, int count, float *values)
{
	if (!m_bReady) return;

	// check cache first
	auto it = m_vUniformCache.find(name.toUtf8());
	if (it != m_vUniformCache.end())
	{
		glUniform1fvARB(it->second, count, values);
		return;
	}

	// couldn't find in cache. get and store
	int id = glGetUniformLocationARB(m_iProgram, name.toUtf8());
	if (id != -1)
	{
		m_vUniformCache[name.toUtf8()] = id;
		glUniform1fvARB(id, count, values);
	}
	else if (debug_shaders.getBool())
		debugLog("Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void Shader::setUniform1i(UString name, int value)
{
	if (!m_bReady) return;

	// check cache first
	auto it = m_vUniformCache.find(name.toUtf8());
	if (it != m_vUniformCache.end())
	{
		glUniform1iARB(it->second, value);
		return;
	}

	// couldn't find in cache. get and store
	int id = glGetUniformLocationARB(m_iProgram, name.toUtf8());
	if (id != -1)
	{
		m_vUniformCache[name.toUtf8()] = id;
		glUniform1iARB(id, value);
	}
	else if (debug_shaders.getBool())
		debugLog("Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void Shader::setUniform2f(UString name, float value1, float value2)
{
	if (!m_bReady) return;

	// check cache first
	auto it = m_vUniformCache.find(name.toUtf8());
	if (it != m_vUniformCache.end())
	{
		glUniform2fARB(it->second, value1, value2);
		return;
	}

	// couldn't find in cache. get and store
	int id = glGetUniformLocationARB(m_iProgram, name.toUtf8());
	if (id != -1)
	{
		m_vUniformCache[name.toUtf8()] = id;
		glUniform2fARB(id, value1, value2);
	}
	else if (debug_shaders.getBool())
		debugLog("Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void Shader::setUniform3f(UString name, float x, float y, float z)
{
	if (!m_bReady) return;

	// check cache first
	auto it = m_vUniformCache.find(name.toUtf8());
	if (it != m_vUniformCache.end())
	{
		glUniform3fARB(it->second, x, y, z);
		return;
	}

	// couldn't find in cache. get and store
	int id = glGetUniformLocationARB(m_iProgram, name.toUtf8());
	if (id != -1)
	{
		m_vUniformCache[name.toUtf8()] = id;
		glUniform3fARB(id, x, y, z);
	}
	else if (debug_shaders.getBool())
		debugLog("Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void Shader::setUniform3fv(UString name, int count, float *vectors)
{
	if (!m_bReady) return;
	int id = glGetUniformLocationARB(m_iProgram, name.toUtf8());
	if (id != -1)
		glUniform3fv(id, count, (float*)&vectors[0]);
	else if (debug_shaders.getBool())
		debugLog("Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void Shader::setUniformMatrix4fv(UString name, Matrix4 &matrix)
{
	if (!m_bReady) return;
	int id = glGetUniformLocationARB(m_iProgram, name.toUtf8());
	if (id != -1)
		glUniformMatrix4fv(id, 1, GL_FALSE, matrix.get());
	else if (debug_shaders.getBool())
		debugLog("Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void Shader::setUniformMatrix4fv(UString name, float *v)
{
	if (!m_bReady) return;
	int id = glGetUniformLocationARB(m_iProgram, name.toUtf8());
	if (id != -1)
		glUniformMatrix4fv(id, 1, GL_FALSE, v);
	else if (debug_shaders.getBool())
		debugLog("Shader Warning: Can't find uniform %s\n",name.toUtf8());
}
