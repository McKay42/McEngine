//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		OpenGL GLSL implementation of Shader
//
// $NoKeywords: $glshader
//===============================================================================//

#include "OpenGLShader.h"

#ifdef MCENGINE_FEATURE_OPENGL

#include "Engine.h"
#include "ConVar.h"

#include "OpenGLHeaders.h"

OpenGLShader::OpenGLShader(UString vertexShader, UString fragmentShader, bool source) : Shader()
{
	m_sVsh = vertexShader;
	m_sFsh = fragmentShader;
	m_bSource = source;

	m_iProgram = 0;
	m_iVertexShader = 0;
	m_iFragmentShader = 0;

	m_iProgramBackup = 0;
}

void OpenGLShader::init()
{
	m_bReady = compile(m_sVsh, m_sFsh, m_bSource);
}

void OpenGLShader::initAsync()
{
	m_bAsyncReady = true;
}

void OpenGLShader::destroy()
{
	if (m_iProgram != 0)
		glDeleteObjectARB(m_iProgram);
	if (m_iFragmentShader != 0)
		glDeleteObjectARB(m_iFragmentShader);
	if (m_iVertexShader != 0)
		glDeleteObjectARB(m_iVertexShader);

	m_iProgram = 0;
	m_iFragmentShader = 0;
	m_iVertexShader = 0;

	m_iProgramBackup = 0;
}

void OpenGLShader::enable()
{
	if (!m_bReady) return;

	glGetIntegerv(GL_CURRENT_PROGRAM, &m_iProgramBackup); // backup
	glUseProgramObjectARB(m_iProgram);
}

void OpenGLShader::disable()
{
	if (!m_bReady) return;

	glUseProgramObjectARB(m_iProgramBackup); // restore
}

void OpenGLShader::setUniform1f(UString name, float value)
{
	if (!m_bReady) return;
	int id = glGetUniformLocationARB(m_iProgram, name.toUtf8());
	if (id != -1)
		glUniform1fARB(id, value);
	else if (debug_shaders->getBool())
		debugLog("Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void OpenGLShader::setUniform1fv(UString name, int count, float *values)
{
	if (!m_bReady) return;
	int id = glGetUniformLocationARB(m_iProgram, name.toUtf8());
	if (id != -1)
		glUniform1fvARB(id, count, values);
	else if (debug_shaders->getBool())
		debugLog("Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void OpenGLShader::setUniform1i(UString name, int value)
{
	if (!m_bReady) return;
	int id = glGetUniformLocationARB(m_iProgram, name.toUtf8());
	if (id != -1)
		glUniform1iARB(id, value);
	else if (debug_shaders->getBool())
		debugLog("Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void OpenGLShader::setUniform2f(UString name, float value1, float value2)
{
	if (!m_bReady) return;
	int id = glGetUniformLocationARB(m_iProgram, name.toUtf8());
	if (id != -1)
		glUniform2fARB(id, value1, value2);
	else if (debug_shaders->getBool())
		debugLog("Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void OpenGLShader::setUniform2fv(UString name, int count, float *vectors)
{
	if (!m_bReady) return;
	int id = glGetUniformLocationARB(m_iProgram, name.toUtf8());
	if (id != -1)
		glUniform2fv(id, count, (float*)&vectors[0]);
	else if (debug_shaders->getBool())
		debugLog("Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void OpenGLShader::setUniform3f(UString name, float x, float y, float z)
{
	if (!m_bReady) return;
	int id = glGetUniformLocationARB(m_iProgram, name.toUtf8());
	if (id != -1)
		glUniform3fARB(id, x, y, z);
	else if (debug_shaders->getBool())
		debugLog("Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void OpenGLShader::setUniform3fv(UString name, int count, float *vectors)
{
	if (!m_bReady) return;
	int id = glGetUniformLocationARB(m_iProgram, name.toUtf8());
	if (id != -1)
		glUniform3fv(id, count, (float*)&vectors[0]);
	else if (debug_shaders->getBool())
		debugLog("Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void OpenGLShader::setUniform4f(UString name, float x, float y, float z, float w)
{
	if (!m_bReady) return;
	int id = glGetUniformLocationARB(m_iProgram, name.toUtf8());
	if (id != -1)
		glUniform4fARB(id, x, y, z, w);
	else if (debug_shaders->getBool())
		debugLog("Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void OpenGLShader::setUniformMatrix4fv(UString name, Matrix4 &matrix)
{
	if (!m_bReady) return;
	int id = glGetUniformLocationARB(m_iProgram, name.toUtf8());
	if (id != -1)
		glUniformMatrix4fv(id, 1, GL_FALSE, matrix.get());
	else if (debug_shaders->getBool())
		debugLog("Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void OpenGLShader::setUniformMatrix4fv(UString name, float *v)
{
	if (!m_bReady) return;
	int id = glGetUniformLocationARB(m_iProgram, name.toUtf8());
	if (id != -1)
		glUniformMatrix4fv(id, 1, GL_FALSE, v);
	else if (debug_shaders->getBool())
		debugLog("Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

int OpenGLShader::getAttribLocation(UString name)
{
	if (!m_bReady) return -1;
	return glGetAttribLocation(m_iProgram, name.toUtf8());
}

bool OpenGLShader::compile(UString vertexShader, UString fragmentShader, bool source)
{
	// load & compile shaders
	debugLog("Shader: Compiling %s ...\n", vertexShader.toUtf8());
	m_iVertexShader = source ? createShaderFromString(vertexShader, GL_VERTEX_SHADER_ARB) : createShaderFromFile(vertexShader, GL_VERTEX_SHADER_ARB);
	debugLog("Shader: Compiling %s ...\n", fragmentShader.toUtf8());
	m_iFragmentShader = source ? createShaderFromString(fragmentShader, GL_FRAGMENT_SHADER_ARB) : createShaderFromFile(fragmentShader, GL_FRAGMENT_SHADER_ARB);

	if (m_iVertexShader == 0 || m_iFragmentShader == 0)
	{
		engine->showMessageError("Shader Error", "Couldn't createShader()");
		return false;
	}

	// create program
	m_iProgram = glCreateProgramObjectARB();
	if (m_iProgram == 0)
	{
		engine->showMessageError("Shader Error", "Couldn't glCreateProgramObjectARB()");
		return false;
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
		return false;
	}

	// validate
	glValidateProgramARB(m_iProgram);
	returnValue = GL_TRUE;
	glGetObjectParameterivARB(m_iProgram, GL_OBJECT_VALIDATE_STATUS_ARB, &returnValue);
	if (returnValue == GL_FALSE)
	{
		engine->showMessageError("Shader Error", "Couldn't glValidateProgramARB()");
		return false;
	}

	return true;
}

int OpenGLShader::createShaderFromString(UString shaderSource, int shaderType)
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

int OpenGLShader::createShaderFromFile(UString fileName, int shaderType)
{
	// load file
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

#endif
