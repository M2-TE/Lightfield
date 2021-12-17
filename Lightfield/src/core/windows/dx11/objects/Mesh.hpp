#pragma once

// TODO: pack this into .cpp to hide from codebase
#define TINYOBJLOADER_IMPLEMENTATION
//#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include "tinyobjloader/tiny_obj_loader.h"

enum class Primitive { Cube, Sphere, Quad };
class Mesh
{
public:
	Mesh(ID3D11Device* const pDevice, Primitive primitive) 
	{
		switch (primitive)
		{
			case Primitive::Cube: SetAsCube(pDevice); break;
			case Primitive::Sphere: SetAsSphere(pDevice); break;
			case Primitive::Quad: SetAsQuad(pDevice); break;
		}

		CreateBuffer(pDevice);
	}
	Mesh(ID3D11Device* const pDevice, std::string fileName) {

		LoadObj(fileName);
		CreateBuffer(pDevice);
	}
	~Mesh() = default;
	ROF_DELETE(Mesh);

public:
	void Draw(ID3D11DeviceContext* const pDeviceContext)
	{
		static constexpr UINT offset = 0u;
		static constexpr UINT startSlot = 0u;
		static constexpr UINT numBuffers = 1u;

		pDeviceContext->IASetVertexBuffers(
			startSlot, numBuffers,
			pVertexBuffer.GetAddressOf(),
			&vertexSize, &offset);

		pDeviceContext->IASetIndexBuffer(
			pIndexBuffer.Get(),
			DXGI_FORMAT_R32_UINT,
			offset);

		pDeviceContext->DrawIndexed(indexCount, 0u, 0u);
	}

private:
	void LoadObj(std::string fileName) {

		tinyobj::ObjReaderConfig readerConfig;
		readerConfig.mtl_search_path = "./"; // Path to material files

		fileName = "data/objs/" + fileName + "/" + fileName + ".obj";

		tinyobj::ObjReader reader;
		bool success = reader.ParseFromFile(fileName, readerConfig);
		const std::string& error = reader.Error();
		const std::string& warning = reader.Warning();
		if (!success) __debugbreak(); // TODO: put actual warning/error message into console

		auto& attrib = reader.GetAttrib();
		auto& shapes = reader.GetShapes();
		auto& materials = reader.GetMaterials();

		// TODO: reserve space for vertices, indices
		// TODO: use the sample values to actually fill the vertex and index buffers

		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces(polygon)
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++) {

					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					Vertex vertex = {};

					vertex.pos.x = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
					vertex.pos.y = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
					vertex.pos.z = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
					vertex.pos.w = 1.0f;

					// Check if `normal_index` is zero or positive. negative = no normal data
					if (idx.normal_index >= 0) {
						vertex.norm.x = attrib.normals[3 * size_t(idx.normal_index) + 0];
						vertex.norm.y = attrib.normals[3 * size_t(idx.normal_index) + 1];
						vertex.norm.z = attrib.normals[3 * size_t(idx.normal_index) + 2];
						vertex.norm.w = 0.0f;
					}

					// Check if `texcoord_index` is zero or positive. negative = no texcoord data
					if (idx.texcoord_index >= 0) {
						// TODO
						tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
						tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
					}

					// Optional: vertex colors
					vertex.col = { 1.0f, 1.0f, 1.0f, 1.0f };
					// tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
					// tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
					// tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];

					vertices.push_back(vertex);
					indices.push_back(vertices.size() - 1);
				}
				index_offset += fv;

