#pragma once

template <class BufferType>
class ConstantBuffer
{
public:
	ConstantBuffer(ID3D11Device* const pDevice, BufferType&& data, D3D11_USAGE usage = D3D11_USAGE_IMMUTABLE, UINT cpuAccessFlags = 0u)
		: bufferStruct({ data })
	{
		Init(pDevice, usage, cpuAccessFlags);
	}
	ConstantBuffer(ID3D11Device* const pDevice, D3D11_USAGE usage = D3D11_USAGE_DYNAMIC, UINT cpuAccessFlags = D3D11_CPU_ACCESS_WRITE)
		: bufferStruct()
	{
		Init(pDevice, usage, cpuAccessFlags);
	}
	ConstantBuffer() {} // need to manually initialize
	~ConstantBuffer() = default;
	ROF_DELETE(ConstantBuffer);

public:
	void Init(ID3D11Device* const pDevice, D3D11_USAGE usage = D3D11_USAGE_DYNAMIC, UINT cpuAccessFlags = D3D11_CPU_ACCESS_WRITE)
	{
		D3D11_BUFFER_DESC cbDesc = {};
		cbDesc.ByteWidth = sizeof(BufferStruct);
		cbDesc.Usage = usage;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = cpuAccessFlags;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		// Fill in the subresource data.
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = &bufferStruct;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		// Create the buffer.
		HRESULT hr = pDevice->CreateBuffer(&cbDesc, &initData, pBuffer.GetAddressOf());
		if (FAILED(hr)) throw std::runtime_error("Could not create constant buffer");
	}

	inline void Update(ID3D11DeviceContext* const pDeviceContext)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		//  Disable GPU access to the vertex buffer data.
		pDeviceContext->Map(pBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedResource);
		//  Update the vertex buffer here.
		memcpy(mappedResource.pData, &bufferStruct, sizeof(BufferStruct));
		//  Reenable GPU access to the vertex buffer data.
		pDeviceContext->Unmap(pBuffer.Get(), 0u);
	}
	inline void Update(ID3D11DeviceContext* const pDeviceContext, BufferType data)
	{
		bufferStruct.data = data;
		Update(pDeviceContext);
	}

	inline BufferType& GetData() { return bufferStruct.data; }
	inline ID3D11Buffer* GetBuffer() { return pBuffer.Get(); }
	inline ID3D11Buffer** GetBufferAddress() { return pBuffer.GetAddressOf(); }

private:
	struct alignas(16) BufferStruct { BufferType data; } bufferStruct; // 16-byte aligned
	Microsoft::WRL::ComPtr<ID3D11Buffer> pBuffer;
};
typedef ConstantBuffer<DirectX::XMMATRIX> ConstantBufferMat;