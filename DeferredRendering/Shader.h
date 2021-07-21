#pragma once
#include <string>
class Shader
{
private:
	LPD3DXEFFECT effect;

public:
	explicit Shader();
	
	void Load(LPDIRECT3DDEVICE9 device, const std::wstring& path);

	~Shader();
};