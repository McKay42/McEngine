//================ Copyright (c) 2019, PG, All rights reserved. =================//
//
// Purpose:		OpenGLES2 GLSL implementation of Shader
//
// $NoKeywords: $gles2shader
//===============================================================================//

#include "OpenGLES2Shader.h"

#ifdef MCENGINE_FEATURE_OPENGLES

#include "Engine.h"
#include "ConVar.h"

#include "OpenGLHeaders.h"

OpenGLES2Shader::OpenGLES2Shader(UString shader, bool source)
{
	SHADER_PARSE_RESULT parsedVertexShader = parseShaderFromFileOrString("OpenGLES2Interface::VertexShader", shader, source);
	SHADER_PARSE_RESULT parsedFragmentShader = parseShaderFromFileOrString("OpenGLES2Interface::FragmentShader", shader, source);

	m_sVsh = parsedVertexShader.source;
	m_sFsh = parsedFragmentShader.source;
	m_bSource = true;

	m_iProgram = 0;
	m_iVertexShader = 0;
	m_iFragmentShader = 0;

	m_iProgramBackup = 0;
}

OpenGLES2Shader::OpenGLES2Shader(UString vertexShader, UString fragmentShader, bool source) : Shader()
{
	m_sVsh = vertexShader;
	m_sFsh = fragmentShader;
	m_bSource = source;

	m_iProgram = 0;
	m_iVertexShader = 0;
	m_iFragmentShader = 0;

	m_iProgramBackup = 0;
}

void OpenGLES2Shader::init()
{
	m_bReady = compile(m_sVsh, m_sFsh, m_bSource);
}

void OpenGLES2Shader::initAsync()
{
	m_bAsyncReady = true;
}

void OpenGLES2Shader::destroy()
{
	if (m_iProgram != 0)
		glDeleteProgram(m_iProgram);
	if (m_iFragmentShader != 0)
		glDeleteShader(m_iFragmentShader);
	if (m_iVertexShader != 0)
		glDeleteShader(m_iVertexShader);

	m_iProgram = 0;
	m_iFragmentShader = 0;
	m_iVertexShader = 0;

	m_iProgramBackup = 0;
}

void OpenGLES2Shader::enable()
{
	if (!m_bReady) return;

	glGetIntegerv(GL_CURRENT_PROGRAM, &m_iProgramBackup); // backup
	glUseProgram(m_iProgram);
}

void OpenGLES2Shader::disable()
{
	if (!m_bReady) return;

	glUseProgram(m_iProgramBackup); // restore
}

