//================ Copyright (c) 2012, PG, All rights reserved. =================//
//
// Purpose:		shader wrapper
//
// $NoKeywords: $shader
//===============================================================================//

#ifndef SHADER_H
#define SHADER_H

#include "Resource.h"

class ConVar;

class Shader : public Resource
{
public:
	Shader() : Resource() {;}
	virtual ~Shader() {;}

	virtual void enable() = 0;
	virtual void disable() = 0;

	virtual void setUniform1f(UString name, float value) = 0;
	virtual void setUniform1fv(UString name, int count, float *values) = 0;
	virtual void setUniform1i(UString name, int value) = 0;
	virtual void setUniform2f(UString name, float x, float y) = 0;
	virtual void setUniform2fv(UString name, int count, float *vectors) = 0;
	virtual void setUniform3f(UString name, float x, float y, float z) = 0;
	virtual void setUniform3fv(UString name, int count, float *vectors) = 0;
	virtual void setUniform4f(UString name, float x, float y, float z, float w) = 0;
	virtual void setUniformMatrix4fv(UString name, Matrix4 &matrix) = 0;
	virtual void setUniformMatrix4fv(UString name, float *v) = 0;

protected:
	static ConVar *debug_shaders;

	virtual void init() = 0;
	virtual void initAsync() = 0;
	virtual void destroy() = 0;
};

#endif
