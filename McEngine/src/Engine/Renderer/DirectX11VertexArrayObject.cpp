//================ Copyright (c) 2022, PG, All rights reserved. =================//
//
// Purpose:		DirectX baking support for vao
//
// $NoKeywords: $dxvao
//===============================================================================//

#include "DirectX11VertexArrayObject.h"

#ifdef MCENGINE_FEATURE_DIRECTX11

#include "Engine.h"

#include "DirectX11Interface.h"

DirectX11VertexArrayObject::DirectX11VertexArrayObject(Graphics::PRIMITIVE primitive, Graphics::USAGE_TYPE usage, bool keepInSystemMemory) : VertexArrayObject(primitive, usage, keepInSystemMemory)
{
	m_vertexBuffer = NULL;

	m_convertedPrimitive = primitive;
}

void DirectX11VertexArrayObject::init()
{
	if (!m_bAsyncReady || m_vertices.size() < 2) return;

	const DirectX11Interface *dx11 = dynamic_cast<DirectX11Interface*>(engine->getGraphics());

	if (m_bReady)
	{
		const D3D11_USAGE usage = (D3D11_USAGE)usageToDirectX(m_usage);

		// TODO: somehow merge this with the partialUpdateColorIndices, annoying
		// TODO: also support converted meshes, extremely annoying. currently this will crash for converted meshes!

		// update vertex buffer

		if (m_partialUpdateVertexIndices.size() > 0)
		{
			for (size_t i=0; i<m_partialUpdateVertexIndices.size(); i++)
			{
				const int offsetIndex = m_partialUpdateVertexIndices[i];

				m_convertedVertices[offsetIndex].pos = m_vertices[offsetIndex];

				// group by continuous chunks to reduce calls
				int numContinuousIndices = 1;
				while ((i + 1) < m_partialUpdateVertexIndices.size())
				{
					if ((m_partialUpdateVertexIndices[i + 1] - m_partialUpdateVertexIndices[i]) == 1)
					{
						numContinuousIndices++;
						i++;

						m_convertedVertices[m_partialUpdateVertexIndices[i]].pos = m_vertices[m_partialUpdateVertexIndices[i]];
					}
					else
						break;
				}

				if (usage == D3D11_USAGE::D3D11_USAGE_DEFAULT)
				{
					D3D11_BOX box;
					{
						box.left = sizeof(DirectX11Interface::SimpleVertex) * offsetIndex;
						box.right = box.left + (sizeof(DirectX11Interface::SimpleVertex) * numContinuousIndices);
						box.top = 0;
						box.bottom = 1;
						box.front = 0;
						box.back = 1;
					}
					dx11->getDeviceContext()->UpdateSubresource(m_vertexBuffer, 0, &box, &m_convertedVertices[offsetIndex], 0, 0);
				}
				else if (usage == D3D11_USAGE::D3D11_USAGE_DYNAMIC)
				{
					D3D11_MAPPED_SUBRESOURCE mappedResource;
					ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
					if (SUCCEEDED(dx11->getDeviceContext()->Map(m_vertexBuffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
					{
						memcpy(mappedResource.pData, &m_convertedVertices[0], sizeof(DirectX11Interface::SimpleVertex) * m_convertedVertices.size());
						dx11->getDeviceContext()->Unmap(m_vertexBuffer, 0);
					}
				}
			}
			m_partialUpdateVertexIndices.clear();
			m_partialUpdateColorIndices.clear();
		}

		// TODO: update color buffer
	}

	if (m_vertexBuffer != NULL && (!m_bKeepInSystemMemory || m_bReady)) return; // only fully load if we are not already loaded

	// TODO: optimize this piece of shit

	m_convertedVertices.clear();
	{
		std::vector<Vector3> finalVertices = m_vertices;
		std::vector<std::vector<Vector2>> finalTexcoords = m_texcoords;
		std::vector<Vector4> colors;
		std::vector<Vector4> finalColors;

		for (size_t i=0; i<m_colors.size(); i++)
		{
			const Vector4 color = Vector4(COLOR_GET_Rf(m_colors[i]), COLOR_GET_Gf(m_colors[i]), COLOR_GET_Bf(m_colors[i]), COLOR_GET_Af(m_colors[i]));
			colors.push_back(color);
			finalColors.push_back(color);
		}
		const size_t maxColorIndex = (finalColors.size() > 0 ? finalColors.size() - 1 : 0);

		if (m_primitive == Graphics::PRIMITIVE::PRIMITIVE_QUADS)
		{
			for (size_t t=0; t<finalTexcoords.size(); t++)
			{
				finalTexcoords[t].clear();
			}
			finalColors.clear();
			m_convertedPrimitive = Graphics::PRIMITIVE::PRIMITIVE_TRIANGLES;

			if (m_vertices.size() > 3)
			{
				for (size_t i=0; i<m_vertices.size(); i+=4)
				{
					finalVertices.push_back(m_vertices[i + 0]);
					finalVertices.push_back(m_vertices[i + 1]);
					finalVertices.push_back(m_vertices[i + 2]);

					for (size_t t=0; t<m_texcoords.size(); t++)
					{
						finalTexcoords[t].push_back(m_texcoords[t][i + 0]);
						finalTexcoords[t].push_back(m_texcoords[t][i + 1]);
						finalTexcoords[t].push_back(m_texcoords[t][i + 2]);
					}

					if (colors.size() > 0)
					{
						finalColors.push_back(colors[clamp<int>(i + 0, 0, maxColorIndex)]);
						finalColors.push_back(colors[clamp<int>(i + 1, 0, maxColorIndex)]);
						finalColors.push_back(colors[clamp<int>(i + 2, 0, maxColorIndex)]);
					}

					finalVertices.push_back(m_vertices[i + 0]);
					finalVertices.push_back(m_vertices[i + 2]);
					finalVertices.push_back(m_vertices[i + 3]);

					for (size_t t=0; t<m_texcoords.size(); t++)
					{
						finalTexcoords[t].push_back(m_texcoords[t][i + 0]);
						finalTexcoords[t].push_back(m_texcoords[t][i + 2]);
						finalTexcoords[t].push_back(m_texcoords[t][i + 3]);
					}

					if (colors.size() > 0)
					{
						finalColors.push_back(colors[clamp<int>(i + 0, 0, maxColorIndex)]);
						finalColors.push_back(colors[clamp<int>(i + 2, 0, maxColorIndex)]);
						finalColors.push_back(colors[clamp<int>(i + 3, 0, maxColorIndex)]);
					}
				}
			}
		}
		else if (m_primitive == Graphics::PRIMITIVE::PRIMITIVE_TRIANGLE_FAN)
		{
			finalVertices.clear();
			for (size_t t=0; t<finalTexcoords.size(); t++)
			{
				finalTexcoords[t].clear();
			}
			finalColors.clear();
			m_convertedPrimitive = Graphics::PRIMITIVE::PRIMITIVE_TRIANGLES;

			if (m_vertices.size() > 2)
			{
				for (size_t i=2; i<m_vertices.size(); i++)
				{
					finalVertices.push_back(m_vertices[0]);

					finalVertices.push_back(m_vertices[i]);
					finalVertices.push_back(m_vertices[i - 1]);

					for (size_t t=0; t<m_texcoords.size(); t++)
					{
						finalTexcoords[t].push_back(m_texcoords[t][0]);
						finalTexcoords[t].push_back(m_texcoords[t][i]);
						finalTexcoords[t].push_back(m_texcoords[t][i - 1]);
					}

					if (colors.size() > 0)
					{
						finalColors.push_back(colors[clamp<int>(0, 0, maxColorIndex)]);
						finalColors.push_back(colors[clamp<int>(i, 0, maxColorIndex)]);
						finalColors.push_back(colors[clamp<int>(i - 1, 0, maxColorIndex)]);
					}
				}
			}
		}

		// build directx vertices
		{
			m_convertedVertices.resize(finalVertices.size());

			m_iNumVertices = m_convertedVertices.size(); // NOTE: overwrite m_iNumVertices for potential conversions

			const bool hasColors = (finalColors.size() > 0);
			const bool hasTexCoords0 = (finalTexcoords.size() > 0 && finalTexcoords[0].size() == m_convertedVertices.size());

			for (size_t i=0; i<finalVertices.size(); i++)
			{
				m_convertedVertices[i].pos.x = finalVertices[i].x;
				m_convertedVertices[i].pos.y = finalVertices[i].y;
				m_convertedVertices[i].pos.z = finalVertices[i].z;

				if (hasColors)
					m_convertedVertices[i].col = finalColors[clamp<size_t>(i, 0, maxColorIndex)];
				else
				{
					m_convertedVertices[i].col.x = 1.0f;
					m_convertedVertices[i].col.y = 1.0f;
					m_convertedVertices[i].col.z = 1.0f;
					m_convertedVertices[i].col.w = 1.0f;
				}

				// TODO: multitexturing
				if (hasTexCoords0)
					m_convertedVertices[i].tex = finalTexcoords[0][i];
			}
		}
	}

	// create buffer
	const D3D11_USAGE usage = (D3D11_USAGE)usageToDirectX(m_usage);
	{
		D3D11_BUFFER_DESC bufferDesc;
		{
			bufferDesc.Usage = usage;
			bufferDesc.ByteWidth = sizeof(DirectX11Interface::SimpleVertex) * m_convertedVertices.size();
			bufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.CPUAccessFlags = (bufferDesc.Usage == D3D11_USAGE::D3D11_USAGE_DYNAMIC ? D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE : 0);
			bufferDesc.MiscFlags = 0;
			bufferDesc.StructureByteStride = 0;
		}
		D3D11_SUBRESOURCE_DATA dataForImmutable;
		{
			dataForImmutable.pSysMem = &m_convertedVertices[0];
			dataForImmutable.SysMemPitch = 0; // (unused for vertices)
			dataForImmutable.SysMemSlicePitch = 0; // (unused for vertices)
		}
		if (FAILED(dx11->getDevice()->CreateBuffer(&bufferDesc, (bufferDesc.Usage == D3D11_USAGE::D3D11_USAGE_IMMUTABLE ? &dataForImmutable : NULL), &m_vertexBuffer))) // NOTE: immutable is uploaded to gpu right here
		{
			debugLog("DirectX Error: Couldn't CreateBuffer(%i)\n", (int)m_convertedVertices.size());
			return;
		}
	}

	// upload everything to gpu
	if (usage == D3D11_USAGE::D3D11_USAGE_DEFAULT)
	{
		D3D11_BOX box;
		{
			box.left = sizeof(DirectX11Interface::SimpleVertex) * 0;
			box.right = box.left + (sizeof(DirectX11Interface::SimpleVertex) * m_convertedVertices.size());
			box.top = 0;
			box.bottom = 1;
			box.front = 0;
			box.back = 1;
		}
		dx11->getDeviceContext()->UpdateSubresource(m_vertexBuffer, 0, &box, &m_convertedVertices[0], 0, 0);
	}
	else if (usage == D3D11_USAGE::D3D11_USAGE_DYNAMIC)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		if (SUCCEEDED(dx11->getDeviceContext()->Map(m_vertexBuffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		{
			memcpy(mappedResource.pData, &m_convertedVertices[0], sizeof(DirectX11Interface::SimpleVertex) * m_convertedVertices.size());
			dx11->getDeviceContext()->Unmap(m_vertexBuffer, 0);
		}
		else
		{
			debugLog("DirectX Error: Couldn't Map(%i) vertexbuffer\n", (int)m_convertedVertices.size());
			return;
		}
	}

	// free memory
	if (!m_bKeepInSystemMemory)
	{
		clear();
		m_convertedVertices = std::vector<DirectX11Interface::SimpleVertex>();
	}

	m_bReady = true;
}

void DirectX11VertexArrayObject::initAsync()
{
	m_bAsyncReady = true;
}

void DirectX11VertexArrayObject::destroy()
{
	VertexArrayObject::destroy();

	if (m_vertexBuffer != NULL)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = NULL;
	}

	m_convertedVertices = std::vector<DirectX11Interface::SimpleVertex>();
}

void DirectX11VertexArrayObject::draw()
{
	if (!m_bReady)
	{
		debugLog("WARNING: DirectX11VertexArrayObject::draw() called, but was not ready!\n");
		return;
	}

	const int start = clamp<int>(m_iDrawRangeFromIndex > -1 ? m_iDrawRangeFromIndex : nearestMultipleUp((int)(m_iNumVertices*m_fDrawPercentFromPercent), m_iDrawPercentNearestMultiple), 0, m_iNumVertices);
	const int end = clamp<int>(m_iDrawRangeToIndex > -1 ? m_iDrawRangeToIndex : nearestMultipleDown((int)(m_iNumVertices*m_fDrawPercentToPercent), m_iDrawPercentNearestMultiple), 0, m_iNumVertices);

	if (start > end || std::abs(end - start) == 0) return;

	const DirectX11Interface *dx11 = dynamic_cast<DirectX11Interface*>(engine->getGraphics());

	// draw it
	{
		const UINT stride = sizeof(DirectX11Interface::SimpleVertex);
		const UINT offset = 0;

		dx11->getDeviceContext()->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
		dx11->getDeviceContext()->IASetPrimitiveTopology((D3D_PRIMITIVE_TOPOLOGY)primitiveToDirectX(m_convertedPrimitive));
		dx11->getDeviceContext()->Draw(end - start, start);
	}
}

int DirectX11VertexArrayObject::primitiveToDirectX(Graphics::PRIMITIVE primitive)
{
	switch (primitive)
	{
	case Graphics::PRIMITIVE::PRIMITIVE_LINES:
		return D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	case Graphics::PRIMITIVE::PRIMITIVE_LINE_STRIP:
		return D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
	case Graphics::PRIMITIVE::PRIMITIVE_TRIANGLES:
		return D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case Graphics::PRIMITIVE::PRIMITIVE_TRIANGLE_FAN:	// NOTE: not available! -------------------
		return D3D_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case Graphics::PRIMITIVE::PRIMITIVE_TRIANGLE_STRIP:
		return D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	case Graphics::PRIMITIVE::PRIMITIVE_QUADS:			// NOTE: not available! -------------------
		return D3D_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}

	return D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

int DirectX11VertexArrayObject::usageToDirectX(Graphics::USAGE_TYPE usage)
{
	switch (usage)
	{
	case Graphics::USAGE_TYPE::USAGE_STATIC:
		return D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
	case Graphics::USAGE_TYPE::USAGE_DYNAMIC:
		return D3D11_USAGE::D3D11_USAGE_DEFAULT; // NOTE: this is intentional. no performance benefits found so far with DYNAMIC, since D3D11_MAP_WRITE_NO_OVERWRITE has very limited use cases
	case Graphics::USAGE_TYPE::USAGE_STREAM:
		return D3D11_USAGE::D3D11_USAGE_DEFAULT;
	}

	return D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
}

#endif
