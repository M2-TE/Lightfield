#pragma once

#include "TexFormatConverter.hpp"

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
	void CreateTextureJPG(ID3D11Device* const pDevice, std::wstring filepath)
	{
		Microsoft::WRL::ComPtr<IWICImagingFactory> pImgFactory;
		HRESULT hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(pImgFactory.GetAddressOf()));
		if (FAILED(hr)) throw std::runtime_error("IWICImagingFactory creation failed");

		Microsoft::WRL::ComPtr<IWICBitmapDecoder> pBitmapDecoder;
		hr = pImgFactory->CreateDecoderFromFilename(
			filepath.c_str(),
			NULL,
			GENERIC_READ,
			WICDecodeMetadataCacheOnLoad,
			pBitmapDecoder.GetAddressOf());
		if (FAILED(hr)) throw std::runtime_error("IWICBitmapDecoder creation failed");

		Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> pFrame;
		hr = pBitmapDecoder->GetFrame(0u, pFrame.GetAddressOf());
		if (FAILED(hr)) throw std::runtime_error("IWICBitmapFrameDecode retrieval failed");

		WICPixelFormatGUID pixelFormatGUID = {};
		hr = pFrame->GetPixelFormat(&pixelFormatGUID);
		if (FAILED(hr)) throw std::runtime_error("WICPixelFormatGUID retrieval failed");

		// gotta do if/else, because guids cannot be used in switch statements?
		// surely theres a table of enums for this instead, will look into it later
		DXGI_FORMAT format;
		if (pixelFormatGUID == GUID_WICPixelFormat128bppRGBAFloat) format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		else if (pixelFormatGUID == GUID_WICPixelFormat96bppRGBFloat) format = DXGI_FORMAT_R32G32B32_FLOAT;
		else if (pixelFormatGUID == GUID_WICPixelFormat64bppRGBAHalf) format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		else if (pixelFormatGUID == GUID_WICPixelFormat64bppRGBA) format = DXGI_FORMAT_R16G16B16A16_UNORM;
		else if (pixelFormatGUID == GUID_WICPixelFormat32bppRGBA) format = DXGI_FORMAT_R8G8B8A8_UNORM;
		else if (pixelFormatGUID == GUID_WICPixelFormat32bppBGRA) format = DXGI_FORMAT_B8G8R8A8_UNORM;
		else if (pixelFormatGUID == GUID_WICPixelFormat32bppBGR) format = DXGI_FORMAT_B8G8R8X8_UNORM;
		else if (pixelFormatGUID == GUID_WICPixelFormat32bppRGBA1010102XR) format = DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
		else if (pixelFormatGUID == GUID_WICPixelFormat32bppRGBA1010102) format = DXGI_FORMAT_R10G10B10A2_UNORM;
		else if (pixelFormatGUID == GUID_WICPixelFormat32bppRGBE) format = DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
		else if (pixelFormatGUID == GUID_WICPixelFormat16bppBGRA5551) format = DXGI_FORMAT_B5G5R5A1_UNORM;
		else if (pixelFormatGUID == GUID_WICPixelFormat16bppBGR565) format = DXGI_FORMAT_B5G6R5_UNORM;

		else if (pixelFormatGUID == GUID_WICPixelFormat32bppGrayFloat) format = DXGI_FORMAT_R32_FLOAT;
		else if (pixelFormatGUID == GUID_WICPixelFormat16bppGrayHalf) format = DXGI_FORMAT_R16_FLOAT;
		else if (pixelFormatGUID == GUID_WICPixelFormat16bppGray) format = DXGI_FORMAT_R16_UNORM;
		else if (pixelFormatGUID == GUID_WICPixelFormat8bppGray) format = DXGI_FORMAT_R8_UNORM;
		else if (pixelFormatGUID == GUID_WICPixelFormat8bppAlpha) format = DXGI_FORMAT_A8_UNORM;
		else throw std::runtime_error("WICPixelFormatGUID is invalid");

		Microsoft::WRL::ComPtr<IWICComponentInfo> pComponentInfo;
		hr = pImgFactory->CreateComponentInfo(pixelFormatGUID, pComponentInfo.GetAddressOf());
		if (FAILED(hr)) throw std::runtime_error("IWICComponentInfo retrieval failed");

		Microsoft::WRL::ComPtr<IWICPixelFormatInfo> pPixelFormatInfo;
		hr = pComponentInfo->QueryInterface(__uuidof(IWICPixelFormatInfo), reinterpret_cast<void**>(pPixelFormatInfo.GetAddressOf()));
		if (FAILED(hr)) throw std::runtime_error("IWICPixelFormatInfo query failed");

		UINT stride;
		hr = pPixelFormatInfo->GetBitsPerPixel(&stride);
		if (FAILED(hr)) throw std::runtime_error("Pixel byte stride retrieval failed");
		stride /= 8u;

		UINT width, height;
		pFrame->GetSize(&width, &height);
		UINT rowStride = stride * width;
		UINT totalStride = rowStride * height;
		std::vector<BYTE> buffer(totalStride);
		hr = pFrame->CopyPixels(NULL, rowStride, totalStride, buffer.data());

		if (FAILED(hr)) throw std::runtime_error("Failed copying image data to memory");

		//// Create texture
		//D3D11_TEXTURE2D_DESC desc;
		//desc.Width = width;
		//desc.Height = height;
		//desc.MipLevels = 1;
		//desc.ArraySize = 1;
		//desc.Format = format;
		//desc.SampleDesc.Count = 1;
		//desc.SampleDesc.Quality = 0;
		//desc.Usage = D3D11_USAGE_DEFAULT;
		//desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		//desc.CPUAccessFlags = 0;
		//desc.MiscFlags = 0;

		//D3D11_SUBRESOURCE_DATA initData;
		//initData.pSysMem = buffer.data();
		//initData.SysMemPitch = rowStride;
		//initData.SysMemSlicePitch = byteSize;

		//CreateTexture(pDevice, desc, &initData);


		// TODO: also create SRV here
	}
	void CreateTexture(ID3D11Device* const pDevice, D3D11_TEXTURE2D_DESC desc, D3D11_SUBRESOURCE_DATA* data = nullptr)
	{
		pTex.Reset();
		descTex = desc;
		HRESULT hr = pDevice->CreateTexture2D(&descTex.value(), data, pTex.GetAddressOf());
		if (FAILED(hr)) throw std::runtime_error("Texture creation failed");
	}
	inline constexpr D3D11_SUBRESOURCE_DATA SubresTemplate(UINT bytesPerPixel, UINT width)
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