#pragma once

class TextureBase
{
public:
	void CreateRTV(ID3D11Device* const pDevice, D3D11_RENDER_TARGET_VIEW_DESC desc)
	{
		pRTV.Reset();
		descRTV = desc;
		HRESULT hr = pDevice->CreateRenderTargetView(GetTexRes(), &descRTV.value(), pRTV.GetAddressOf());
		if (FAILED(hr)) throw std::runtime_error("Texture RTV creation failed");
	}
	void CreateSRV(ID3D11Device* const pDevice, D3D11_SHADER_RESOURCE_VIEW_DESC desc)
	{
		pSRV.Reset();
		descSRV = desc;
		HRESULT hr = pDevice->CreateShaderResourceView(GetTexRes(), &descSRV.value(), pSRV.GetAddressOf());
		if (FAILED(hr)) throw std::runtime_error("Texture SRV creation failed");
	}
	void CreateUAV(ID3D11Device* const pDevice, D3D11_UNORDERED_ACCESS_VIEW_DESC desc)
	{
		pUAV.Reset();
		descUAV = desc;
		HRESULT hr = pDevice->CreateUnorderedAccessView(GetTexRes(), &descUAV.value(), pUAV.GetAddressOf());
		if (FAILED(hr)) throw std::runtime_error("Texture UAV creation failed");
	}

	inline ID3D11RenderTargetView* GetRTV() { return pRTV.Get(); }
	inline ID3D11ShaderResourceView* GetSRV() { return pSRV.Get(); }
	inline ID3D11UnorderedAccessView* GetUAV() { return pUAV.Get(); }
	inline ID3D11RenderTargetView** GetRTVAddress() { return pRTV.GetAddressOf(); }
	inline ID3D11ShaderResourceView** GetSRVAddress() { return pSRV.GetAddressOf(); }
	inline ID3D11UnorderedAccessView** GetUAVAddress() { return pUAV.GetAddressOf(); }

protected:
	inline virtual ID3D11Resource* GetTexRes() = 0;

protected:
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pUAV;

	std::optional<D3D11_RENDER_TARGET_VIEW_DESC> descRTV;
	std::optional<D3D11_SHADER_RESOURCE_VIEW_DESC> descSRV;
	std::optional<D3D11_UNORDERED_ACCESS_VIEW_DESC> descUAV;
};

class Texture2D : public TextureBase
{
public:
	Texture2D() = default;
	~Texture2D() = default;
	ROF_DELETE(Texture2D);

public:
	void CreateTexture(ID3D11Device* const pDevice, D3D11_TEXTURE2D_DESC desc, D3D11_SUBRESOURCE_DATA* data = nullptr)
	{
		pTex.Reset();
		descTex = desc;
		HRESULT hr = pDevice->CreateTexture2D(&descTex.value(), data, pTex.GetAddressOf());
		if (FAILED(hr)) throw std::runtime_error("Texture creation failed");
	}
	inline D3D11_SUBRESOURCE_DATA SubresTemplate(UINT bytesPerPixel, UINT width)
	{
		D3D11_SUBRESOURCE_DATA data = {};
		data.SysMemPitch = bytesPerPixel * width; // bytes between rows
		data.SysMemSlicePitch = 0u; // bytes between 2d slices
		return data;
	}
	inline ID3D11Texture2D* GetTex() { return pTex.Get(); }

private:
	ID3D11Resource* GetTexRes() { return pTex.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pTex;
	std::optional<D3D11_TEXTURE2D_DESC> descTex;
};

class Texture3D : public TextureBase
{
public:
	Texture3D() = default;
	~Texture3D() = default;
	ROF_DELETE(Texture3D);

public:
	void CreateTexture(ID3D11Device* const pDevice, D3D11_TEXTURE3D_DESC desc, D3D11_SUBRESOURCE_DATA* data = nullptr)
	{
		pTex.Reset();
		descTex = desc;
		HRESULT hr = pDevice->CreateTexture3D(&descTex.value(), data, pTex.GetAddressOf());
		if (FAILED(hr)) throw std::runtime_error("Texture creation failed");
	}
	inline D3D11_SUBRESOURCE_DATA SubresTemplate(UINT bytesPerPixel, UINT width, UINT height)
	{
		D3D11_SUBRESOURCE_DATA data = {};
		data.SysMemPitch = bytesPerPixel * width; // bytes between rows
		data.SysMemSlicePitch = bytesPerPixel * width * height; // bytes between 2d slices
		return data;
	}
	inline ID3D11Texture3D* GetTex() { return pTex.Get(); }

private:
	ID3D11Resource* GetTexRes() { return pTex.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3D11Texture3D> pTex;
	std::optional<D3D11_TEXTURE3D_DESC> descTex;
};