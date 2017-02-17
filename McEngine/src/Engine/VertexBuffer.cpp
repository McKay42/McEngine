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

VertexBuffer::VertexBuffer(Graphics::PRIMITIVE primitive, VertexArrayObject::USAGE usage)
{
	m_bReady = false;
	m_primitive = primitive;
	m_usage = usage;

	m_iNumVertices = 0;
	m_GLVertices = 0;
	m_GLTextureCoordinates0 = 0;

	m_iDrawPercentNearestMultiple = 0;
	m_fFromPercent = 0.0f;
	m_fToPercent = 1.0f;

	// generate & bind vertex buffer
	glGenBuffers(1, &m_GLVertices);

	m_bHasTexture0 = false;
	glGenBuffers(1, &m_GLTextureCoordinates0);

	m_bReady = true;
}

void VertexBuffer::set(std::vector<Vector3> *vertices, std::vector<Vector2> *textureCoordinates0)
{
	if (vertices == NULL || vertices->size() < 1)
	{
		if (vertices == NULL)
			engine->showMessageError("VertexBuffer Error", "Invalid numVertices!");
		else
			engine->showMessageError("VertexBuffer Error", UString::format("Invalid numVertices (%i)!", vertices->size()));
		m_bReady = false;
		return;
	}

	if (vertices != NULL)
	{
		m_iNumVertices = vertices->size();
		m_iDrawPercentNearestMultiple = m_iNumVertices;

		glBindBuffer(GL_ARRAY_BUFFER, m_GLVertices);
		glBufferData(GL_ARRAY_BUFFER, m_iNumVertices*3*sizeof(float), &(*vertices)[0], usageToOpenGL(m_usage));
	}

	if (textureCoordinates0 != NULL)
	{
		m_bHasTexture0 = true;

		glBindBuffer(GL_ARRAY_BUFFER, m_GLTextureCoordinates0);
		glBufferData(GL_ARRAY_BUFFER, m_iNumVertices*2*sizeof(float), &(*textureCoordinates0)[0], usageToOpenGL(m_usage));
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

	glDeleteBuffers(1, &m_GLVertices);
	glDeleteBuffers(1, &m_GLTextureCoordinates0);
}

void VertexBuffer::draw(Graphics *g)
{
	if (!m_bReady) return;

	int start = clamp<int>(nearestMultipleOf((int)(m_iNumVertices*m_fFromPercent), m_iDrawPercentNearestMultiple), 0, m_iNumVertices);
	int end = clamp<int>(nearestMultipleOf((int)(m_iNumVertices*m_fToPercent), m_iDrawPercentNearestMultiple), 0, m_iNumVertices);

	if (start > end || std::abs(end-start) == 0)
		return;

	// set vertices
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, m_GLVertices);
	glVertexPointer(3, GL_FLOAT, 0, (char*)NULL); // set vertex pointer to vertex buffer

	// set texture0
	if (m_bHasTexture0)
	{
		glClientActiveTexture(GL_TEXTURE0);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, m_GLTextureCoordinates0);
		glTexCoordPointer(2, GL_FLOAT, 0, (char*)NULL); // set first texcoord pointer to texcoord buffer
	}

	// render it
	glDrawArrays(primitiveToOpenGL(m_primitive), start, end);

	// disable everything
	if (m_bHasTexture0)
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glDisableClientState(GL_VERTEX_ARRAY);
}

unsigned int VertexBuffer::primitiveToOpenGL(Graphics::PRIMITIVE primitive)
{
	switch (primitive)
	{
	case Graphics::PRIMITIVE::PRIMITIVE_TRIANGLES:
		return GL_TRIANGLES;
	case Graphics::PRIMITIVE::PRIMITIVE_TRIANGLE_FAN:
		return GL_TRIANGLE_FAN;
	case Graphics::PRIMITIVE::PRIMITIVE_QUADS:
		return GL_QUADS;
	}

	return GL_TRIANGLES;
}

unsigned int VertexBuffer::usageToOpenGL(VertexArrayObject::USAGE usage)
{
	switch (usage)
	{
	case VertexArrayObject::USAGE::USAGE_STATIC:
		return GL_STATIC_DRAW;
	case VertexArrayObject::USAGE::USAGE_DYNAMIC:
		return GL_DYNAMIC_DRAW;
	case VertexArrayObject::USAGE::USAGE_STREAM:
		return GL_STREAM_DRAW;
	}

	return GL_STATIC_DRAW;
}

int VertexBuffer::nearestMultipleOf(int number, int multiple)
{
	int result = number + multiple/2;
	result -= result % multiple;

	return result;
}
