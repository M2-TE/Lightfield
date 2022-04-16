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
			pDeviceContext->ClearRenderTargetView(simDepthArr[i].GetRTV(), clearColor);
			depthStencilArr[i].ClearDepthStencil(pDeviceContext);
		}
	}
	void Simulate(ID3D11DeviceContext* const pDeviceContext, std::vector<std::unique_ptr<RenderObject>>& renderObjects)
	{
		for (UINT i = 0u; i < nCams; i++) {
			ID3D11RenderTargetView* rtvs[2] = { rtvArr[i].Get(), simDepthArr[i].GetRTV() };
			pDeviceContext->OMSetRenderTargets(2u, rtvs, depthStencilArr[i].GetView());

			// Bind offset
			pDeviceContext->VSSetConstantBuffers(3u, 1u, offsetBufferArr[i].GetBufferAddress());

			// Bind and draw all the individual objects
			for (auto cur = renderObjects.begin(); cur != renderObjects.end(); cur++) {;
				pDeviceContext->VSSetConstantBuffers(0u, 1u, (*cur)->GetTransform().GetBuffer(pDeviceContext).GetBufferAddress());
				(*cur)->Draw(pDeviceContext);
			}
		}
	}

	// save gpu textures to disk in .jpg format
	void Screenshot(ID3D11DeviceContext* const pDeviceContext)
	{
		for (UINT i = 0u; i < nCams; i++) {

			std::wstringstream wss;
			wss << L"screenshots/simulated_depth_" << i << L".jpg";
			simDepthArr[i].SaveTextureToFile(pDeviceContext, wss.str());
		}

		pDeviceContext->PSSetShaderResources(0u, 1u, pSrvArr.GetAddressOf());
		pDeviceContext->PSSetConstantBuffers(0u, 1u, previewCamBuffer.GetBufferAddress());
		pDeviceContext->OMSetRenderTargets(1u, screenshotBuffer.GetRTVAddress(), nullptr);
		UINT iOrig = previewCamBuffer.GetData(); // save original index
		for (UINT i = 0u; i < nCams; i++) {

			// update index (using the already existing preview cam buffer for that purpose)
			previewCamBuffer.Update(pDeviceContext, i);

			// render current texture into screenshot buffer
			static constexpr UINT zero = 0u;
			pDeviceContext->IASetVertexBuffers(zero, zero, nullptr, &zero, &zero);
			pDeviceContext->Draw(3u, zero);
			
			// then save screenshot buffer as .jpg
			std::wstringstream wss;
			wss << L"screenshots/simulated_color" << i << L".jpg";
			screenshotBuffer.SaveTextureToFile(pDeviceContext, wss.str());
		}

		// reset back to original state
		previewCamBuffer.Update(pDeviceContext, iOrig);
	}

	void CyclePreviewCamera(ID3D11DeviceContext* const pDeviceContext)
	{
		UINT iCur = previewCamBuffer.GetData();
		//previewCamBuffer.GetData() = (iCur + 1) % nCams;
		previewCamBuffer.Update(pDeviceContext, (iCur + 1) % nCams);
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

	void BindPreviewTextures(ID3D11DeviceContext* const pDeviceContext)
	{
		ID3D11ShaderResourceView* srvs[] = { pSrvArr.Get(), simDepthArr[previewCamBuffer.GetData()].GetSRV() };
		pDeviceContext->PSSetShaderResources(0u, 2u, srvs);
		pDeviceContext->PSSetConstantBuffers(1u, 1u, previewCamBuffer.GetBufferAddress());
	}
	void UnbindPreviewTextures(ID3D11DeviceContext* const pDeviceContext)
	{
		ID3D11ShaderResourceView* const pSRVs[] = { nullptr, nullptr };
		pDeviceContext->PSSetShaderResources(0u, 2u, pSRVs);
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
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
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

		// simulated depths
		texDesc.Format = DXGI_FORMAT_R16_UNORM;
		texDesc.ArraySize = 1u;
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1u;
		srvDesc.Texture2D.MostDetailedMip = 0u;
		rtvDesc.Format = texDesc.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0u;
		for (UINT i = 0u; i < nCams; i++) {
			simDepthArr[i].CreateTexture(pDevice, texDesc);
			simDepthArr[i].CreateRTV(pDevice, rtvDesc);
			simDepthArr[i].CreateSRV(pDevice, srvDesc);
		}

		// screenshot buffer
		texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		srvDesc.Format = texDesc.Format;
		rtvDesc.Format = texDesc.Format;
		screenshotBuffer.CreateTexture(pDevice, texDesc);
		screenshotBuffer.CreateRTV(pDevice, rtvDesc);
		screenshotBuffer.CreateSRV(pDevice, srvDesc);

	}
	void InitDepthStencils(ID3D11Device* const pDevice, UINT width, UINT height)
	{
		for (UINT i = 0u; i < nCams; i++) {
			depthStencilArr[i].Init(pDevice, width, height);
		}
	}
	void InitOffsets(ID3D11Device* const pDevice)
	{
		UINT bufferIndex = iInitialCam;
		for (int x = -camLoopLim; x <= camLoopLim; x++) {
			for (int y = -camLoopLim; y <= camLoopLim; y++) {
				offsetBufferArr[bufferIndex].GetData() = DirectX::XMFLOAT3A(offset * static_cast<float>(x), offset *- static_cast<float>(y), 0.0f); // invert y to match texture coord grid
				offsetBufferArr[bufferIndex++].Init(pDevice);
			}
		}

		previewCamBuffer.GetData() = 0u;
		previewCamBuffer.Init(pDevice);
	}

private:
	static constexpr float offset = .01f; // distance to center camera
	static constexpr int camLoopLim = 1;
	static constexpr UINT nCams = 9u;
	static constexpr UINT iInitialCam = 0u;
	ConstantBuffer<UINT> previewCamBuffer;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexArr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pSrvArr;
	std::array<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>, nCams> rtvArr;
	std::array<ConstantBuffer<DirectX::XMFLOAT3A>, nCams> offsetBufferArr;
	std::array<DepthStencil, nCams> depthStencilArr; // only really need one in reality?

	// for screenshots
	std::array<Texture2D, nCams> simDepthArr;
	Texture2D screenshotBuffer;

	std::array<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>, nCams> depthRtvArr;
	std::array<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>, nCams> depthSrvArr;
	std::array<Microsoft::WRL::ComPtr<ID3D11Texture2D>, nCams> depthTexArr;
};