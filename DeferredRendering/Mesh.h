#pragma once
#include <string>
#include "Shader.h"
class Mesh
{
private:
	LPD3DXMESH originalMesh;
	LPD3DXMESH mesh;
	LPD3DXBUFFER adjacency;
	LPD3DXBUFFER subsetBuffer;

	D3DXMATERIAL* subset;
	LPDIRECT3DTEXTURE9* textures;

	DWORD subsetCount;

public:
	explicit Mesh() = default;
	__forceinline ~Mesh();

	__forceinline HRESULT Load(LPDIRECT3DDEVICE9 device, const std::wstring& path, const std::wstring& name);
	void Render(const Shader* shader);
};

Mesh::~Mesh()
{
	for (DWORD i = 0; i < subsetCount; ++i)
		textures[i]->Release();

	mesh->Release();
	originalMesh->Release();
	adjacency->Release();
	subsetBuffer->Release();

	delete[] textures;
}

HRESULT
Mesh::Load(LPDIRECT3DDEVICE9 device, const std::wstring& path, const std::wstring& name)
{
	wchar_t fullPath[MAX_PATH] = L"";
	
	lstrcpy(fullPath, path.c_str());
	lstrcat(fullPath, name.c_str());

	if (FAILED(D3DXLoadMeshFromX(fullPath, D3DXMESH_MANAGED, device, &adjacency, &subsetBuffer, NULL, &subsetCount, &originalMesh)))
	{
		MessageBoxA(NULL, "MESH_LOAD_FAILED", "FAIL", MB_OK);
		return E_FAIL;
	}

	DWORD FVF = originalMesh->GetFVF();

	if (!(FVF & D3DFVF_NORMAL))
	{
		originalMesh->CloneMeshFVF(originalMesh->GetOptions(), FVF | D3DFVF_NORMAL, device, &mesh);
		D3DXComputeNormals(mesh, reinterpret_cast<DWORD*>(adjacency->GetBufferPointer()));
	}
	else
		originalMesh->CloneMeshFVF(originalMesh->GetOptions(), FVF, device, &mesh);

	subset = reinterpret_cast<D3DXMATERIAL*>(subsetBuffer->GetBufferPointer());
	textures = new LPDIRECT3DTEXTURE9[subsetCount];

	for (DWORD i = 0; i < subsetCount; ++i)
	{
		textures[i] = nullptr;
		if (subset[i].pTextureFilename == nullptr)
			continue;

		wchar_t fileName[MAX_PATH] = L"";
		lstrcpy(fullPath, path.c_str());

		MultiByteToWideChar(CP_ACP, 0, subset[i].pTextureFilename, strlen(subset[i].pTextureFilename), fileName, MAX_PATH);

		lstrcat(fullPath, fileName);

		if (FAILED(D3DXCreateTextureFromFile(device, fullPath, &textures[i])))
		{
			MessageBoxA(NULL, "TEXTURE_LOAD_FAIL", "FAIL", MB_OK);
			return E_FAIL;
		}
	}

	return S_OK;
}
