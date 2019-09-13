//================ Copyright (c) 2019, PG, All rights reserved. =================//
//
// Purpose:		OpenGLES2 baking support for vao
//
// $NoKeywords: $gles2vao
//===============================================================================//

#include "OpenGLES2VertexArrayObject.h"

#ifdef MCENGINE_FEATURE_OPENGLES

#include "Engine.h"
#include "OpenGLES2Interface.h"

#include "OpenGLHeaders.h"

OpenGLES2VertexArrayObject::OpenGLES2VertexArrayObject(Graphics::PRIMITIVE primitive, Graphics::USAGE_TYPE usage, bool keepInSystemMemory) : VertexArrayObject(primitive, usage, keepInSystemMemory)
{
	m_iVertexBuffer = 0;
	m_iTexcoordBuffer = 0;

	m_iNumTexcoords = 0;
}

void OpenGLES2VertexArrayObject::init()
{
	if (m_vertices.size() < 2) return;

	// populate a vertex buffer
	glGenBuffers(1, &m_iVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_iVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * m_vertices.size(), &(m_vertices[0]), usageToOpenGL(m_usage));

	// populate texcoord buffer
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

void OpenGLES2VertexArrayObject::initAsync()
{
	m_bAsyncReady = true;
}

void OpenGLES2VertexArrayObject::destroy()
{
	VertexArrayObject::destroy();

	if (m_iVertexBuffer > 0)
		glDeleteBuffers(1, &m_iVertexBuffer);

	if (m_iTexcoordBuffer > 0)
		glDeleteBuffers(1, &m_iTexcoordBuffer);

	m_iVertexBuffer = 0;
	m_iTexcoordBuffer = 0;
}

void OpenGLES2VertexArrayObject::draw()
{
	if (!m_bReady)
	{
		debugLog("WARNING: OpenGLES2VertexArrayObject::draw() called, but was not ready!\n");
		return;
	}

	const int start = clamp<int>(nearestMultipleUp((int)(m_iNumVertices*m_fDrawPercentFromPercent), m_iDrawPercentNearestMultiple), 0, m_iNumVertices); // HACKHACK: osu sliders
	const int end = clamp<int>(nearestMultipleDown((int)(m_iNumVertices*m_fDrawPercentToPercent), m_iDrawPercentNearestMultiple), 0, m_iNumVertices); // HACKHACK: osu sliders

	if (start > end || std::abs(end-start) == 0)
		return;

	OpenGLES2Interface *g = (OpenGLES2Interface*)engine->getGraphics();

	// bind
	{
		// NOTE: since opengl es 2.0 doesn't support vaos, we have to update glVertexAttribPointer for every buffer independently every time
		// HACKHACK: these must match the default renderer exactly
		glDisableVertexAttribArray(g->getShaderGenericAttribCol());

		glBindBuffer(GL_ARRAY_BUFFER, m_iVertexBuffer);
		glVertexAttribPointer(g->getShaderGenericAttribPosition(), 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

		glBindBuffer(GL_ARRAY_BUFFER, m_iTexcoordBuffer);
		glVertexAttribPointer(g->getShaderGenericAttribUV(), 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	}

	// draw
	{
		glDrawArrays(primitiveToOpenGL(m_primitive), start, end-start);
	}

	// reset
	{
		// HACKHACK: these must match the default renderer exactly
		glBindBuffer(GL_ARRAY_BUFFER, g->getVBOVertices());
		glVertexAttribPointer(g->getShaderGenericAttribPosition(), 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

		glBindBuffer(GL_ARRAY_BUFFER, g->getVBOTexcoords());
		glVertexAttribPointer(g->getShaderGenericAttribUV(), 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);

		glEnableVertexAttribArray(g->getShaderGenericAttribCol());
	}
}

int OpenGLES2VertexArrayObject::primitiveToOpenGL(Graphics::PRIMITIVE primitive)
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
		return 0; // not supported
	}

	return GL_TRIANGLES;
}

unsigned int OpenGLES2VertexArrayObject::usageToOpenGL(Graphics::USAGE_TYPE usage)
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
