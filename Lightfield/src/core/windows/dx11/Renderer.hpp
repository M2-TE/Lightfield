#pragma once

#include "DepthStencil.hpp"

class Renderer
{
public:
	Renderer(HWND hWnd, UINT width, UINT height)
	{

		this->width = static_cast<UINT>(width);
		this->height = static_cast<UINT>(height);

		HRESULT hr;

		// Device and SwapChain
		{
			DXGI_SWAP_CHAIN_DESC sd = {};
			sd.BufferDesc.Width = 0;
			sd.BufferDesc.Height = 0;
			sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			sd.BufferDesc.RefreshRate.Numerator = 0;
			sd.BufferDesc.RefreshRate.Denominator = 0;
			sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

			sd.SampleDesc.Count = 1u;
			sd.SampleDesc.Quality = 0u;

			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.BufferCount = 2u;

			sd.OutputWindow = hWnd;
			sd.Windowed = TRUE;
			//sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			//sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
			sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

			UINT creationFlags = 0u;
#ifdef _DEBUG
			creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

			std::vector<D3D_FEATURE_LEVEL> featureLevels =
			{
				D3D_FEATURE_LEVEL_12_1,
				D3D_FEATURE_LEVEL_12_0,
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_10_1,
				D3D_FEATURE_LEVEL_10_0,
				D3D_FEATURE_LEVEL_9_3,
				D3D_FEATURE_LEVEL_9_2,
				D3D_FEATURE_LEVEL_9_1
			};

			// create device and front/back buffers, and swap chain and rendering context
			hr = D3D11CreateDeviceAndSwapChain(
				nullptr,
				D3D_DRIVER_TYPE_HARDWARE,
				nullptr,
				creationFlags,
				featureLevels.data(),
				static_cast<UINT>(featureLevels.size()),
				D3D11_SDK_VERSION,
				&sd,
				&pSwapChain,
				&pDevice,
				nullptr,
				&pDeviceContext);
			if (FAILED(hr)) throw std::runtime_error("Swapchain and rendering context creation failed");

			// only present as frequently as the screen can actually display
			//pDxgiDevice->SetMaximumFrameLatency(1u);

			// Set primitive topology to triangle list
			pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}

		// Get access to swapchain backbuffer and create render target view for it
		{
			hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)pBackBuffer.GetAddressOf());
			if (FAILED(hr)) throw std::runtime_error("Could not aquire swapchain backbuffer");

			//Create a render target view on the back buffer
			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			//rtvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; // no srgb required for lightfield i think
			rtvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D.MipSlice = 0u;
			HRESULT hr = pDevice->CreateRenderTargetView(pBackBuffer.Get(), &rtvDesc, pBackBufferRTV.GetAddressOf());
			if (FAILED(hr)) throw std::runtime_error("RTV creation failed");
		}

		// depth stencil
		pDepthStencil = std::make_unique<DepthStencil>(pDevice.Get(), width, height);
	}
	~Renderer() = default;
	ROF_DELETE(Renderer);

	void Render()
	{
		// TODO
	}

private:
	UINT width, height;
	std::unique_ptr<DepthStencil> pDepthStencil;

	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pDeviceContext;

	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pBackBufferRTV;
};