void OpenGLES2Shader::setUniform1f(UString name, float value)
{
	if (!m_bReady) return;
	int id = glGetUniformLocation(m_iProgram, name.toUtf8());
	if (id != -1)
		glUniform1f(id, value);
	else if (debug_shaders->getBool())
		debugLog("OpenGLES2Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void OpenGLES2Shader::setUniform1fv(UString name, int count, float *values)
{
	if (!m_bReady) return;
	int id = glGetUniformLocation(m_iProgram, name.toUtf8());
	if (id != -1)
		glUniform1fv(id, count, values);
	else if (debug_shaders->getBool())
		debugLog("OpenGLES2Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void OpenGLES2Shader::setUniform1i(UString name, int value)
{
	if (!m_bReady) return;
	int id = glGetUniformLocation(m_iProgram, name.toUtf8());
	if (id != -1)
		glUniform1i(id, value);
	else if (debug_shaders->getBool())
		debugLog("OpenGLES2Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void OpenGLES2Shader::setUniform2f(UString name, float value1, float value2)
{
	if (!m_bReady) return;
	int id = glGetUniformLocation(m_iProgram, name.toUtf8());
	if (id != -1)
		glUniform2f(id, value1, value2);
	else if (debug_shaders->getBool())
		debugLog("OpenGLES2Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void OpenGLES2Shader::setUniform2fv(UString name, int count, float *vectors)
{
	if (!m_bReady) return;
	int id = glGetUniformLocation(m_iProgram, name.toUtf8());
	if (id != -1)
		glUniform2fv(id, count, (float*)&vectors[0]);
	else if (debug_shaders->getBool())
		debugLog("OpenGLES2Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void OpenGLES2Shader::setUniform3f(UString name, float x, float y, float z)
{
	if (!m_bReady) return;
	int id = glGetUniformLocation(m_iProgram, name.toUtf8());
	if (id != -1)
		glUniform3f(id, x, y, z);
	else if (debug_shaders->getBool())
		debugLog("OpenGLES2Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void OpenGLES2Shader::setUniform3fv(UString name, int count, float *vectors)
{
	if (!m_bReady) return;
	int id = glGetUniformLocation(m_iProgram, name.toUtf8());
	if (id != -1)
		glUniform3fv(id, count, (float*)&vectors[0]);
	else if (debug_shaders->getBool())
		debugLog("OpenGLES2Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void OpenGLES2Shader::setUniform4f(UString name, float x, float y, float z, float w)
{
	if (!m_bReady) return;
	int id = glGetUniformLocation(m_iProgram, name.toUtf8());
	if (id != -1)
		glUniform4f(id, x, y, z, w);
	else if (debug_shaders->getBool())
		debugLog("OpenGLES2Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void OpenGLES2Shader::setUniformMatrix4fv(UString name, Matrix4 &matrix)
{
	if (!m_bReady) return;
	int id = glGetUniformLocation(m_iProgram, name.toUtf8());
	if (id != -1)
		glUniformMatrix4fv(id, 1, GL_FALSE, matrix.get());
	else if (debug_shaders->getBool())
		debugLog("OpenGLES2Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

void OpenGLES2Shader::setUniformMatrix4fv(UString name, float *v)
{
	if (!m_bReady) return;
	int id = glGetUniformLocation(m_iProgram, name.toUtf8());
	if (id != -1)
		glUniformMatrix4fv(id, 1, GL_FALSE, v);
	else if (debug_shaders->getBool())
		debugLog("OpenGLES2Shader Warning: Can't find uniform %s\n",name.toUtf8());
}

int OpenGLES2Shader::getAttribLocation(UString name)
{
	if (!m_bReady) return -1;
	return glGetAttribLocation(m_iProgram, name.toUtf8());
}

bool OpenGLES2Shader::isActive()
{
	int currentProgram = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
	return (m_bReady && currentProgram == m_iProgram);
}

bool OpenGLES2Shader::compile(UString vertexShader, UString fragmentShader, bool source)
{
	// load & compile shaders
	debugLog("OpenGLES2Shader: Compiling %s ...\n", (source ? "vertex source" : vertexShader.toUtf8()));
	m_iVertexShader = source ? createShaderFromString(vertexShader, GL_VERTEX_SHADER) : createShaderFromFile(vertexShader, GL_VERTEX_SHADER);
	debugLog("OpenGLES2Shader: Compiling %s ...\n", (source ? "fragment source" : fragmentShader.toUtf8()));
	m_iFragmentShader = source ? createShaderFromString(fragmentShader, GL_FRAGMENT_SHADER) : createShaderFromFile(fragmentShader, GL_FRAGMENT_SHADER);

	if (m_iVertexShader == 0 || m_iFragmentShader == 0)
	{
		engine->showMessageError("OpenGLES2Shader Error", "Couldn't createShader()");
		return false;
	}

	// create program
	m_iProgram = (int)glCreateProgram();
	if (m_iProgram == 0)
	{
		engine->showMessageError("OpenGLES2Shader Error", "Couldn't glCreateProgram()");
		return false;
	}

	// attach
	glAttachShader(m_iProgram, m_iVertexShader);
	glAttachShader(m_iProgram, m_iFragmentShader);

	// link
	glLinkProgram(m_iProgram);

	GLint ret = GL_FALSE;
	glGetProgramiv(m_iProgram, GL_LINK_STATUS, &ret);
	if (ret == GL_FALSE)
	{
		engine->showMessageError("OpenGLES2Shader Error", "Couldn't glLinkProgram()");
		return false;
	}

	// validate
	ret = GL_FALSE;
	glValidateProgram(m_iProgram);
	glGetProgramiv(m_iProgram, GL_VALIDATE_STATUS, &ret);
	if (ret == GL_FALSE)
	{
		engine->showMessageError("OpenGLES2Shader Error", "Couldn't glValidateProgram()");
		return false;
	}

	return true;
}

int OpenGLES2Shader::createShaderFromString(UString shaderSource, int shaderType)
{
	const GLint shader = glCreateShader(shaderType);

	if (shader == 0)
	{
		engine->showMessageError("OpenGLES2Shader Error", "Couldn't glCreateShader()");
		return 0;
	}

	// compile shader
	const char *shaderSourceChar = shaderSource.toUtf8();
	glShaderSource(shader, 1, &shaderSourceChar, NULL);
	glCompileShader(shader);

	GLint ret = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ret);
	if (ret == GL_FALSE)
	{
		debugLog("------------------OpenGLES2Shader Compile Error------------------\n");

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &ret);
		char *errorLog = new char[ret];
		{
			glGetShaderInfoLog(shader, ret, &ret, errorLog);
			debugLog(errorLog);
		}
		delete[] errorLog;

		debugLog("-----------------------------------------------------------------\n");

		engine->showMessageError("OpenGLES2Shader Error", "Couldn't glShaderSource() or glCompileShader()");
		return 0;
	}

	return shader;
}

int OpenGLES2Shader::createShaderFromFile(UString fileName, int shaderType)
{
	// load file
	std::ifstream inFile(fileName.toUtf8());
	if (!inFile)
	{
		engine->showMessageError("OpenGLES2Shader Error", fileName);
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
