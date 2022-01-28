#pragma once

class DepthStencil
{
public:
	DepthStencil() = default;
	~DepthStencil() = default;
	ROF_DELETE(DepthStencil);

	void Init(ID3D11Device* const pDevice, UINT width, UINT height)
	{

		HRESULT hr;

		// create depth stensil texture
		{
			D3D11_TEXTURE2D_DESC descDepth = {};
			descDepth.Width = width;
			descDepth.Height = height;
			descDepth.MipLevels = 1u;
			descDepth.ArraySize = 1u;
			descDepth.Format = DXGI_FORMAT_R24G8_TYPELESS; // typeless required for both read and write
			descDepth.SampleDesc.Count = 1u;
			descDepth.SampleDesc.Quality = 0u;
			descDepth.Usage = D3D11_USAGE_DEFAULT;
			descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; // for write and read
			pDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencilTex);
		}

		// create view of depth stensil texture
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
			descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // cast typeless to typed format for view
			descDSV.Texture2D.MipSlice = 0u;
			hr = pDevice->CreateDepthStencilView(pDepthStencilTex.Get(), &descDSV, pDepthStencilView.GetAddressOf());
			if (FAILED(hr)) throw std::runtime_error("Depth stencil view creation failed");
		}

		// create shader resource view of depth stencil texture
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			desc.Texture2D = { 0u, 1u };

			desc.Format = DXGI_FORMAT_X24_TYPELESS_G8_UINT; // read stencil only
			hr = pDevice->CreateShaderResourceView(pDepthStencilTex.Get(), &desc, pStencilSRV.GetAddressOf());
			if (FAILED(hr)) throw std::runtime_error("Stencil SRV creation failed");


			desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; // read depth only
			hr = pDevice->CreateShaderResourceView(pDepthStencilTex.Get(), &desc, pDepthSRV.GetAddressOf());
			if (FAILED(hr)) throw std::runtime_error("Depth SRV creation failed");
		}
	}

	inline void ClearDepthStencil(ID3D11DeviceContext* const pDeviceContext)
	{
		pDeviceContext->ClearDepthStencilView(
			pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);
	}
	inline ID3D11DepthStencilView* const GetView() const
	{
		return pDepthStencilView.Get();
	}
	inline ID3D11ShaderResourceView* const GetStencilSRV() const
	{
		return pStencilSRV.Get();
	}
	inline ID3D11ShaderResourceView* const GetDepthSRV() const
	{
		return pDepthSRV.Get();
	}
private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pStencilSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pDepthSRV;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	pDepthStencilView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthStencilTex;
};