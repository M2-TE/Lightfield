#pragma once

class TextureBase 
{
protected:
	inline void CheckHR(HRESULT hr)
	{
		if (FAILED(hr)) throw std::runtime_error("Texture creation failed");
	}
};

class Texture2D : TextureBase
{
public:
	Texture2D() = default;
	~Texture2D() = default;
	ROF_DELETE(Texture2D);

	void CreateTexture(ID3D11Device* const pDevice, D3D11_TEXTURE2D_DESC desc, D3D11_SUBRESOURCE_DATA* data = nullptr)
	{
		pTex.Reset();
		descTex = desc;
		HRESULT hr = pDevice->CreateTexture2D(&descTex.value(), data, pTex.GetAddressOf());
		CheckHR(hr);
	}
	void CreateRTV(ID3D11Device* const pDevice, D3D11_RENDER_TARGET_VIEW_DESC desc)
	{
		pRTV.Reset();
		descRTV = desc;
		HRESULT hr = pDevice->CreateRenderTargetView(pTex.Get(), &descRTV.value(), pRTV.GetAddressOf());
		CheckHR(hr);
	}
	void CreateSRV(ID3D11Device* const pDevice, D3D11_SHADER_RESOURCE_VIEW_DESC desc)
	{
		pSRV.Reset();
		descSRV = desc;
		HRESULT hr = pDevice->CreateShaderResourceView(pTex.Get(), &descSRV.value(), pSRV.GetAddressOf());
		CheckHR(hr);
	}
	void CreateUAV(ID3D11Device* const pDevice, D3D11_UNORDERED_ACCESS_VIEW_DESC desc)
	{
		pUAV.Reset();
		descUAV = desc;
		HRESULT hr = pDevice->CreateUnorderedAccessView(pTex.Get(), &descUAV.value(), pUAV.GetAddressOf());
		CheckHR(hr);
	}

	inline ID3D11Texture2D* GetTex() { return pTex.Get(); }
	inline ID3D11RenderTargetView* GetRTV() { return pRTV.Get(); }
	inline ID3D11ShaderResourceView* GetSRV() { return pSRV.Get(); }
	inline ID3D11UnorderedAccessView* GetUAV() { return pUAV.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pTex;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pUAV;

	std::optional<D3D11_TEXTURE2D_DESC> descTex;
	std::optional<D3D11_RENDER_TARGET_VIEW_DESC> descRTV;
	std::optional<D3D11_SHADER_RESOURCE_VIEW_DESC> descSRV;
	std::optional<D3D11_UNORDERED_ACCESS_VIEW_DESC> descUAV;
};