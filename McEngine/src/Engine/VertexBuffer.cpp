//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		DEPRECATED: OpenGL vertex buffer wrapper
//
// $NoKeywords: $vbo
//===============================================================================//

// DEPRECATED! DO NOT USE ANYMORE

#include "VertexBuffer.h"
#include "Engine.h"

#include "OpenGLHeaders.h"

VertexBuffer::VertexBuffer(VertexArrayObject::PRIMITIVE primitive, VertexArrayObject::USAGE usage)
{
	m_bReady = false;
	m_primitive = primitive;
	m_usage = usage;

	debugLog("Building VertexBuffer ...\n");

	m_iNumVertices = 0;
	m_GLVertices = 0;
	m_GLTextureCoordinates0 = 0;

	// generate & bind vertex buffer
	glGenBuffersARB(1, &m_GLVertices);

	m_bHasTexture0 = false;
	glGenBuffersARB(1, &m_GLTextureCoordinates0);

	m_bReady = true;
}

void VertexBuffer::set(std::vector<Vector3> *vertices, std::vector<Vector2> *textureCoordinates0)
{
	if (vertices == NULL || vertices->size() < 1)
	{
		engine->showMessageError("VertexBuffer Error", "Invalid numVertices!");
		m_bReady = false;
		return;
	}

	if (vertices != NULL)
	{
		m_iNumVertices = vertices->size();

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_GLVertices);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_iNumVertices*3*sizeof(float), &(*vertices)[0], usageToOpenGL(m_usage));
	}

	if (textureCoordinates0 != NULL)
	{
		m_bHasTexture0 = true;

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_GLTextureCoordinates0);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_iNumVertices*2*sizeof(float), &(*textureCoordinates0)[0], usageToOpenGL(m_usage));
	}
	else
		m_bHasTexture0 = false;
}

VertexBuffer::~VertexBuffer()
{
	free();
}

void VertexBuffer::free()
{
	m_bReady = false;

	glDeleteBuffersARB(1, &m_GLVertices);

	if (m_bHasTexture0)
		glDeleteBuffersARB(1, &m_GLTextureCoordinates0);
}

void VertexBuffer::draw(Graphics *g)
{
	if (!m_bReady) return;

	// set vertices
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_GLVertices);
	glVertexPointer(3, GL_FLOAT, 0, (char*)NULL); // set vertex pointer to vertex buffer

	// set texture0
	if (m_bHasTexture0)
	{
		glClientActiveTextureARB(GL_TEXTURE0_ARB);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_GLTextureCoordinates0);
		glTexCoordPointer(2, GL_FLOAT, 0, (char*)NULL); // set first texcoord pointer to texcoord buffer
	}

	// render it
	glDrawArrays(primitiveToOpenGL(m_primitive), 0, m_iNumVertices);

	// disable everything
	if (m_bHasTexture0)
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glDisableClientState(GL_VERTEX_ARRAY);
}

unsigned int VertexBuffer::primitiveToOpenGL(VertexArrayObject::PRIMITIVE primitive)
{
	switch (primitive)
	{
	case VertexArrayObject::PRIMITIVE::PRIMITIVE_TRIANGLES:
		return GL_TRIANGLES;
	case VertexArrayObject::PRIMITIVE::PRIMITIVE_TRIANGLE_FAN:
		return GL_TRIANGLE_FAN;
	case VertexArrayObject::PRIMITIVE::PRIMITIVE_QUADS:
		return GL_QUADS;
	}

	return GL_TRIANGLES;
}

unsigned int VertexBuffer::usageToOpenGL(VertexArrayObject::USAGE usage)
{
	switch (usage)
	{
	case VertexArrayObject::USAGE::USAGE_STATIC:
		return GL_STATIC_DRAW_ARB;
	case VertexArrayObject::USAGE::USAGE_DYNAMIC:
		return GL_DYNAMIC_DRAW_ARB;
	case VertexArrayObject::USAGE::USAGE_STREAM:
		return GL_STREAM_DRAW_ARB;
	}

	return GL_STATIC_DRAW_ARB;
}
