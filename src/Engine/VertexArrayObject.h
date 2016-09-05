//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		modern opengl style mesh wrapper (vertices, texcoords, etc.)
//
// $NoKeywords: $vao
//===============================================================================//

#ifndef VERTEXARRAYOBJECT_H
#define VERTEXARRAYOBJECT_H

#include "cbase.h"

class VertexArrayObject
{
public:
	enum TYPE
	{
		TYPE_TRIANGLES,
		TYPE_TRIANGLE_FAN,
		TYPE_QUADS
	};

	VertexArrayObject();
	virtual ~VertexArrayObject();

	void clear();

	void setType(TYPE type) {m_type = type;}

	void addVertex(Vector2 v);
	void addVertex(Vector3 v);
	void addVertex(float x, float y, float z = 0);

	void addTexcoord(Vector2 uv, unsigned int textureUnit = 0);
	void addTexcoord(float u, float v, unsigned int textureUnit = 0);

	void addNormal(Vector3 normal);
	void addNormal(float x, float y, float z);

	void addColor(Color color);

	inline TYPE getType() {return m_type;}

	const std::vector<Vector3> &getVertices() const {return m_vertices;}
	const std::vector<std::vector<Vector2>> &getTexcoords() const {return m_texcoords;}
	const std::vector<Vector3> &getNormals() const {return m_normals;}
	const std::vector<Color> &getColors() const {return m_colors;}

private:
	void updateTexcoordArraySize(unsigned int textureUnit);

	TYPE m_type;

	std::vector<Vector3> m_vertices;
	std::vector<std::vector<Vector2>> m_texcoords;
	std::vector<Vector3> m_normals;
	std::vector<Color> m_colors;
};

#endif