				// per-face material
				shapes[s].mesh.material_ids[f];
			}
		}
	}
	void SetAsCube(ID3D11Device* const pDevice)
	{
		DirectX::XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		static constexpr float p = .5f, z = 0.0f, n = -.5f;
		// Normals
		static constexpr DirectX::XMFLOAT4 right	= { 1.0f, z, z, z };
		static constexpr DirectX::XMFLOAT4 up		= { z, 1.0f, z, z };
		static constexpr DirectX::XMFLOAT4 fwd		= { z, z, 1.0f, z };
		static constexpr DirectX::XMFLOAT4 left		= { -1.0f, z, z, z };
		static constexpr DirectX::XMFLOAT4 down		= { z, -1.0f, z, z };
		static constexpr DirectX::XMFLOAT4 bwd		= { z, z, -1.0f, z };
		// Vertex Positions
		static constexpr DirectX::XMFLOAT4 lftTopFwd = { n, p, p, 1.0f };
		static constexpr DirectX::XMFLOAT4 rgtTopFwd = { p, p, p, 1.0f };
		static constexpr DirectX::XMFLOAT4 lftTopBwd = { n, p, n, 1.0f };
		static constexpr DirectX::XMFLOAT4 rgtTopBwd = { p, p, n, 1.0f };
		static constexpr DirectX::XMFLOAT4 lftBotFwd = { n, n, p, 1.0f };
		static constexpr DirectX::XMFLOAT4 rgtBotFwd = { p, n, p, 1.0f };
		static constexpr DirectX::XMFLOAT4 lftBotBwd = { n, n, n, 1.0f };
		static constexpr DirectX::XMFLOAT4 rgtBotBwd = { p, n, n, 1.0f };

		vertices = {

			{ rgtTopBwd, right, col },
			{ rgtTopFwd, right, col },
			{ rgtBotBwd, right, col },
			{ rgtBotFwd, right, col },

			{ lftTopFwd, up, col },
			{ rgtTopFwd, up, col },
			{ lftTopBwd, up, col },
			{ rgtTopBwd, up, col },

			{ rgtTopFwd, fwd, col },
			{ lftTopFwd, fwd, col },
			{ rgtBotFwd, fwd, col },
			{ lftBotFwd, fwd, col },

			{ lftTopFwd, left, col },
			{ lftTopBwd, left, col },
			{ lftBotFwd, left, col },
			{ lftBotBwd, left, col },

			{ rgtBotFwd, down, col },
			{ lftBotFwd, down, col },
			{ rgtBotBwd, down, col },
			{ lftBotBwd, down, col },

			{ lftTopBwd, bwd, col },
			{ rgtTopBwd, bwd, col },
			{ lftBotBwd, bwd, col },
			{ rgtBotBwd, bwd, col }
		};

		indices.reserve(6u * 4u);
		for (UINT i = 0u; i < 6u * 4u; i += 4u) {
			indices.insert(indices.end(), { 0 + i, 1 + i, 2 + i, 1 + i, 3 + i, 2 + i });
		}
	}
	void SetAsSphere(ID3D11Device* const pDevice)
	{
		// TODO
	}
	void SetAsQuad(ID3D11Device* const pDevice)
	{
		DirectX::XMFLOAT4 norm = { 0.0f, 0.0f, -1.0f, 0.0f };
		DirectX::XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };

		float depth = 0.0f;
		float p = .5f, n = -.5f;
		DirectX::XMFLOAT4 topLeft =		{ n, p, depth, 1.0f };
		DirectX::XMFLOAT4 topRight =	{ p, p, depth, 1.0f };
		DirectX::XMFLOAT4 botLeft =		{ n, n, depth, 1.0f };
		DirectX::XMFLOAT4 botRight =	{ p, n, depth, 1.0f };

		vertices = {
			{ topLeft, norm, col },
			{ topRight, norm, col },
			{ botLeft, norm, col },
			{ botRight, norm, col }
		};
		indices = {
			0, 1, 2,
			1, 3, 2
		};
	}

	void CreateBuffer(ID3D11Device* const pDevice)
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		D3D11_SUBRESOURCE_DATA bufferData = {};

		// Fill vertex buffer
		{
			vertexCount = static_cast<UINT>(vertices.size());

			bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			bufferDesc.ByteWidth = vertexSize * vertexCount;
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.CPUAccessFlags = 0u;
			bufferDesc.MiscFlags = 0u;

			bufferData.pSysMem = vertices.data();
			bufferData.SysMemPitch = 0u;
			bufferData.SysMemSlicePitch = 0u;

			pDevice->CreateBuffer(&bufferDesc, &bufferData, &pVertexBuffer);
		}

		// Fill index buffer
		{
			indexCount = static_cast<UINT>(indices.size());

			// Fill in a buffer description.
			bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			bufferDesc.ByteWidth = sizeof(Index) * indexCount;
			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bufferDesc.CPUAccessFlags = 0u;
			bufferDesc.MiscFlags = 0u;

			// Define the resource data.
			bufferData.pSysMem = indices.data();
			bufferData.SysMemPitch = 0u;
			bufferData.SysMemSlicePitch = 0u;

			// Create the buffer with the device.
			pDevice->CreateBuffer(&bufferDesc, &bufferData, &pIndexBuffer);
		}
	}

private:
	struct Vertex { DirectX::XMFLOAT4 pos; DirectX::XMFLOAT4 norm; DirectX::XMFLOAT4 col; };
	typedef uint32_t Index;

	Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer, pIndexBuffer;
	std::vector<Vertex> vertices;
	std::vector<Index> indices;
	static constexpr UINT vertexSize = sizeof(Vertex);
	UINT vertexCount, indexCount;
};