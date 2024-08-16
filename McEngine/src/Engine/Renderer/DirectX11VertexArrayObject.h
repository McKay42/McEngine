//================ Copyright (c) 2022, PG, All rights reserved. =================//
//
// Purpose:		DirectX baking support for vao
//
// $NoKeywords: $dxvao
//===============================================================================//

#ifndef DIRECTX11VERTEXARRAYOBJECT_H
#define DIRECTX11VERTEXARRAYOBJECT_H

#include "VertexArrayObject.h"

#ifdef MCENGINE_FEATURE_DIRECTX11

#include "DirectX11Interface.h"

class DirectX11VertexArrayObject : public VertexArrayObject
{
public:
	DirectX11VertexArrayObject(Graphics::PRIMITIVE primitive = Graphics::PRIMITIVE::PRIMITIVE_TRIANGLES, Graphics::USAGE_TYPE usage = Graphics::USAGE_TYPE::USAGE_STATIC, bool keepInSystemMemory = false);
	virtual ~DirectX11VertexArrayObject() {destroy();}

	void draw();

protected:
	virtual void init();
	virtual void initAsync();
	virtual void destroy();

private:
	static int primitiveToDirectX(Graphics::PRIMITIVE primitive);
	static int usageToDirectX(Graphics::USAGE_TYPE usage);

	ID3D11Buffer *m_vertexBuffer;

	Graphics::PRIMITIVE m_convertedPrimitive;
	std::vector<DirectX11Interface::SimpleVertex> m_convertedVertices;
};

#endif

#endif
