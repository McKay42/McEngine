//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		OpenGL baking support for vao
//
// $NoKeywords: $glvao
//===============================================================================//

#include "OpenGL3VertexArrayObject.h"

#ifdef MCENGINE_FEATURE_OPENGL

#include "Engine.h"
#include "OpenGL3Interface.h"

#include "OpenGLHeaders.h"

OpenGL3VertexArrayObject::OpenGL3VertexArrayObject(Graphics::PRIMITIVE primitive, Graphics::USAGE_TYPE usage, bool keepInSystemMemory) : VertexArrayObject(primitive, usage, keepInSystemMemory)
{
	m_iVAO = 0;
	m_iVertexBuffer = 0;
	m_iTexcoordBuffer = 0;

	m_iNumTexcoords = 0;
}

void OpenGL3VertexArrayObject::init()
{
	if (m_vertices.size() < 2) return;

	OpenGL3Interface *g = (OpenGL3Interface*)engine->getGraphics();

	// backup vao
	int vaoBackup = 0;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vaoBackup);

	// create and bind a VAO to hold state for this model
	glGenVertexArrays(1, &m_iVAO);
	glBindVertexArray(m_iVAO);
	{
		// populate a vertex buffer
		glGenBuffers(1, &m_iVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_iVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * m_vertices.size(), &(m_vertices[0]), usageToOpenGL(m_usage));

		// identify the components in the vertex buffer
		glEnableVertexAttribArray(g->getShaderGenericAttribPosition());
		glVertexAttribPointer(g->getShaderGenericAttribPosition(), 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), (GLvoid*)0);

		// populate texcoord buffer
		if (m_texcoords.size() > 0 && m_texcoords[0].size() > 0)
		{
			m_iNumTexcoords = m_texcoords[0].size();

			glGenBuffers(1, &m_iTexcoordBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, m_iTexcoordBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * m_texcoords[0].size(), &(m_texcoords[0][0]), usageToOpenGL(m_usage));

			// identify the components in the texcoord buffer
			glEnableVertexAttribArray(g->getShaderGenericAttribUV());
			glVertexAttribPointer(g->getShaderGenericAttribUV(), 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		}
	}
	glBindVertexArray(vaoBackup); // restore vao

	// free memory
	if (!m_bKeepInSystemMemory)
		clear();

	m_bReady = true;
}

void OpenGL3VertexArrayObject::initAsync()
{
	m_bAsyncReady = true;
}

void OpenGL3VertexArrayObject::destroy()
{
	VertexArrayObject::destroy();

	if (m_iVAO > 0)
	{
		glDeleteBuffers(1, &m_iVertexBuffer);
		glDeleteBuffers(1, &m_iTexcoordBuffer);
		glDeleteVertexArrays(1, &m_iVAO);

		m_iVAO = 0;
		m_iVertexBuffer = 0;
		m_iTexcoordBuffer = 0;
	}
}

void OpenGL3VertexArrayObject::draw()
{
	if (!m_bReady)
	{
		debugLog("WARNING: OpenGL3VertexArrayObject::draw() called, but was not ready!\n");
		return;
	}

	int start = clamp<int>(nearestMultipleUp((int)(m_iNumVertices*m_fDrawPercentFromPercent), m_iDrawPercentNearestMultiple), 0, m_iNumVertices); // HACKHACK: osu sliders
	int end = clamp<int>(nearestMultipleDown((int)(m_iNumVertices*m_fDrawPercentToPercent), m_iDrawPercentNearestMultiple), 0, m_iNumVertices); // HACKHACK: osu sliders

	if (start > end || std::abs(end-start) == 0)
		return;

	// backup vao
	int vaoBackup = 0;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vaoBackup);

	// bind and draw
	glBindVertexArray(m_iVAO);
	{
		glDrawArrays(primitiveToOpenGL(m_primitive), start, end-start);
	}
	glBindVertexArray(vaoBackup); // restore vao
}

int OpenGL3VertexArrayObject::primitiveToOpenGL(Graphics::PRIMITIVE primitive)
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

unsigned int OpenGL3VertexArrayObject::usageToOpenGL(Graphics::USAGE_TYPE usage)
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
