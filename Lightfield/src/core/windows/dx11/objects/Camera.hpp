#pragma once

#include "Transform.hpp"

class Camera
{
public:
	Camera(ID3D11Device* const pDevice) : transform(pDevice, true) 
	{
        projMatStruct.ProjectionMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(1.25f, 1.777777f, 0.1f, 1000.0f));

        D3D11_BUFFER_DESC cbDesc = {};
        cbDesc.ByteWidth = sizeof(BufferStruct);
        cbDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        cbDesc.MiscFlags = 0;
        cbDesc.StructureByteStride = 0;

        // Repeat for proj matrix
        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = &projMatStruct;
        initData.SysMemPitch = 0;
        initData.SysMemSlicePitch = 0;

        HRESULT hr = pDevice->CreateBuffer(&cbDesc, &initData, buffer.GetAddressOf());
        if (FAILED(hr)) throw std::runtime_error("Could not create camera projection buffer");
	}
	~Camera() = default;
	ROF_DELETE(Camera);

public:
    inline ID3D11Buffer* const GetPerspMatBuffer() { return buffer.Get(); }
    inline ID3D11Buffer* const GetViewMatBuffer(ID3D11DeviceContext* const pDeviceContext) { return transform.GetBuffer(pDeviceContext); }

private:
    struct BufferStruct { DirectX::XMMATRIX ProjectionMatrix; } projMatStruct;
    Microsoft::WRL::ComPtr<ID3D11Buffer> buffer; // holds perspective matrix
	Transform transform;
};