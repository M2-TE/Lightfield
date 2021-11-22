#include "pch.hpp"
#include "Shader.hpp"

// ctors
template <>
Shader<ID3D11VertexShader>::Shader(ID3D11Device* const pDevice, const LPCWSTR filePath)
{
	LoadShader(pDevice, filePath);
}
template <>
Shader<ID3D11PixelShader>::Shader(ID3D11Device* const pDevice, const LPCWSTR filePath)
{
	LoadShader(pDevice, filePath);
}
template <>
Shader<ID3D11GeometryShader>::Shader(ID3D11Device* const pDevice, const LPCWSTR filePath)
{
	LoadShader(pDevice, filePath);
}
template <>
Shader<ID3D11ComputeShader>::Shader(ID3D11Device* const pDevice, const LPCWSTR filePath)
{
	LoadShader(pDevice, filePath);
}

// shader loading
template<>
void Shader<ID3D11VertexShader>::LoadShader(ID3D11Device* const pDevice, const LPCWSTR filePath)
{
	DeleteShader();

	HRESULT hr = {};
	Microsoft::WRL::ComPtr<ID3DBlob> pBlob;

	hr = D3DReadFileToBlob(filePath, &pBlob);

	pDevice->CreateVertexShader(
		pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(),
		nullptr, &pShader);
	

	CreateInputLayout(pDevice, pBlob->GetBufferPointer(), pBlob->GetBufferSize());
}
template<>
void Shader<ID3D11PixelShader>::LoadShader(ID3D11Device* const pDevice, const LPCWSTR filePath)
{
	DeleteShader();

	HRESULT hr = {};
	Microsoft::WRL::ComPtr<ID3DBlob> pBlob;

	hr = D3DReadFileToBlob(filePath, &pBlob);

	pDevice->CreatePixelShader(
		pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(),
		nullptr, &pShader);
	
}
template<>
void Shader<ID3D11GeometryShader>::LoadShader(ID3D11Device* const pDevice, const LPCWSTR filePath)
{
	DeleteShader();

	HRESULT hr = {};
	Microsoft::WRL::ComPtr<ID3DBlob> pBlob;

	hr = D3DReadFileToBlob(filePath, &pBlob);

	pDevice->CreateGeometryShader(
		pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(),
		nullptr, &pShader);
	
}
template<>
void Shader<ID3D11ComputeShader>::LoadShader(ID3D11Device* const pDevice, const LPCWSTR filePath)
{
	DeleteShader();

	HRESULT hr = {};
	Microsoft::WRL::ComPtr<ID3DBlob> pBlob;

	hr = D3DReadFileToBlob(filePath, &pBlob);

	pDevice->CreateComputeShader(
		pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(),
		nullptr, &pShader);
	
}

// shader and resource binding
template<>
void Shader<ID3D11VertexShader>::Bind(ID3D11DeviceContext* const pDeviceContext) const
{
	pDeviceContext->IASetInputLayout(pInputLayout.Get());
	pDeviceContext->VSSetShader(pShader, nullptr, 0u);

	if (cbs.size() > 0) pDeviceContext->VSSetConstantBuffers(0u, static_cast<UINT>(cbs.size()), cbs.data());
	if (srvs.size() > 0) pDeviceContext->VSSetShaderResources(0u, static_cast<UINT>(srvs.size()), srvs.data());
}
template<>
void Shader<ID3D11PixelShader>::Bind(ID3D11DeviceContext* const pDeviceContext) const
{
	pDeviceContext->PSSetShader(pShader, nullptr, 0u);

	if (cbs.size() > 0) pDeviceContext->PSSetConstantBuffers(0u, static_cast<UINT>(cbs.size()), cbs.data());
	if (srvs.size() > 0) pDeviceContext->PSSetShaderResources(0u, static_cast<UINT>(srvs.size()), srvs.data());
}
template<>
void Shader<ID3D11GeometryShader>::Bind(ID3D11DeviceContext* const pDeviceContext) const
{
	pDeviceContext->GSSetShader(pShader, nullptr, 0u);

	if (cbs.size() > 0) pDeviceContext->GSSetConstantBuffers(0u, static_cast<UINT>(cbs.size()), cbs.data());
	if (srvs.size() > 0) pDeviceContext->GSSetShaderResources(0u, static_cast<UINT>(srvs.size()), srvs.data());
}

