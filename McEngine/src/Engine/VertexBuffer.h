//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		DEPRECATED: OpenGL vertex buffer wrapper
//
// $NoKeywords: $vbo
//===============================================================================//

#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

// DEPRECATED! DO NOT USE ANYMORE

#include "VertexArrayObject.h"

class VertexBuffer
{
public:
	VertexBuffer(VertexArrayObject::PRIMITIVE vertexType = VertexArrayObject::PRIMITIVE::PRIMITIVE_TRIANGLES, VertexArrayObject::USAGE usage = VertexArrayObject::USAGE::USAGE_STATIC);
	~VertexBuffer();

	void free();

	void draw(Graphics *g);

	void set(std::vector<Vector3> *vertices, std::vector<Vector2> *textureCoordinates0);

private:
	static unsigned int primitiveToOpenGL(VertexArrayObject::PRIMITIVE primitive);
	static unsigned int usageToOpenGL(VertexArrayObject::USAGE usage);

	VertexArrayObject::PRIMITIVE m_primitive;
	VertexArrayObject::USAGE m_usage;
	unsigned int m_iNumVertices;

	unsigned int m_GLVertices;
	unsigned int m_GLTextureCoordinates0;

	bool m_bReady;
	bool m_bHasTexture0;
};

#endif
