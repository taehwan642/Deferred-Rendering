#pragma once
#include <string>
#include "Shader.h"
class Mesh
{
private:
protected:
	explicit Mesh();

	void Load(LPDIRECT3DDEVICE9 device, const std::wstring& path, const std::wstring& name);
	void Render(const Shader* shader);

	~Mesh();
};