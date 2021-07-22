#pragma once
struct Vertex
{
	D3DXVECTOR4 position;
	D3DXVECTOR2 uv;
};
constexpr DWORD VertexFVF = D3DFVF_XYZRHW | D3DFVF_TEX1;

struct Index16
{
	unsigned short _0, _1, _2;
};

class VIBuffer
{
private:
	LPDIRECT3DVERTEXBUFFER9 vb;
	LPDIRECT3DINDEXBUFFER9 ib;

	D3DFORMAT idxFormat;

	DWORD vtxSize;
	DWORD vtxCount;
	DWORD vtxFVF;
	DWORD triCount;
	DWORD idxSize;

public:
	explicit VIBuffer() = default;
	__forceinline ~VIBuffer();

	__forceinline HRESULT Load(LPDIRECT3DDEVICE9 device, float x, float y, float width, float height);
	void Render(LPDIRECT3DDEVICE9 device);
};

VIBuffer::~VIBuffer()
{
	vb->Release();
	ib->Release();
}

HRESULT VIBuffer::Load(LPDIRECT3DDEVICE9 device, float x, float y, float width, float height)
{
	vtxCount = 4;
	vtxFVF = VertexFVF;
	vtxSize = sizeof(Vertex);
	triCount = 2;
	idxFormat = D3DFMT_INDEX16;
	idxSize = sizeof(Index16);

	if (FAILED(device->CreateVertexBuffer(vtxSize * vtxCount, 0, vtxFVF, D3DPOOL_MANAGED, &vb, nullptr)))
		return E_FAIL;

	if (FAILED(device->CreateIndexBuffer(idxSize * triCount, 0, idxFormat, D3DPOOL_MANAGED, &ib, nullptr)))
		return E_FAIL;

	Vertex* vertices = nullptr;
	vb->Lock(0, 0, reinterpret_cast<void**>(&vertices), 0);

	vertices[0].position = D3DXVECTOR4(x, y, 0.f, 1.f);
	vertices[0].uv = D3DXVECTOR2(0.f, 0.f);

	vertices[1].position = D3DXVECTOR4(x + width, y, 0.f, 1.f);
	vertices[1].uv = D3DXVECTOR2(1.f, 0.f);

	vertices[2].position = D3DXVECTOR4(x + width, y + height, 0.f, 1.f);
	vertices[2].uv = D3DXVECTOR2(1.f, 1.f);

	vertices[3].position = D3DXVECTOR4(x, y + height, 0.f, 1.f);
	vertices[3].uv = D3DXVECTOR2(0.f, 1.f);

	vb->Unlock();

	Index16* indices = nullptr;
	ib->Lock(0, 0, reinterpret_cast<void**>(&indices), 0);

	indices[0]._0 = 0;
	indices[0]._1 = 1;
	indices[0]._2 = 2;

	indices[1]._0 = 0;
	indices[1]._1 = 2;
	indices[1]._2 = 3;

	ib->Unlock();

	return S_OK;
}

