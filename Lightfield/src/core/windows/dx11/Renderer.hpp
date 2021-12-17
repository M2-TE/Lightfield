#pragma once

#include "wrappers/ConstantBuffer.hpp"
#include "wrappers/DepthStencil.hpp"
#include "wrappers/Shader.hpp"
#include "wrappers/Texture.hpp"
#include "objects/Camera.hpp"
#include "objects/RenderObject.hpp"

class Renderer
{
public:
	Renderer(HWND hWnd, UINT width, UINT height)
	{
		this->width = static_cast<UINT>(width);
		this->height = static_cast<UINT>(height);

		CreateDeviceSwapchain(hWnd);
		AccessBackbuffer();
		CreateRasterizer();
		CreateDepthStencilStates();

		// Viewport creation and permanent assignment
		CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
		pDeviceContext->RSSetViewports(1u, &viewport);

		pDepthStencil = std::make_unique<DepthStencil>(pDevice.Get(), width, height);

		oversizedTriangleVS.LoadShader(pDevice.Get(), L"data/shaders/OversizedTriangleVS.cso");
		defaultPS.LoadShader(pDevice.Get(), L"data/shaders/DefaultPS.cso");
		forwardVS.LoadShader(pDevice.Get(), L"data/shaders/ForwardVS.cso");
		forwardPS.LoadShader(pDevice.Get(), L"data/shaders/ForwardPS.cso");

		// create camera and move it back a bit to see all the objects
		pCamera = std::make_unique<Camera>(pDevice.Get());
	}
	ROF_DELETE(Renderer);

	void Render()
	{
		//pRenderObject->GetTransform().RotateEuler(0.0f, Time::Get().deltaTime, 0.0f);

		// clear textures from previous render
		static constexpr float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		pDeviceContext->ClearRenderTargetView(pBackBufferRTV.Get(), clearColor);
		pDepthStencil->ClearDepthStencil(pDeviceContext.Get());

		if (true) DrawDefault();
		else DrawOversizedTriangle();

		// Present backbuffer to the screen
		if (bVSync)pSwapChain->Present(1u, 0u);
		else pSwapChain->Present(0u, DXGI_PRESENT_ALLOW_TEARING);
	}

	Camera& GetCamera() { return *pCamera; }
	std::vector<std::unique_ptr<RenderObject>>& GetRenderObjects() { return renderObjects; }
	ID3D11Device* GetDevice() { return pDevice.Get(); }
	ID3D11DeviceContext* GetDeviceContext() { return pDeviceContext.Get(); }

private:
	// Pipeline usage
	void DrawDefault()
	{
		// Bind constant buffers (camera)
		ID3D11Buffer* camVsBuffers[] = {
			pCamera->GetTransform().GetInverseBuffer(pDeviceContext.Get()).GetBuffer(), // Camera's ViewMatrix
			pCamera->GetProjectionBuffer().GetBuffer() // Camera's ProjectionMatrix
		};
		pDeviceContext->VSSetConstantBuffers(1u, 2u, camVsBuffers);
		pDeviceContext->PSSetConstantBuffers(1u, 1u, pCamera->GetPosBuffer().GetBufferAddress());

		// Bind shaders
		forwardVS.Bind(pDeviceContext.Get());
		forwardPS.Bind(pDeviceContext.Get());

		// Set render target and depth stencil view for rendering
		pDeviceContext->OMSetDepthStencilState(pDefaultDSS.Get(), 1u);
		pDeviceContext->OMSetRenderTargets(1u, pBackBufferRTV.GetAddressOf(), pDepthStencil->GetView());

		// Bind and draw all the individual objects
		for (auto cur = renderObjects.begin(); cur != renderObjects.end(); cur++) {
			pDeviceContext->VSSetConstantBuffers(0u, 1u, (*cur)->GetTransform().GetBuffer(pDeviceContext.Get()).GetBufferAddress());
			(*cur)->Draw(pDeviceContext.Get());
		}
	}
	void DrawOversizedTriangle()
	{
		// bind shaders
		oversizedTriangleVS.Bind(pDeviceContext.Get());
		defaultPS.Bind(pDeviceContext.Get());

		// set render target and depth stencil view for rendering
		pDeviceContext->OMSetDepthStencilState(pNoDepthDSS.Get(), 1u);
		pDeviceContext->OMSetRenderTargets(1u, pBackBufferRTV.GetAddressOf(), nullptr);

		// set empty vertex buffer
		static constexpr UINT zero = 0u;
		pDeviceContext->IASetVertexBuffers(zero, zero, nullptr, &zero, &zero);

		// draw three vertices (oversized screen space triangle created in vertex shader)
		pDeviceContext->Draw(3u, zero);
	}

	// Pipeline creation
	void CreateDeviceSwapchain(HWND hWnd)
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
		HRESULT hr = D3D11CreateDeviceAndSwapChain(
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
	void AccessBackbuffer()
	{
		// Get access to swapchain backbuffer and create render target view for it
		HRESULT hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)pBackBuffer.GetAddressOf());
		if (FAILED(hr)) throw std::runtime_error("Could not aquire swapchain backbuffer");

		//Create a render target view on the back buffer
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		//rtvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; // no srgb required for lightfield i think
		rtvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0u;
		hr = pDevice->CreateRenderTargetView(pBackBuffer.Get(), &rtvDesc, pBackBufferRTV.GetAddressOf());
		if (FAILED(hr)) throw std::runtime_error("RTV creation failed");
	}
	void CreateRasterizer()
	{
		D3D11_RASTERIZER_DESC rastDesc = {};
		rastDesc.FillMode = D3D11_FILL_SOLID;
		rastDesc.CullMode = D3D11_CULL_BACK;
		rastDesc.FrontCounterClockwise = FALSE;
		rastDesc.DepthBias = 0;
		rastDesc.SlopeScaledDepthBias = 0.0f;
		rastDesc.DepthBiasClamp = 0.0f;
		rastDesc.DepthClipEnable = FALSE;
		rastDesc.ScissorEnable = FALSE;
		rastDesc.MultisampleEnable = FALSE;
		rastDesc.AntialiasedLineEnable = FALSE;

		pDevice->CreateRasterizerState(&rastDesc, pRasterizer.GetAddressOf());
		pDeviceContext->RSSetState(pRasterizer.Get());
	}
	void CreateDepthStencilStates()
	{
		D3D11_DEPTH_STENCIL_DESC dssDesc = {};
		dssDesc.DepthEnable = FALSE;
		dssDesc.StencilEnable = FALSE;
		dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dssDesc.DepthFunc = D3D11_COMPARISON_LESS;
		pDevice->CreateDepthStencilState(&dssDesc, pNoDepthDSS.GetAddressOf());


		dssDesc.DepthEnable = TRUE;
		dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		pDevice->CreateDepthStencilState(&dssDesc, pDefaultDSS.GetAddressOf());
	}

private:
	bool bVSync = true;
	UINT width, height;

	// Device with context and swapchain
	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pDeviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain;

	// Pipeline objects
	std::unique_ptr<DepthStencil> pDepthStencil;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pNoDepthDSS, pDefaultDSS;
	// Swapchain backbuffer
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pBackBufferRTV;

	// Shaders
	Shader<ID3D11VertexShader> forwardVS, oversizedTriangleVS;
	Shader<ID3D11PixelShader> forwardPS, defaultPS;

	// Render objects
	std::unique_ptr<Camera> pCamera;
	std::vector<std::unique_ptr<RenderObject>> renderObjects;
};