//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		OpenGL baking support for vao
//
// $NoKeywords: $glvao
//===============================================================================//

#include "OpenGLVertexArrayObject.h"

#ifdef MCENGINE_FEATURE_OPENGL

#include "Engine.h"
#include "ConVar.h"

#include "OpenGLHeaders.h"

ConVar r_opengl_legacy_vao_use_vertex_array("r_opengl_legacy_vao_use_vertex_array", false, FCVAR_CHEAT, "dramatically reduces per-vao draw calls, but completely breaks legacy ffp draw calls (vertices work, but texcoords/normals/etc. are NOT in gl_MultiTexCoord0 -> requiring a shader with attributes)");

OpenGLVertexArrayObject::OpenGLVertexArrayObject(Graphics::PRIMITIVE primitive, Graphics::USAGE_TYPE usage, bool keepInSystemMemory) : VertexArrayObject(primitive, usage, keepInSystemMemory)
{
	m_iVertexBuffer = 0;
	m_iTexcoordBuffer = 0;
	m_iColorBuffer = 0;
	m_iNormalBuffer = 0;

	m_iNumTexcoords = 0;
	m_iNumColors = 0;
	m_iNumNormals = 0;

	m_iVertexArray = 0;
}

void OpenGLVertexArrayObject::init()
{
	if (!m_bAsyncReady || m_vertices.size() < 2) return;

	// handle partial reloads

	if (m_bReady)
	{
		// update vertex buffer
		if (m_partialUpdateVertexIndices.size() > 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_iVertexBuffer);
			for (size_t i=0; i<m_partialUpdateVertexIndices.size(); i++)
			{
				const int offsetIndex = m_partialUpdateVertexIndices[i];

				// group by continuous chunks to reduce calls
				int numContinuousIndices = 1;
				while ((i + 1) < m_partialUpdateVertexIndices.size())
				{
					if ((m_partialUpdateVertexIndices[i + 1] - m_partialUpdateVertexIndices[i]) == 1)
					{
						numContinuousIndices++;
						i++;
					}
					else
						break;
				}

				glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vector3) * offsetIndex, sizeof(Vector3) * numContinuousIndices, &(m_vertices[offsetIndex]));
			}
			m_partialUpdateVertexIndices.clear();
		}

		// update color buffer
		if (m_partialUpdateColorIndices.size() > 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_iColorBuffer);
			for (size_t i=0; i<m_partialUpdateColorIndices.size(); i++)
			{
				const int offsetIndex = m_partialUpdateColorIndices[i];

				m_colors[offsetIndex] = ARGBtoABGR(m_colors[offsetIndex]);

				// group by continuous chunks to reduce calls
				int numContinuousIndices = 1;
				while ((i + 1) < m_partialUpdateColorIndices.size())
				{
					if ((m_partialUpdateColorIndices[i + 1] - m_partialUpdateColorIndices[i]) == 1)
					{
						numContinuousIndices++;
						i++;

						m_colors[m_partialUpdateColorIndices[i]] = ARGBtoABGR(m_colors[m_partialUpdateColorIndices[i]]);
					}
					else
						break;
				}

				glBufferSubData(GL_ARRAY_BUFFER, sizeof(Color) * offsetIndex, sizeof(Color) * numContinuousIndices, &(m_colors[offsetIndex]));
			}
			m_partialUpdateColorIndices.clear();
		}
	}

	if (m_iVertexBuffer != 0 && (!m_bKeepInSystemMemory || m_bReady)) return; // only fully load if we are not already loaded

	// handle full loads

	unsigned int vertexAttribArrayIndexCounter = 0;
	if (r_opengl_legacy_vao_use_vertex_array.getBool())
	{
		// build and bind vertex array
		glGenVertexArrays(1, &m_iVertexArray);
		glBindVertexArray(m_iVertexArray);
	}

	// build and fill vertex buffer
	{
		glGenBuffers(1, &m_iVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_iVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * m_vertices.size(), &(m_vertices[0]), usageToOpenGL(m_usage));

		if (r_opengl_legacy_vao_use_vertex_array.getBool())
		{
			glEnableVertexAttribArray(vertexAttribArrayIndexCounter);
			glVertexAttribPointer(vertexAttribArrayIndexCounter, 3, GL_FLOAT, GL_FALSE, 0, (char*)NULL);
			vertexAttribArrayIndexCounter++;
		}
		else
		{
			// NOTE: this state will persist engine-wide forever
			glEnableClientState(GL_VERTEX_ARRAY);
		}
	}

	// build and fill texcoord buffer
	if (m_texcoords.size() > 0 && m_texcoords[0].size() > 0)
	{
		m_iNumTexcoords = m_texcoords[0].size();

		glGenBuffers(1, &m_iTexcoordBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_iTexcoordBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * m_texcoords[0].size(), &(m_texcoords[0][0]), usageToOpenGL(m_usage));

		if (r_opengl_legacy_vao_use_vertex_array.getBool())
		{
			if (m_iNumTexcoords > 0)
			{
				glEnableVertexAttribArray(vertexAttribArrayIndexCounter);
				glVertexAttribPointer(vertexAttribArrayIndexCounter, 2, GL_FLOAT, GL_FALSE, 0, (char*)NULL);
				vertexAttribArrayIndexCounter++;
			}
		}
	}

	// build and fill color buffer
	if (m_colors.size() > 0)
	{
		m_iNumColors = m_colors.size();

		for (size_t i=0; i<m_colors.size(); i++)
		{
			m_colors[i] = ARGBtoABGR(m_colors[i]);
		}

		glGenBuffers(1, &m_iColorBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_iColorBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Color) * m_colors.size(), &(m_colors[0]), usageToOpenGL(m_usage));

		if (r_opengl_legacy_vao_use_vertex_array.getBool())
		{
			if (m_iNumColors > 0)
			{
				glEnableVertexAttribArray(vertexAttribArrayIndexCounter);
				glVertexAttribPointer(vertexAttribArrayIndexCounter, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (char*)NULL);
				vertexAttribArrayIndexCounter++;
			}
		}
	}

	// build and fill normal buffer
	if (m_normals.size() > 0)
	{
		m_iNumNormals = m_normals.size();

		glGenBuffers(1, &m_iNormalBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_iNormalBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * m_normals.size(), &(m_normals[0]), usageToOpenGL(m_usage));

		if (r_opengl_legacy_vao_use_vertex_array.getBool())
		{
			if (m_iNumNormals > 0)
			{
				glEnableVertexAttribArray(vertexAttribArrayIndexCounter);
				glVertexAttribPointer(vertexAttribArrayIndexCounter, 3, GL_FLOAT, GL_FALSE, 0, (char*)NULL);
				vertexAttribArrayIndexCounter++;
			}
		}
	}

	if (r_opengl_legacy_vao_use_vertex_array.getBool())
	{
		glBindVertexArray(0);
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

	if (m_iColorBuffer > 0)
		glDeleteBuffers(1, &m_iColorBuffer);

	if (m_iNormalBuffer > 0)
		glDeleteBuffers(1, &m_iNormalBuffer);

	if (m_iVertexArray > 0)
		glDeleteVertexArrays(1, &m_iVertexArray);

	m_iVertexBuffer = 0;
	m_iTexcoordBuffer = 0;
	m_iColorBuffer = 0;
	m_iNormalBuffer = 0;

	m_iVertexArray = 0;
}

