#pragma once
#include <string>
class Shader
{
private:
	LPD3DXEFFECT effect;

public:
	explicit Shader() = default;
	__forceinline ~Shader();
	
	__forceinline HRESULT Load(LPDIRECT3DDEVICE9 device, const std::wstring& path);

	[[nodiscard]] __forceinline constexpr LPD3DXEFFECT const GetEffect() const { return effect; };
};

Shader::~Shader()
{
	effect->Release();
}

HRESULT
Shader::Load(LPDIRECT3DDEVICE9 device, const std::wstring& path)
{
	LPD3DXBUFFER errorMessage = nullptr;

	if (FAILED(D3DXCreateEffectFromFile(device, path.c_str(), NULL, NULL, D3DXSHADER_DEBUG, NULL, &effect, &errorMessage)))
	{
		MessageBoxA(NULL, (char*)errorMessage->GetBufferPointer(), "SHADER_ERROR", MB_OK);
		return E_FAIL;
	}
	else if (errorMessage != nullptr)
	{
		MessageBoxA(NULL, (char*)errorMessage->GetBufferPointer(), "SHADER_WARNING", MB_OK);
	}

	return S_OK;
}