#pragma once

#include "Transform.hpp"

class Camera
{
public:
	Camera(ID3D11Device* const pDevice) : transform(pDevice), posCbuffer(pDevice),
		cbuffer(
			pDevice, DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(1.25f, 1.777777f, 0.1f, 100.0f)),
			D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE)
	{
	}
	~Camera() = default;
	ROF_DELETE(Camera);

public:
	inline void UpdatePos(ID3D11DeviceContext* const pDeviceContext)
	{
		posCbuffer.GetData() = transform.GetPosition();
		posCbuffer.Update(pDeviceContext);
	}
	inline Transform& GetTransform() { return transform; }
	inline ConstantBufferMat& GetProjectionBuffer() { return cbuffer; }
	inline ConstantBuffer<DirectX::XMFLOAT3A>& GetPosBuffer() { return posCbuffer; }

private:
	Transform transform;
	ConstantBuffer<DirectX::XMFLOAT3A> posCbuffer;
	ConstantBufferMat cbuffer;
};