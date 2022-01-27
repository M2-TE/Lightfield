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
		
		CreateViewport();
		CreateRasterizer();
		CreateDepthStencilStates();
		CreateDepthStencil();
		CreateSamplerState();
		CreateTextureBuffers();
		
		LoadShaders();

		// create camera and move it back a bit to see all the objects
		pCamera = std::make_unique<Camera>(pDevice.Get());
	}
	ROF_DELETE(Renderer);

	void SimulateScene()
	{
		Clear();

		forwardVS.Bind(pDeviceContext.Get());
		forwardPS.Bind(pDeviceContext.Get());

		// Bind constant buffers (camera)
		ID3D11Buffer* camVsBuffers[] = {
			pCamera->GetTransform().GetInverseBuffer(pDeviceContext.Get()).GetBuffer(), // Camera's ViewMatrix
			pCamera->GetProjectionBuffer().GetBuffer() // Camera's ProjectionMatrix
		};
		pDeviceContext->VSSetConstantBuffers(1u, 2u, camVsBuffers);
		pDeviceContext->PSSetConstantBuffers(1u, 1u, pCamera->GetPosBuffer().GetBufferAddress());

		// Set render target and depth stencil view for rendering
		pDeviceContext->OMSetDepthStencilState(pDefaultDSS.Get(), 1u);
		pDeviceContext->OMSetRenderTargets(1u, simulatedColor.GetRTVAddress(), pDepthStencil->GetView());

		// Bind and draw all the individual objects
		for (auto cur = renderObjects.begin(); cur != renderObjects.end(); cur++) {
			pDeviceContext->VSSetConstantBuffers(0u, 1u, (*cur)->GetTransform().GetBuffer(pDeviceContext.Get()).GetBufferAddress());
			(*cur)->Draw(pDeviceContext.Get());
		}
	}
	void DeduceDepth()
	{
		oversizedTriangleVS.Bind(pDeviceContext.Get());
		depthDeductionPS.Bind(pDeviceContext.Get());

		// attach the non-depth DSS and set depthOutput as render target
		pDeviceContext->OMSetDepthStencilState(pNoDepthDSS.Get(), 1u);
		pDeviceContext->OMSetRenderTargets(1u, outputDepth.GetRTVAddress(), nullptr);

		// read intermediate color buffer as input
		pDeviceContext->PSSetShaderResources(0u, 1u, simulatedColor.GetSRVAddress());

		DrawOversizedTriangle();
	}
	void Present()
	{
		oversizedTriangleVS.Bind(pDeviceContext.Get());
		presentationPS.Bind(pDeviceContext.Get());

		// attach the non-depth DSS and set backbuffer as render target
		pDeviceContext->OMSetDepthStencilState(pNoDepthDSS.Get(), 1u);
		pDeviceContext->OMSetRenderTargets(1u, backBuffer.GetRTVAddress(), nullptr);

		// add depth output as a shader resource
		pDeviceContext->PSSetShaderResources(1u, 1u, outputDepth.GetSRVAddress());

		DrawOversizedTriangle();

		// Present backbuffer to the screen
		if (bVSync)pSwapChain->Present(1u, 0u);
		else pSwapChain->Present(0u, DXGI_PRESENT_ALLOW_TEARING);

		// unbind resources
		ID3D11ShaderResourceView* pSRVsNull[2] = { nullptr, nullptr };
		pDeviceContext->PSSetShaderResources(0u, 2u, pSRVsNull);
	}

	void Screenshot()
	{
		// bind shaders
		oversizedTriangleVS.Bind(pDeviceContext.Get());
		screenshotPS.Bind(pDeviceContext.Get());

		// set screenshot render targets and depth stencil view for rendering
		pDeviceContext->OMSetDepthStencilState(pNoDepthDSS.Get(), 1u);
		ID3D11RenderTargetView* rtvs[2] = { simulatedDepth.GetRTV(), simulatedStencil.GetRTV() };
		pDeviceContext->OMSetRenderTargets(2u, rtvs, nullptr);

		// set depth stencil as shader resource to be written to a texture
		ID3D11ShaderResourceView* pSRVs[2] = { pDepthStencil->GetDepthSRV(), pDepthStencil->GetStencilSRV() };
		pDeviceContext->PSSetShaderResources(0u, 2u, pSRVs);
		pDeviceContext->PSSetConstantBuffers(0u, 1u, pCamera->GetTransform().GetInverseBuffer(pDeviceContext.Get()).GetBufferAddress());

		DrawOversizedTriangle();

		// save gpu textures to disk in .jpg format
		simulatedColor.SaveTextureToFile(pDeviceContext.Get(), L"screenshots/simulatedColor.jpg");
		simulatedDepth.SaveTextureToFile(pDeviceContext.Get(), L"screenshots/simulatedDepth.jpg");
		simulatedStencil.SaveTextureToFile(pDeviceContext.Get(), L"screenshots/simulatedStencil.jpg");
		outputDepth.SaveTextureToFile(pDeviceContext.Get(), L"screenshots/outputDepth.jpg");
		
		// unbind stencil buffer to be able to use it again
		ID3D11ShaderResourceView* pSRVsNull[2] = { nullptr, nullptr };
		pDeviceContext->PSSetShaderResources(0u, 2u, pSRVsNull);
	}

	Camera& GetCamera() { return *pCamera; }
	std::vector<std::unique_ptr<RenderObject>>& GetRenderObjects() { return renderObjects; }
	ID3D11Device* GetDevice() { return pDevice.Get(); }
	ID3D11DeviceContext* GetDeviceContext() { return pDeviceContext.Get(); }

