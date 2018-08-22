//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		OpenGL baking support for vao
//
// $NoKeywords: $glvao
//===============================================================================//

#ifndef OPENGLVERTEXARRAYOBJECT_H
#define OPENGLVERTEXARRAYOBJECT_H

#include "VertexArrayObject.h"

#ifdef MCENGINE_FEATURE_OPENGL

class OpenGLVertexArrayObject : public VertexArrayObject
{
public:
	OpenGLVertexArrayObject(Graphics::PRIMITIVE primitive = Graphics::PRIMITIVE::PRIMITIVE_TRIANGLES, Graphics::USAGE_TYPE usage = Graphics::USAGE_TYPE::USAGE_STATIC, bool keepInSystemMemory = false);
	virtual ~OpenGLVertexArrayObject() {destroy();}

	void draw();

private:
	static int primitiveToOpenGL(Graphics::PRIMITIVE primitive);
	static unsigned int usageToOpenGL(Graphics::USAGE_TYPE usage);

	virtual void init();
	virtual void initAsync();
	virtual void destroy();

	unsigned int m_iVertexBuffer;
	unsigned int m_iTexcoordBuffer;

	unsigned int m_iNumTexcoords;
};

#endif

#endif
