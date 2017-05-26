//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		software rasterizer implementation of Shader
//
// $NoKeywords: $swshader
//===============================================================================//

#ifndef SWSHADER_H
#define SWSHADER_H

#include "Shader.h"

class SWShader : public Shader
{
public:
	SWShader(UString vertexShader, UString fragmentShader, bool source);
	virtual ~SWShader() {destroy();}

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

private:
	virtual void init();
	virtual void initAsync();
	virtual void destroy();
};

#endif
