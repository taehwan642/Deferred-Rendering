#include "DXUT.h"
#include "VIBuffer.h"

void VIBuffer::Render(LPDIRECT3DDEVICE9 device)
{
	device->SetStreamSource(0, vb, 0, vtxSize);
	device->SetFVF(vtxFVF);
	device->SetIndices(ib);
	device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vtxCount, 0, triCount);
}