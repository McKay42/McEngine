//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		modern opengl style mesh wrapper (vertices, texcoords, etc.)
//
// $NoKeywords: $vao
//===============================================================================//

#ifndef VERTEXARRAYOBJECT_H
#define VERTEXARRAYOBJECT_H

#include "Resource.h"

class VertexArrayObject : public Resource
{
public:
	VertexArrayObject(Graphics::PRIMITIVE primitive = Graphics::PRIMITIVE::PRIMITIVE_TRIANGLES, Graphics::USAGE_TYPE usage = Graphics::USAGE_TYPE::USAGE_STATIC, bool keepInSystemMemory = false);
	virtual ~VertexArrayObject();

	// TODO: fix the naming schema. clear = empty = just empty the containers, but not necessarily release memory
	void clear();
	void empty();

	void addVertex(Vector2 v);
	void addVertex(Vector3 v);
	void addVertex(float x, float y, float z = 0);

	void addTexcoord(Vector2 uv, unsigned int textureUnit = 0);
	void addTexcoord(float u, float v, unsigned int textureUnit = 0);

	void addNormal(Vector3 normal);
	void addNormal(float x, float y, float z);

	void addColor(Color color);

	void setType(Graphics::PRIMITIVE primitive);
	void setDrawPercent(float fromPercent = 0.0f, float toPercent = 1.0f, int nearestMultiple = 0);

	inline Graphics::PRIMITIVE getPrimitive() {return m_primitive;}
	inline Graphics::USAGE_TYPE getUsage() {return m_usage;}

	const std::vector<Vector3> &getVertices() const {return m_vertices;}
	const std::vector<std::vector<Vector2>> &getTexcoords() const {return m_texcoords;}
	const std::vector<Vector3> &getNormals() const {return m_normals;}
	const std::vector<Color> &getColors() const {return m_colors;}

	inline unsigned int getNumVertices() const {return m_iNumVertices;}

protected:
	static int nearestMultipleOf(int number, int multiple);
	static int nearestMultipleUp(int number, int multiple);
	static int nearestMultipleDown(int number, int multiple);

	virtual void init();
	virtual void initAsync();
	virtual void destroy();

	void updateTexcoordArraySize(unsigned int textureUnit);

	Graphics::PRIMITIVE m_primitive;
	Graphics::USAGE_TYPE m_usage;
	bool m_bKeepInSystemMemory;

	std::vector<Vector3> m_vertices;
	std::vector<std::vector<Vector2>> m_texcoords;
	std::vector<Vector3> m_normals;
	std::vector<Color> m_colors;

	unsigned int m_iNumVertices;

	int m_iDrawPercentNearestMultiple;
	float m_fDrawPercentFromPercent;
	float m_fDrawPercentToPercent;
};

#endif
