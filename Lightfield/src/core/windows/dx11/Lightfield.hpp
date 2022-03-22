#pragma once

class Lightfield
{
public:
	Lightfield() = default;
	~Lightfield() = default;
	ROF_DELETE(Lightfield);

	void Init(ID3D11Device* const pDevice, UINT width, UINT height)
	{
		InitTextures(pDevice, width, height);
		InitDepthStencils(pDevice, width, height);
		InitOffsets(pDevice);
	}
	void Clear(ID3D11DeviceContext* const pDeviceContext)
	{
		static constexpr float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		for (UINT i = 0u; i < nCams; i++) {
			pDeviceContext->ClearRenderTargetView(rtvArr[i].Get(), clearColor);
			depthStencilArr[i].ClearDepthStencil(pDeviceContext);
		}
		
	}
	void Simulate(ID3D11DeviceContext* const pDeviceContext, std::vector<std::unique_ptr<RenderObject>>& renderObjects)
	{
		for (UINT i = 0u; i < nCams; i++) {
			pDeviceContext->OMSetRenderTargets(1u, rtvArr[i].GetAddressOf(), depthStencilArr[i].GetView());

			// Bind offset
			pDeviceContext->VSSetConstantBuffers(3u, 1u, offsetBufferArr[i].GetBufferAddress());

			// Bind and draw all the individual objects
			for (auto cur = renderObjects.begin(); cur != renderObjects.end(); cur++) {
				pDeviceContext->VSSetConstantBuffers(0u, 1u, (*cur)->GetTransform().GetBuffer(pDeviceContext).GetBufferAddress());
				(*cur)->Draw(pDeviceContext);
			}
		}
	}
	// TODO: screenshot functionality outside of Texture2D wrapper
	void Screenshot(ID3D11DeviceContext* const pDeviceContext)
	{
		throw new std::exception("TODO: screenshots");
		// save gpu textures to disk in .jpg format
		//simulatedColors[0].SaveTextureToFile(pDeviceContext, L"screenshots/simulatedColor0.jpg");
		//simulatedDepths[0].SaveTextureToFile(pDeviceContext, L"screenshots/simulatedDepth0.jpg");

		//simulatedColors[1].SaveTextureToFile(pDeviceContext, L"screenshots/simulatedColor1.jpg");
		//simulatedDepths[1].SaveTextureToFile(pDeviceContext, L"screenshots/simulatedDepth1.jpg");
	}

	void CyclePreviewCamera()
	{
		iPreviewCam = (iPreviewCam + 1) % nCams;
	}
	void BindColorTextures(ID3D11DeviceContext* const pDeviceContext)
	{
		pDeviceContext->PSSetShaderResources(0u, 1u, pSrvArr.GetAddressOf());
	}
	void UnbindColorTextures(ID3D11DeviceContext* const pDeviceContext)
	{
		ID3D11ShaderResourceView* pSRVs[] = { nullptr };
		pDeviceContext->PSSetShaderResources(0u, 1u, pSRVs);
	}

	// TODO: create single SRV for each tex for preview! (or cbuffer?)
	void BindPreviewTextures(ID3D11DeviceContext* const pDeviceContext)
	{
		pDeviceContext->PSSetShaderResources(0u, 1u, pSrvArr.GetAddressOf());
	}
	void UnbindPreviewTextures(ID3D11DeviceContext* const pDeviceContext)
	{
		ID3D11ShaderResourceView* const pSRVs[] = { nullptr };
		pDeviceContext->PSSetShaderResources(0u, 1u, pSRVs);
	}


private:
	// All part of the Init() func
	void InitTextures(ID3D11Device* const pDevice, UINT width, UINT height)
	{
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = 1u;
		texDesc.ArraySize = nCams;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0u;
		texDesc.SampleDesc.Count = 1u;
		texDesc.SampleDesc.Quality = 0u;
		pDevice->CreateTexture2D(&texDesc, nullptr, pTexArr.GetAddressOf());

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2DArray.FirstArraySlice = 0u;
		srvDesc.Texture2DArray.MostDetailedMip = 0u;
		srvDesc.Texture2DArray.MipLevels = 1u;
		srvDesc.Texture2DArray.ArraySize = nCams;
		pDevice->CreateShaderResourceView(pTexArr.Get(), &srvDesc, pSrvArr.GetAddressOf());

		// creating multiple RTV (one for each array slice)
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = texDesc.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.MipSlice = 0u;
		rtvDesc.Texture2DArray.ArraySize = 1u;
		for (UINT i = 0u; i < nCams; i++) {
			rtvDesc.Texture2DArray.FirstArraySlice = D3D11CalcSubresource(0u, i, 1u);
			pDevice->CreateRenderTargetView(pTexArr.Get(), &rtvDesc, rtvArr[i].GetAddressOf());
		}
	}
	void InitDepthStencils(ID3D11Device* const pDevice, UINT width, UINT height)
	{
		for (UINT i = 0u; i < nCams; i++) {
			depthStencilArr[i].Init(pDevice, width, height);
		}
	}
	void InitOffsets(ID3D11Device* const pDevice)
	{
		UINT bufferIndex = 0u;
		for (int x = -camLoopLim; x <= camLoopLim; x++) {
			for (int y = -camLoopLim; y <= camLoopLim; y++) {
				offsetBufferArr[bufferIndex].GetData() = DirectX::XMFLOAT3A(offset * static_cast<float>(x), offset * static_cast<float>(y), 0.0f);
				offsetBufferArr[bufferIndex++].Init(pDevice);
			}
		}
	}

private:
	static constexpr float offset = 0.1f; // distance to center camera
	static constexpr int camLoopLim = 1;
	static constexpr UINT nCams = 9u;
	UINT iPreviewCam = 0u;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexArr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pSrvArr;
	std::array<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>, nCams> rtvArr; // simultaneous render target limit of 4, so each needs to be rendered to separately
	std::array<ConstantBuffer<DirectX::XMFLOAT3A>, nCams> offsetBufferArr;
	std::array<DepthStencil, nCams> depthStencilArr; // only really need one in reality?
};