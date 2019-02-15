//================ Copyright (c) 2019, PG, All rights reserved. =================//
//
// Purpose:		OpenGLES2 baking support for vao
//
// $NoKeywords: $gles2vao
//===============================================================================//

#ifndef OPENGLES2VERTEXARRAYOBJECT_H
#define OPENGLES2VERTEXARRAYOBJECT_H

#include "VertexArrayObject.h"

#ifdef MCENGINE_FEATURE_OPENGLES

class OpenGLES2VertexArrayObject : public VertexArrayObject
{
public:
	OpenGLES2VertexArrayObject(Graphics::PRIMITIVE primitive = Graphics::PRIMITIVE::PRIMITIVE_TRIANGLES, Graphics::USAGE_TYPE usage = Graphics::USAGE_TYPE::USAGE_STATIC, bool keepInSystemMemory = false);
	virtual ~OpenGLES2VertexArrayObject() {destroy();}

	void draw();

	inline unsigned int const getNumTexcoords0() const {return m_iNumTexcoords;}

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
