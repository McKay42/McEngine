//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		OpenGL baking support for vao
//
// $NoKeywords: $glvao
//===============================================================================//

#include "OpenGLVertexArrayObject.h"

#ifdef MCENGINE_FEATURE_OPENGL

#include "Engine.h"

#include "OpenGLHeaders.h"

OpenGLVertexArrayObject::OpenGLVertexArrayObject(Graphics::PRIMITIVE primitive, Graphics::USAGE_TYPE usage, bool keepInSystemMemory) : VertexArrayObject(primitive, usage, keepInSystemMemory)
{
	m_iVertexBuffer = 0;
	m_iTexcoordBuffer = 0;

	m_iNumTexcoords = 0;
}

void OpenGLVertexArrayObject::init()
{
	if (m_vertices.size() < 2) return;

	// build and fill vertex buffer
	glGenBuffers(1, &m_iVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_iVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * m_vertices.size(), &(m_vertices[0]), usageToOpenGL(m_usage));

	// build and fill texcoord buffer
	if (m_texcoords.size() > 0 && m_texcoords[0].size() > 0)
	{
		m_iNumTexcoords = m_texcoords[0].size();

		glGenBuffers(1, &m_iTexcoordBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_iTexcoordBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * m_texcoords[0].size(), &(m_texcoords[0][0]), usageToOpenGL(m_usage));
	}

	// free memory
	if (!m_bKeepInSystemMemory)
		clear();

	m_bReady = true;
}

void OpenGLVertexArrayObject::initAsync()
{
	m_bAsyncReady = true;
}

void OpenGLVertexArrayObject::destroy()
{
	VertexArrayObject::destroy();

	if (m_iVertexBuffer > 0)
		glDeleteBuffers(1, &m_iVertexBuffer);

	if (m_iTexcoordBuffer > 0)
		glDeleteBuffers(1, &m_iTexcoordBuffer);

	m_iVertexBuffer = 0;
	m_iTexcoordBuffer = 0;
}

void OpenGLVertexArrayObject::draw()
{
	if (!m_bReady)
	{
		debugLog("WARNING: OpenGLVertexArrayObject::draw() called, but was not ready!\n");
		return;
	}

	int start = clamp<int>(nearestMultipleUp((int)(m_iNumVertices*m_fDrawPercentFromPercent), m_iDrawPercentNearestMultiple), 0, m_iNumVertices); // HACKHACK: osu sliders
	int end = clamp<int>(nearestMultipleDown((int)(m_iNumVertices*m_fDrawPercentToPercent), m_iDrawPercentNearestMultiple), 0, m_iNumVertices); // HACKHACK: osu sliders

	if (start > end || std::abs(end-start) == 0)
		return;

	// set vertices
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, m_iVertexBuffer);
	glVertexPointer(3, GL_FLOAT, 0, (char*)NULL); // set vertex pointer to vertex buffer

	// set texture0
	if (m_iNumTexcoords > 0)
	{
		glClientActiveTexture(GL_TEXTURE0);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, m_iTexcoordBuffer);
		glTexCoordPointer(2, GL_FLOAT, 0, (char*)NULL); // set first texcoord pointer to texcoord buffer
	}

	// render it
	glDrawArrays(primitiveToOpenGL(m_primitive), start, end-start);

	// disable everything
	if (m_iNumTexcoords > 0)
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glDisableClientState(GL_VERTEX_ARRAY);
}

int OpenGLVertexArrayObject::primitiveToOpenGL(Graphics::PRIMITIVE primitive)
{
	switch (primitive)
	{
	case Graphics::PRIMITIVE::PRIMITIVE_LINES:
		return GL_LINES;
	case Graphics::PRIMITIVE::PRIMITIVE_LINE_STRIP:
		return GL_LINE_STRIP;
	case Graphics::PRIMITIVE::PRIMITIVE_TRIANGLES:
		return GL_TRIANGLES;
	case Graphics::PRIMITIVE::PRIMITIVE_TRIANGLE_FAN:
		return GL_TRIANGLE_FAN;
	case Graphics::PRIMITIVE::PRIMITIVE_TRIANGLE_STRIP:
		return GL_TRIANGLE_STRIP;
	case Graphics::PRIMITIVE::PRIMITIVE_QUADS:
		return GL_QUADS;
	}

	return GL_TRIANGLES;
}

unsigned int OpenGLVertexArrayObject::usageToOpenGL(Graphics::USAGE_TYPE usage)
{
	switch (usage)
	{
	case Graphics::USAGE_TYPE::USAGE_STATIC:
		return GL_STATIC_DRAW;
	case Graphics::USAGE_TYPE::USAGE_DYNAMIC:
		return GL_DYNAMIC_DRAW;
	case Graphics::USAGE_TYPE::USAGE_STREAM:
		return GL_STREAM_DRAW;
	}

	return GL_STATIC_DRAW;
}

#endif
