#pragma once

// container to hold all potential textures for objects
struct TexturePack {
	std::unique_ptr<Texture2D> alpha_tex;
	std::unique_ptr<Texture2D> ambient_tex;
	std::unique_ptr<Texture2D> bump_tex;
	std::unique_ptr<Texture2D> diffuse_tex;
	std::unique_ptr<Texture2D> displacement_tex;
	std::unique_ptr<Texture2D> emissive_tex;
	std::unique_ptr<Texture2D> metallic_tex;
	std::unique_ptr<Texture2D> normal_tex;
	std::unique_ptr<Texture2D> reflection_tex;
};
// Vertex type used for standard meshes
struct Vertex 
{ 
	DirectX::XMFLOAT4 pos; 
	DirectX::XMFLOAT4 norm; 
	DirectX::XMFLOAT4 col; 
	DirectX::XMFLOAT4 uvCoords;
};
// Default index type
typedef uint32_t Index;

struct Submesh
{
	Submesh() {}
	~Submesh() {}
	ROF_DELETE(Submesh);

	void Draw(ID3D11DeviceContext* const pDeviceContext)
	{
		static constexpr UINT offset = 0u;
		static constexpr UINT startSlot = 0u;
		static constexpr UINT numBuffers = 1u;
		static constexpr UINT vertexSize = sizeof(Vertex);

		pDeviceContext->IASetVertexBuffers(
			startSlot, numBuffers,
			pVertexBuffer.GetAddressOf(),
			&vertexSize, &offset);

		pDeviceContext->IASetIndexBuffer(
			pIndexBuffer.Get(),
			DXGI_FORMAT_R32_UINT,
			offset);

		if (texturePack.diffuse_tex.get() != nullptr) {
			pDeviceContext->PSSetShaderResources(0u, 1u, texturePack.diffuse_tex->GetSRVAddress());
		}
		pDeviceContext->DrawIndexed(indexCount, 0u, 0u);
	}

	void SetAsCube(ID3D11Device* const pDevice)
	{
		DirectX::XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		static constexpr float p = .5f, z = 0.0f, n = -.5f;
		// Normals
		static constexpr DirectX::XMFLOAT4 right = { 1.0f, z, z, z };
		static constexpr DirectX::XMFLOAT4 up = { z, 1.0f, z, z };
		static constexpr DirectX::XMFLOAT4 fwd = { z, z, 1.0f, z };
		static constexpr DirectX::XMFLOAT4 left = { -1.0f, z, z, z };
		static constexpr DirectX::XMFLOAT4 down = { z, -1.0f, z, z };
		static constexpr DirectX::XMFLOAT4 bwd = { z, z, -1.0f, z };
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
		DirectX::XMFLOAT4 topLeft = { n, p, depth, 1.0f };
		DirectX::XMFLOAT4 topRight = { p, p, depth, 1.0f };
		DirectX::XMFLOAT4 botLeft = { n, n, depth, 1.0f };
		DirectX::XMFLOAT4 botRight = { p, n, depth, 1.0f };

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
			static constexpr UINT vertexSize = sizeof(Vertex);
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

	Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer, pIndexBuffer;
	std::vector<Vertex> vertices;
	std::vector<Index> indices;
	UINT vertexCount, indexCount;
	TexturePack texturePack;
};