private:
	// Pipeline macros
	void Clear()
	{
		// clear textures from previous render/simulation
		static constexpr float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		pDeviceContext->ClearRenderTargetView(backBuffer.GetRTV(), clearColor);
		pDeviceContext->ClearRenderTargetView(simulatedColor.GetRTV(), clearColor);
		pDeviceContext->ClearRenderTargetView(outputDepth.GetRTV(), clearColor); // TODO: only really need to write one channel
		pDepthStencil->ClearDepthStencil(pDeviceContext.Get());
	}
	void DrawOversizedTriangle()
	{
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
		ID3D11Texture2D* pBackBuffer = nullptr; // no ownership of texture
		HRESULT hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
		if (FAILED(hr)) throw std::runtime_error("Could not aquire swapchain backbuffer");

		//Create a render target view on the back buffer
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		//rtvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; // no srgb required for lightfield
		rtvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0u;

		backBuffer.WrapTexture(pBackBuffer);
		backBuffer.CreateRTV(pDevice.Get(), rtvDesc);
	}
	void CreateViewport()
	{
		// Viewport creation and permanent assignment
		CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
		pDeviceContext->RSSetViewports(1u, &viewport);
	}
	void CreateRasterizer()
	{
		D3D11_RASTERIZER_DESC rastDesc = {};
		rastDesc.FillMode = D3D11_FILL_SOLID;
		rastDesc.CullMode = D3D11_CULL_BACK;
		rastDesc.CullMode = D3D11_CULL_NONE;
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
	void CreateDepthStencil()
	{
		pDepthStencil = std::make_unique<DepthStencil>(pDevice.Get(), width, height);
	}
	void CreateSamplerState()
	{
		// sets default sampler to PS sampler slot 0
		D3D11_SAMPLER_DESC desc = {};
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.MinLOD = -FLT_MAX;
		desc.MaxLOD = FLT_MAX;
		desc.MipLODBias = 0.0f;
		desc.MaxAnisotropy = 1u;
		desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		desc.BorderColor[0] = 1.0f;
		desc.BorderColor[1] = 1.0f;
		desc.BorderColor[2] = 1.0f;
		desc.BorderColor[3] = 1.0f;

		HRESULT hr = pDevice->CreateSamplerState(&desc, pSamplerState.GetAddressOf());
		pDeviceContext->PSSetSamplers(0u, 1u, pSamplerState.GetAddressOf());
	}
	void CreateTextureBuffers()
	{
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = 1u;
		texDesc.ArraySize = 1u;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
		texDesc.CPUAccessFlags = 0u;
		texDesc.SampleDesc.Count = 1u;
		texDesc.SampleDesc.Quality = 0u;

		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = texDesc.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1u;
		srvDesc.Texture2D.MostDetailedMip = 0u;

		simulatedDepth.CreateTexture(pDevice.Get(), texDesc);
		simulatedDepth.CreateRTV(pDevice.Get(), rtvDesc);
		simulatedStencil.CreateTexture(pDevice.Get(), texDesc);
		simulatedStencil.CreateRTV(pDevice.Get(), rtvDesc);

		texDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		simulatedColor.CreateTexture(pDevice.Get(), texDesc);
		simulatedColor.CreateRTV(pDevice.Get(), rtvDesc);
		simulatedColor.CreateSRV(pDevice.Get(), srvDesc);

		texDesc.Format = DXGI_FORMAT_R16_UNORM;
		rtvDesc.Format = texDesc.Format;
		srvDesc.Format = texDesc.Format;
		outputDepth.CreateTexture(pDevice.Get(), texDesc);
		outputDepth.CreateRTV(pDevice.Get(), rtvDesc);
		outputDepth.CreateSRV(pDevice.Get(), srvDesc);
	}
	void LoadShaders()
	{
		forwardVS.LoadShader(pDevice.Get(), L"data/shaders/ForwardVS.cso");
		oversizedTriangleVS.LoadShader(pDevice.Get(), L"data/shaders/OversizedTriangleVS.cso");

		forwardPS.LoadShader(pDevice.Get(), L"data/shaders/ForwardPS.cso");
		screenshotPS.LoadShader(pDevice.Get(), L"data/shaders/ScreenshotPS.cso");
		depthDeductionPS.LoadShader(pDevice.Get(), L"data/shaders/DepthDeductionPS.cso");
		presentationPS.LoadShader(pDevice.Get(), L"data/shaders/PresentationPS.cso");
	}

public:
		enum class PresentationMode { Default };
		PresentationMode presentationMode;
private:
	bool bVSync = true;
	UINT width, height;

	// Device with context and swapchain
	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pDeviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain;

	// Pipeline objects
	std::unique_ptr<DepthStencil> pDepthStencil;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> pSamplerState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pNoDepthDSS, pDefaultDSS;

	// Texture Buffers
	Texture2D backBuffer; // swapchain backbuffer
	Texture2D simulatedColor;
	Texture2D simulatedDepth;
	Texture2D simulatedStencil;
	Texture2D outputDepth;

	// Shaders
	Shader<ID3D11VertexShader> forwardVS, oversizedTriangleVS;
	Shader<ID3D11PixelShader> forwardPS, screenshotPS, depthDeductionPS, presentationPS;

	// Render objects
	std::unique_ptr<Camera> pCamera;
	std::vector<std::unique_ptr<RenderObject>> renderObjects;
};