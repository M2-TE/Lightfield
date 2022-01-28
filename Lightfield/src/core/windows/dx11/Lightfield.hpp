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
	void Simulate(ID3D11DeviceContext* const pDeviceContext, std::vector<std::unique_ptr<RenderObject>>& renderObjects)
	{
		for (UINT i = 0u; i < nCams; i++) {
			// could create a separate array to hold these, but that kind of optimization isnt necessary yet
			// -> would result in less readable code
			ID3D11RenderTargetView* pRTVs[] = {
				simulatedColors[i].GetRTV(),
				simulatedDepths[i].GetRTV()
			};
			pDeviceContext->OMSetRenderTargets(2u, pRTVs, depthStencils[i].GetView());

			// Bind and draw all the individual objects
			for (auto cur = renderObjects.begin(); cur != renderObjects.end(); cur++) {
				pDeviceContext->VSSetConstantBuffers(0u, 1u, (*cur)->GetTransform().GetBuffer(pDeviceContext).GetBufferAddress());
				(*cur)->Draw(pDeviceContext);
			}
		}
	}

private:
	void InitTextures(ID3D11Device* const pDevice, UINT width, UINT height)
	{
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = 1u;
		texDesc.ArraySize = 1u;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
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

		// color maps
		for (UINT i = 0u; i < nCams; i++) {
			simulatedColors[i].CreateTexture(pDevice, texDesc);
			simulatedColors[i].CreateRTV(pDevice, rtvDesc);
			simulatedColors[i].CreateSRV(pDevice, srvDesc);
		}

		// depth maps
		texDesc.Format = DXGI_FORMAT_R16_FLOAT;
		rtvDesc.Format = texDesc.Format;
		srvDesc.Format = texDesc.Format;
		for (UINT i = 0u; i < nCams; i++) {
			simulatedDepths[i].CreateTexture(pDevice, texDesc);
			simulatedDepths[i].CreateRTV(pDevice, rtvDesc);
			simulatedDepths[i].CreateSRV(pDevice, srvDesc);
		}
	}
	void InitDepthStencils(ID3D11Device* const pDevice, UINT width, UINT height)
	{
		for (UINT i = 0u; i < nCams; i++) {
			depthStencils[i].Init(pDevice, width, height);
		}
	}
	void InitOffsets(ID3D11Device* const pDevice)
	{
		for (UINT i = 0u; i < nCams; i++) {
			offsetBuffers[i].Init(pDevice); // buffers can be edited at runtime (increase distance between eyes?)
		}
	}

private:
	static constexpr UINT nCams = 2u;
	std::array<Texture2D, nCams> simulatedColors;
	std::array<Texture2D, nCams> simulatedDepths;
	std::array<DepthStencil, nCams> depthStencils;
	std::array<ConstantBuffer<DirectX::XMFLOAT3A>, nCams> offsetBuffers;
};