// shader and resource unbinding
template<>
void Shader<ID3D11VertexShader>::Unbind(ID3D11DeviceContext* const pDeviceContext) const
{
	pDeviceContext->VSSetShader(nullptr, nullptr, 0u);

	if (nullCBs.size() > 0) pDeviceContext->VSSetConstantBuffers(0u, static_cast<UINT>(nullCBs.size()), nullCBs.data());
	if (nullSRVs.size() > 0) pDeviceContext->VSSetShaderResources(0u, static_cast<UINT>(nullSRVs.size()), nullSRVs.data());
}
template<>
void Shader<ID3D11PixelShader>::Unbind(ID3D11DeviceContext* const pDeviceContext) const
{
	pDeviceContext->PSSetShader(nullptr, nullptr, 0u);

	if (nullCBs.size() > 0) pDeviceContext->PSSetConstantBuffers(0u, static_cast<UINT>(nullCBs.size()), nullCBs.data());
	if (nullSRVs.size() > 0) pDeviceContext->PSSetShaderResources(0u, static_cast<UINT>(nullSRVs.size()), nullSRVs.data());
}
template<>
void Shader<ID3D11GeometryShader>::Unbind(ID3D11DeviceContext* const pDeviceContext) const
{
	pDeviceContext->GSSetShader(nullptr, nullptr, 0u);

	if (nullCBs.size() > 0) pDeviceContext->PSSetConstantBuffers(0u, static_cast<UINT>(nullCBs.size()), nullCBs.data());
	if (nullSRVs.size() > 0) pDeviceContext->PSSetShaderResources(0u, static_cast<UINT>(nullSRVs.size()), nullSRVs.data());
}

template<class T>
void Shader<T>::CreateInputLayout(ID3D11Device* const pDevice, LPCVOID bufferPointer, SIZE_T bufferSize)
{
	HRESULT hr;

	// get reflection
	ID3D11ShaderReflection* pReflector = nullptr;
	hr = D3DReflect(bufferPointer, bufferSize,
		__uuidof(ID3D11ShaderReflection), (void**)&pReflector);
	

	// get shader desc
	D3D11_SHADER_DESC shaderDesc = {};
	hr = pReflector->GetDesc(&shaderDesc);
	

	// create input description array
	D3D11_SIGNATURE_PARAMETER_DESC signParamDesc = {};
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputDesc(shaderDesc.InputParameters);
	for (UINT iParam = 0; iParam < shaderDesc.InputParameters; ++iParam)
	{
		// get input param desc
		hr = pReflector->GetInputParameterDesc(iParam, &signParamDesc);
		

		// build input element descr
		inputDesc[iParam] = {
			signParamDesc.SemanticName, signParamDesc.SemanticIndex,
			InterpretFormat(signParamDesc.Mask, signParamDesc.ComponentType), 0u,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA, 0u };
	}

	hr = pDevice->CreateInputLayout(
		inputDesc.data(), shaderDesc.InputParameters,
		bufferPointer,
		bufferSize,
		&pInputLayout);
	

	pReflector->Release();
}

template<class T>
DXGI_FORMAT Shader<T>::InterpretFormat(const BYTE mask, const D3D_REGISTER_COMPONENT_TYPE componentType) const
{
	auto registerBits = std::bitset<4>(mask);
	UINT registerCount = 0u;
	for (; registerCount < 4u; registerCount++)
	{
		if (!registerBits[registerCount]) break;
	}

	switch (componentType)
	{
		case D3D_REGISTER_COMPONENT_FLOAT32:
			switch (registerCount)
			{
				default:
				case 0u: return DXGI_FORMAT_R32G32B32A32_FLOAT;
				case 1u: return DXGI_FORMAT_R32_FLOAT;
				case 2u: return DXGI_FORMAT_R32G32_FLOAT;
				case 3u: return DXGI_FORMAT_R32G32B32_FLOAT;
				case 4u: return DXGI_FORMAT_R32G32B32A32_FLOAT;
			}

		case D3D_REGISTER_COMPONENT_UINT32:
			switch (registerCount)
			{
				default:
				case 0u: return DXGI_FORMAT_R32G32B32A32_UINT;
				case 1u: return DXGI_FORMAT_R32_UINT;
				case 2u: return DXGI_FORMAT_R32G32_UINT;
				case 3u: return DXGI_FORMAT_R32G32B32_UINT;
				case 4u: return DXGI_FORMAT_R32G32B32A32_UINT;
			}

		case D3D_REGISTER_COMPONENT_SINT32:
			switch (registerCount)
			{
				default:
				case 0u: return DXGI_FORMAT_R32G32B32A32_SINT;
				case 1u: return DXGI_FORMAT_R32_SINT;
				case 2u: return DXGI_FORMAT_R32G32_SINT;
				case 3u: return DXGI_FORMAT_R32G32B32_SINT;
				case 4u: return DXGI_FORMAT_R32G32B32A32_SINT;
			}

		default:
		case D3D_REGISTER_COMPONENT_UNKNOWN:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
	}
}
