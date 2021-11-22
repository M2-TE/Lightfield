#pragma once

template <class T = ID3D11VertexShader>
class Shader
{
public:
	Shader() = default;
	Shader(ID3D11Device* const pDevice, const LPCWSTR filePath);
	~Shader()
	{
		DeleteShader();
	}

	void SetCBs(std::vector<ID3D11Buffer*>&& constantBuffers)
	{
		cbs = std::move(constantBuffers);
		nullCBs.resize(cbs.size(), nullptr);
	}
	void SetSRVs(std::vector<ID3D11ShaderResourceView*>&& shaderResourceViews)
	{
		srvs = std::move(shaderResourceViews);
		nullSRVs.resize(srvs.size(), nullptr);
	}

	void LoadShader(ID3D11Device* const, const LPCWSTR);
	void Bind(ID3D11DeviceContext* const pDeviceContext) const;
	void Unbind(ID3D11DeviceContext* const pDeviceContext) const;

private:
	void DeleteShader()
	{
		if (pShader) pShader->Release();
	}
	void CreateInputLayout(ID3D11Device* const pDevice, LPCVOID bufferPointer, SIZE_T bufferSize);
	DXGI_FORMAT InterpretFormat(const BYTE mask, const D3D_REGISTER_COMPONENT_TYPE componentType) const;

private:
	T* pShader = nullptr;
	std::vector<ID3D11Buffer*> cbs;
	std::vector<ID3D11Buffer*> nullCBs;
	std::vector<ID3D11ShaderResourceView*> srvs;
	std::vector<ID3D11ShaderResourceView*> nullSRVs;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
};
