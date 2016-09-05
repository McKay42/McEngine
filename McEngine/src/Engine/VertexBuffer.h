//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		DEPRECATED: OpenGL vertex buffer wrapper
//
// $NoKeywords: $vbo
//===============================================================================//

#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

// DEPRECATED! DO NOT USE ANYMORE

#include "cbase.h"

class VertexBuffer
{
public:
	VertexBuffer();
	~VertexBuffer();

	void free();

	void draw(Graphics *g);

	void set(std::vector<Vector3> *vertices, std::vector<Vector2> *textureCoordinates0);

private:
	unsigned int m_iNumVertices;

	unsigned int m_GLVertices;
	unsigned int m_GLTextureCoordinates0;

	bool m_bReady;
	bool m_bHasTexture0;
};

#endif
