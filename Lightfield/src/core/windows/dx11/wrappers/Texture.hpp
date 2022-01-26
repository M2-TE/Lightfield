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
	void CreateTextureFromJPG(ID3D11Device* const pDevice, std::wstring filepath)
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

		WICPixelFormatGUID pixelFormatGUID;
		hr = pFrame->GetPixelFormat(&pixelFormatGUID);
		if (FAILED(hr)) throw std::runtime_error("WICPixelFormatGUID retrieval failed");

		auto& conv = TexFormatConverter::GetInstance();
		DXGI_FORMAT format = conv.GetDXGIFormat(pixelFormatGUID);

		Microsoft::WRL::ComPtr<IWICComponentInfo> pComponentInfo;
		hr = pImgFactory->CreateComponentInfo(pixelFormatGUID, pComponentInfo.GetAddressOf());
		if (FAILED(hr)) throw std::runtime_error("IWICComponentInfo retrieval failed");

		Microsoft::WRL::ComPtr<IWICPixelFormatInfo> pPixelFormatInfo;
		hr = pComponentInfo->QueryInterface(__uuidof(IWICPixelFormatInfo), reinterpret_cast<void**>(pPixelFormatInfo.GetAddressOf()));
		if (FAILED(hr)) throw std::runtime_error("IWICPixelFormatInfo query failed");

		UINT stride;
		hr = pPixelFormatInfo->GetBitsPerPixel(&stride);
		if (FAILED(hr)) throw std::runtime_error("Pixel stride retrieval failed");
		stride /= 8u; // convert from bits to bytes (compiler should use bitshifting here)


		UINT width, height;
		pFrame->GetSize(&width, &height);
		UINT rowStride = stride * width;
		UINT totalStride = rowStride * height;
		std::vector<BYTE> buffer(totalStride);
		hr = pFrame->CopyPixels(NULL, rowStride, totalStride, buffer.data());
		if (FAILED(hr)) throw std::runtime_error("Failed copying image data to memory");

		// round up stride to the nearest possible val for dx11 (24bpp -> 32bpp) bpp = bits per pixel
		if (stride == 3u) { // TODO: this handles 24bpp formats, what about 96bpp and others?

			// adjust strides according to new format
			stride = 4u;
			rowStride = stride * width;
			totalStride = rowStride * height;

			// copy original buffer into seperate vector for iteration
			// then resize new vector according to new strides
			std::vector<BYTE> cpy(buffer);
			buffer.resize(totalStride);
			auto* curCpy = cpy.begin()._Ptr;
			auto* cur = buffer.begin()._Ptr;
			auto* end = buffer.end()._Ptr;

			// insert padding to match dx11 format
			while (cur < end) {
				*cur++ = *curCpy++; // r or b
				*cur++ = *curCpy++; // g
				*cur++ = *curCpy++; // b or r
				*cur++ = UCHAR_MAX;	// a
			}
		}

		// Create texture
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = buffer.data();
		initData.SysMemPitch = rowStride;
		initData.SysMemSlicePitch = totalStride;
		CreateTexture(pDevice, desc, &initData);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = format;
		srvDesc.Texture2D.MipLevels = 1u;
		srvDesc.Texture2D.MostDetailedMip = 0u;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		CreateSRV(pDevice, srvDesc);
	}
	void CreateTexture(ID3D11Device* const pDevice, D3D11_TEXTURE2D_DESC desc, D3D11_SUBRESOURCE_DATA* data = nullptr)
	{
		pTex.Reset();
		descTex = desc;
		HRESULT hr = pDevice->CreateTexture2D(&descTex.value(), data, pTex.GetAddressOf());
		if (FAILED(hr)) throw std::runtime_error("Texture creation failed");
	}
	void WrapTexture(ID3D11Texture2D* pTexture)
	{
		pTex.Attach(pTexture);
	}

	void SaveTextureToFile(ID3D11DeviceContext* const pDeviceContext, std::wstring fileName, const GUID& guidContainerFormat = GUID_ContainerFormatJpeg)
	{
		HRESULT hr = DirectX::SaveWICTextureToFile(pDeviceContext, pTex.Get(), guidContainerFormat, fileName.c_str());
		if (FAILED(hr)) throw std::runtime_error("Screenshot failed");
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