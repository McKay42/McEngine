//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		modern opengl style mesh wrapper (vertices, texcoords, etc.)
//
// $NoKeywords: $vao
//===============================================================================//

#include "VertexArrayObject.h"

#include "Engine.h"

VertexArrayObject::VertexArrayObject(Graphics::PRIMITIVE primitive, Graphics::USAGE_TYPE usage) : Resource()
{
	m_primitive = primitive;
	m_usage = usage;

	m_iDrawPercentNearestMultiple = 0;
	m_fDrawPercentFromPercent = 0.0f;
	m_fDrawPercentToPercent = 1.0f;
}

VertexArrayObject::~VertexArrayObject()
{
}

void VertexArrayObject::init()
{
	m_bReady = true;
}

void VertexArrayObject::initAsync()
{
	m_bAsyncReady = true;
}

void VertexArrayObject::destroy()
{
}

void VertexArrayObject::clear()
{
	m_vertices.clear();
	m_texcoords.clear();
	m_normals.clear();
	m_colors.clear();
}

void VertexArrayObject::addVertex(Vector2 v)
{
	addVertex(Vector3(v.x, v.y, 0));
}

void VertexArrayObject::addVertex(float x, float y, float z)
{
	addVertex(Vector3(x,y,z));
}

void VertexArrayObject::addVertex(Vector3 v)
{
	m_vertices.push_back(v);
}

void VertexArrayObject::addTexcoord(float u, float v, unsigned int textureUnit)
{
	updateTexcoordArraySize(textureUnit);
	m_texcoords[textureUnit].push_back(Vector2(u, v));
}

void VertexArrayObject::addTexcoord(Vector2 uv, unsigned int textureUnit)
{
	updateTexcoordArraySize(textureUnit);
	m_texcoords[textureUnit].push_back(uv);
}

void VertexArrayObject::addNormal(Vector3 normal)
{
	m_normals.push_back(normal);
}

void VertexArrayObject::addNormal(float x, float y, float z)
{
	m_normals.push_back(Vector3(x,y,z));
}

void VertexArrayObject::addColor(Color color)
{
	m_colors.push_back(color);
}

void VertexArrayObject::setType(Graphics::PRIMITIVE primitive)
{
	m_primitive = primitive;
}

void VertexArrayObject::setDrawPercent(float fromPercent, float toPercent, int nearestMultiple)
{
	m_fDrawPercentFromPercent = fromPercent;
	m_fDrawPercentToPercent = toPercent;
	m_iDrawPercentNearestMultiple = nearestMultiple;
}

void VertexArrayObject::updateTexcoordArraySize(unsigned int textureUnit)
{
	while (m_texcoords.size() < textureUnit+1)
	{
		std::vector<Vector2> emptyVector;
		m_texcoords.push_back(emptyVector);
	}
}

int VertexArrayObject::nearestMultipleOf(int number, int multiple)
{
	int result = number + multiple/2;

	if (multiple > 0)
		result -= result % multiple;

	return result;
}
