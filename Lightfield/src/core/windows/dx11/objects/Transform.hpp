#pragma once

class Transform
{
public:
	Transform(ID3D11Device* const pDevice, bool bInverted = false) : Transform(pDevice, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, bInverted) {}
	Transform(ID3D11Device* const pDevice, DirectX::XMFLOAT3A pos, DirectX::XMFLOAT3A rot, DirectX::XMFLOAT3A scale_param, bool bInverted = false) :
		rotation(DirectX::XMQuaternionRotationRollPitchYaw(rot.x, rot.y, rot.z)),
		position(DirectX::XMLoadFloat3A(&pos)), scale(DirectX::XMLoadFloat3A(&scale_param)),
		bufferStruct(), bDirty(true), bInverted(bInverted)
	{
		D3D11_BUFFER_DESC cbDesc = {};
		cbDesc.ByteWidth = sizeof(BufferStruct);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		// Fill in the subresource data.
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = &bufferStruct;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		// Create the buffer.
		HRESULT hr = pDevice->CreateBuffer(&cbDesc, &initData,
			pModelMatBuffer.GetAddressOf());
		if (FAILED(hr)) throw std::runtime_error("Could not create transform buffer");
	}
	ROF_DELETE(Transform);

public:
	// Local transform Getters
	DirectX::XMFLOAT3A GetPosition() const
	{
		DirectX::XMFLOAT3A vec;
		DirectX::XMStoreFloat3A(&vec, position);
		return vec;
	}
	DirectX::XMFLOAT4A GetRotation() const
	{
		DirectX::XMFLOAT4A vec;
		DirectX::XMStoreFloat4A(&vec, rotation);
		return vec;
	}
	DirectX::XMFLOAT3A GetScale() const
	{
		DirectX::XMFLOAT3A vec;
		DirectX::XMStoreFloat3A(&vec, scale);
		return vec;
	}

	// Local transform Setters
	void SetPosition(const float x, const float y, const float z)
	{
		bDirty = true;
		DirectX::XMFLOAT3A positionBase = { x, y, z };
		position = DirectX::XMLoadFloat3A(&positionBase);
	}
	void SetRotation(const float x, const float y, const float z, const float w)
	{
		bDirty = true;
		DirectX::XMFLOAT4A rotationBase = { x, y, z, w };
		rotation = DirectX::XMLoadFloat4A(&rotationBase);
	}
	void SetRotationEuler(const float x, const float y, const float z)
	{
		bDirty = true;
		rotation = DirectX::XMQuaternionRotationRollPitchYaw(x, y, z);
	}
	void SetRotationEulerImmediate(ID3D11DeviceContext* const pDeviceContext, const float x, const float y, const float z)
	{
		bDirty = true;
		DirectX::XMFLOAT3A rotationEulerBase = { x, y, z };
		rotation = DirectX::XMQuaternionRotationRollPitchYaw(rotationEulerBase.x, rotationEulerBase.y, rotationEulerBase.z);

		if (bDirty) UpdateTransformMatrix(pDeviceContext);
	}
	void SetScale(const float x, const float y, const float z)
	{
		bDirty = true;
		DirectX::XMFLOAT3A scaleBase = { x, y, z };
		scale = DirectX::XMLoadFloat3A(&scaleBase);
	}

	// Adding offset to current transform vectors
	void Translate(const float x, const float y, const float z)
	{
		bDirty = true;
		DirectX::XMFLOAT3A translationBase = { x, y, z };
		position = DirectX::XMVectorAdd(DirectX::XMLoadFloat3A(&translationBase), position);
	}
	void RotateEuler(const float x, const float y, const float z)
	{
		bDirty = true;
		rotation = DirectX::XMQuaternionMultiply(rotation,
			DirectX::XMQuaternionRotationRollPitchYaw(x, y, z));
	}

	// Local directional vectors
	DirectX::XMFLOAT3A GetRight()
	{
		DirectX::XMFLOAT3A vec;
		const DirectX::XMVECTOR tempRight = { 1.0f, 0.0f, 0.0f, 0.0f };
		DirectX::XMStoreFloat3A(&vec, DirectX::XMVector3Rotate(tempRight, rotation));
		return vec;
	}
	DirectX::XMFLOAT3A GetUp()
	{
		DirectX::XMFLOAT3A vec;
		const DirectX::XMVECTOR tempUp = { 0.0f, 1.0f, 0.0f, 0.0f };
		DirectX::XMStoreFloat3A(&vec, DirectX::XMVector3Rotate(tempUp, rotation));
		return vec;
	}
	DirectX::XMFLOAT3A GetForward()
	{
		DirectX::XMFLOAT3A vec;
		const DirectX::XMVECTOR tempForward = { 0.0f, 0.0f, 1.0f, 0.0f };
		DirectX::XMStoreFloat3A(&vec, DirectX::XMVector3Rotate(tempForward, rotation));
		return vec;
	}

	// Bind to render pipeline or get buffer directly
	inline void Bind(ID3D11DeviceContext* const pDeviceContext)
	{
		if (bDirty) UpdateTransformMatrix(pDeviceContext);

		pDeviceContext->VSSetConstantBuffers(2u, 1u, pModelMatBuffer.GetAddressOf());
		//pDeviceContext->PSSetConstantBuffers(0u, 1u, pModelMatBuffer.GetAddressOf());
	}
	inline ID3D11Buffer* const GetBuffer(ID3D11DeviceContext* const pDeviceContext)
	{
		if (bDirty) UpdateTransformMatrix(pDeviceContext);
		return pModelMatBuffer.Get();
	}
	inline ID3D11Buffer** const GetBufferAddress(ID3D11DeviceContext* const pDeviceContext)
	{
		if (bDirty) UpdateTransformMatrix(pDeviceContext);
		return pModelMatBuffer.GetAddressOf();
	}

private:
	inline void UpdateTransformMatrix(ID3D11DeviceContext* const pDeviceContext)
	{

		bufferStruct.ModelMatrix = DirectX::XMMatrixMultiplyTranspose(
			DirectX::XMMatrixMultiply(
				DirectX::XMMatrixScalingFromVector(scale),
				DirectX::XMMatrixRotationQuaternion(rotation)),
			DirectX::XMMatrixTranslationFromVector(position));

		if (bInverted)
		{
			// invert matrix
			bufferStruct.ModelMatrix = DirectX::XMMatrixInverse(nullptr, bufferStruct.ModelMatrix);
		}

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		//  Disable GPU access to the vertex buffer data.
		pDeviceContext->Map(pModelMatBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedResource);
		//  Update the vertex buffer here.
		memcpy(mappedResource.pData, &bufferStruct, sizeof(BufferStruct));
		//  Reenable GPU access to the vertex buffer data.
		pDeviceContext->Unmap(pModelMatBuffer.Get(), 0u);

		bDirty = false;
	}

private:
	struct BufferStruct { DirectX::XMMATRIX ModelMatrix; } bufferStruct;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pModelMatBuffer;

	DirectX::XMVECTOR position;
	DirectX::XMVECTOR rotation; // quaternion
	DirectX::XMVECTOR scale;

	const bool bInverted; // useful for e.g. camera transforms
	bool bDirty;
};