#include "DXUT.h"
#include "Mesh.h"

void Mesh::Render(const Shader* shader)
{
	const LPD3DXEFFECT effect = shader->GetEffect();
	for (size_t i = 0; i < subsetCount; ++i)
	{
		effect->SetTexture((D3DXHANDLE)"Tex", textures[i]);
		effect->CommitChanges();
		mesh->DrawSubset(i);
	}
}