void OpenGLVertexArrayObject::draw()
{
	if (!m_bReady)
	{
		debugLog("WARNING: OpenGLVertexArrayObject::draw() called, but was not ready!\n");
		return;
	}

	const int start = clamp<int>(m_iDrawRangeFromIndex > -1 ? m_iDrawRangeFromIndex : nearestMultipleUp((int)(m_iNumVertices*m_fDrawPercentFromPercent), m_iDrawPercentNearestMultiple), 0, m_iNumVertices);
	const int end = clamp<int>(m_iDrawRangeToIndex > -1 ? m_iDrawRangeToIndex : nearestMultipleDown((int)(m_iNumVertices*m_fDrawPercentToPercent), m_iDrawPercentNearestMultiple), 0, m_iNumVertices);

	if (start > end || std::abs(end - start) == 0) return;

	if (r_opengl_legacy_vao_use_vertex_array.getBool())
	{
		// set vao
		glBindVertexArray(m_iVertexArray);

		// render it
		glDrawArrays(primitiveToOpenGL(m_primitive), start, end - start); // (everything is already preconfigured inside the vertexArray)
	}
	else
	{
		// set vertices
		glBindBuffer(GL_ARRAY_BUFFER, m_iVertexBuffer);
		glVertexPointer(3, GL_FLOAT, 0, (char*)NULL); // set vertex pointer to vertex buffer

		// set texture0
		if (m_iNumTexcoords > 0)
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glBindBuffer(GL_ARRAY_BUFFER, m_iTexcoordBuffer);
			glTexCoordPointer(2, GL_FLOAT, 0, (char*)NULL); // set first texcoord pointer to texcoord buffer
		}
		else
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		// set colors
		if (m_iNumColors > 0)
		{
			glEnableClientState(GL_COLOR_ARRAY);
			glBindBuffer(GL_ARRAY_BUFFER, m_iColorBuffer);
			glColorPointer(4, GL_UNSIGNED_BYTE, 0, (char*)NULL); // set color pointer to color buffer
		}
		else
			glDisableClientState(GL_COLOR_ARRAY);

		// set normals
		if (m_iNumNormals > 0)
		{
			glEnableClientState(GL_NORMAL_ARRAY);
			glBindBuffer(GL_ARRAY_BUFFER, m_iNormalBuffer);
			glNormalPointer(GL_FLOAT, 0, (char*)NULL); // set normal pointer to normal buffer
		}
		else
			glDisableClientState(GL_NORMAL_ARRAY);

		// render it
		glDrawArrays(primitiveToOpenGL(m_primitive), start, end - start);
	}
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
