//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		OpenGL baking support for vao
//
// $NoKeywords: $glvao
//===============================================================================//

#ifndef OPENGL3VERTEXARRAYOBJECT_H
#define OPENGL3VERTEXARRAYOBJECT_H

#include "VertexArrayObject.h"

#ifdef MCENGINE_FEATURE_OPENGL

class OpenGL3VertexArrayObject : public VertexArrayObject
{
public:
	OpenGL3VertexArrayObject(Graphics::PRIMITIVE primitive = Graphics::PRIMITIVE::PRIMITIVE_TRIANGLES, Graphics::USAGE_TYPE usage = Graphics::USAGE_TYPE::USAGE_STATIC, bool keepInSystemMemory = false);
	virtual ~OpenGL3VertexArrayObject() {destroy();}

	void draw();

	inline unsigned int const getNumTexcoords0() const {return m_iNumTexcoords;}

private:
	static int primitiveToOpenGL(Graphics::PRIMITIVE primitive);
	static unsigned int usageToOpenGL(Graphics::USAGE_TYPE usage);

	virtual void init();
	virtual void initAsync();
	virtual void destroy();

	unsigned int m_iVAO;
	unsigned int m_iVertexBuffer;
	unsigned int m_iTexcoordBuffer;

	unsigned int m_iNumTexcoords;
};

#endif

#endif
