#pragma once

class Transform
{
public:
	Transform(ID3D11Device* const pDevice) : Transform(pDevice, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }) {}
	Transform(ID3D11Device* const pDevice, DirectX::XMFLOAT3A pos, DirectX::XMFLOAT3A rot, DirectX::XMFLOAT3A scale_param) :
		rotation(DirectX::XMQuaternionRotationRollPitchYaw(rot.x, rot.y, rot.z)),
		position(DirectX::XMLoadFloat3A(&pos)), scale(DirectX::XMLoadFloat3A(&scale_param)),
		bDirty(false), cbuffer(pDevice, CalcMatrix(true), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE),
		inverseCbuffer(pDevice, CalcMatrix(false), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE)
	{
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

	// Get Pointer to buffers
	inline ConstantBufferMat& GetBuffer(ID3D11DeviceContext* const pDeviceContext)
	{
		if (bDirty) UpdateTransformMatrix(pDeviceContext);
		return cbuffer;
	}
	inline ConstantBufferMat& GetInverseBuffer(ID3D11DeviceContext* const pDeviceContext)
	{
		if (bDirty) UpdateTransformMatrix(pDeviceContext);
		return inverseCbuffer;
	}

private:
	inline void UpdateTransformMatrix(ID3D11DeviceContext* const pDeviceContext)
	{
		cbuffer.GetData() = CalcMatrix(false);
		cbuffer.Update(pDeviceContext);
		inverseCbuffer.GetData() = CalcMatrix(true);
		inverseCbuffer.Update(pDeviceContext);
		bDirty = false;
	}
	inline DirectX::XMMATRIX CalcMatrix(bool bInverted = false)
	{
		auto mat = DirectX::XMMatrixMultiplyTranspose(
			DirectX::XMMatrixMultiply(
				DirectX::XMMatrixScalingFromVector(scale),
				DirectX::XMMatrixRotationQuaternion(rotation)),
			DirectX::XMMatrixTranslationFromVector(position));
		if (bInverted) mat = DirectX::XMMatrixInverse(nullptr, mat);
		return mat;
	}

private:
	DirectX::XMVECTOR position;
	DirectX::XMVECTOR rotation; // quaternion
	DirectX::XMVECTOR scale;

	bool bDirty;

	ConstantBufferMat cbuffer;
	ConstantBufferMat inverseCbuffer;
};