#pragma once

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
	void SetAsCube(ID3D11Device* const pDevice)
	{
		// TODO
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
		float p = 1.0f, n = -1.0f;
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