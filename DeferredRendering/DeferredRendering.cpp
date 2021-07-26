#include "DXUT.h"
#include "Mesh.h"
#include "Shader.h"
#include "VIBuffer.h"
#include "resource.h"
#include <memory>

constexpr int screenWidth = 1280;
constexpr int screenHeight = 720;

D3DXVECTOR3 position = { 0, 0, 0 };
float rotValue = 0.f;

D3DXMATRIX world;
D3DXMATRIX view;
D3DXMATRIX proj;

Mesh* mesh;

struct RenderTarget
{
    VIBuffer* buffer;

    LPDIRECT3DTEXTURE9 renderTarget;
    LPDIRECT3DSURFACE9 surface;

    LPDIRECT3DSURFACE9 oldSurface;

    D3DCOLOR clearColor = D3DCOLOR_ARGB(0, 45, 50, 0);

    void SetRenderTarget(LPDIRECT3DDEVICE9 device, DWORD index)
    {
        device->GetRenderTarget(index, &oldSurface);
        device->SetRenderTarget(index, surface);
    };
    
    void End(LPDIRECT3DDEVICE9 device, DWORD index) 
    {
        device->SetRenderTarget(index, oldSurface);
        if (oldSurface != nullptr)
            oldSurface->Release();
    };

    void Clear(LPDIRECT3DDEVICE9 device) 
    {
        SetRenderTarget(device, 0);
        device->Clear(0, NULL, D3DCLEAR_TARGET, clearColor, 1.f, 0);
        End(device, 0);
    };
};

enum RT { DIFFUSE, NORMAL, LIGHT, END };
RenderTarget renderTargets[END];

VIBuffer* lightBuffer;
VIBuffer* blendBuffer;

Shader* meshShader;
Shader* deferred;
Shader* blend;


bool CALLBACK IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat,
                                      bool bWindowed, void* pUserContext )
{
    IDirect3D9* pD3D = DXUTGetD3D9Object();
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                                         AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
                                         D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

    return true;
}

bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    return true;
}

void Initialize(LPDIRECT3DDEVICE9 device)
{
    D3DVIEWPORT9 viewPort;
    device->GetViewport(&viewPort);

    for (int i = 0; i < END; ++i)
    {
        renderTargets[i].buffer = new VIBuffer;
        renderTargets[i].buffer->Load(device, 0, 0 + (i * 200), 200, 200);

        if (i == 0) 
        {
            if (FAILED(D3DXCreateTexture(device,
                viewPort.Width, viewPort.Height,
                1,
                D3DUSAGE_RENDERTARGET,
                D3DFMT_A8B8G8R8,
                D3DPOOL_DEFAULT,
                &renderTargets[i].renderTarget)))
            {
                MessageBoxA(NULL, "DIFFUSE_RENDER_TARGET_CREATE_FAILED", "FAIL", MB_OK);
                return;
            }
        }
        else
        {
            if (FAILED(D3DXCreateTexture(device,
                viewPort.Width, viewPort.Height,
                1,
                D3DUSAGE_RENDERTARGET,
                D3DFMT_A16B16G16R16F,
                D3DPOOL_DEFAULT,
                &renderTargets[i].renderTarget)))
            {
                MessageBoxA(NULL, "NORMAL_RENDER_TARGET_CREATE_FAILED", "FAIL", MB_OK);
                return;
            }
        }
        
        renderTargets[i].renderTarget->GetSurfaceLevel(0, &renderTargets[i].surface);
    }

    lightBuffer = new VIBuffer;
    lightBuffer->Load(device, 0, 0, viewPort.Width, viewPort.Height);

    blendBuffer = new VIBuffer;
    blendBuffer->Load(device, 0, 0, viewPort.Width, viewPort.Height);
}

HRESULT CALLBACK OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext )
{
    const D3DSURFACE_DESC* bbsd = DXUTGetD3D9BackBufferSurfaceDesc();
    D3DXMatrixPerspectiveFovLH(&proj, D3DXToRadian(70),
        static_cast<FLOAT>(bbsd->Width) /
        static_cast<FLOAT>(bbsd->Height),
        1,
        500);
    pd3dDevice->SetTransform(D3DTS_PROJECTION, &proj);

    D3DXVECTOR3 eye = { 0.f, 3.1f, -0.001f };
    D3DXVECTOR3 lookat = { 0.f, -1.f, 0.f };
    D3DXVECTOR3 up = { 0.f, 1.f, 0.f };
    D3DXMatrixLookAtLH(&view, &eye, &lookat, &up);
    pd3dDevice->SetTransform(D3DTS_VIEW, &view);

    D3DXMatrixTranslation(&world, position.x, position.y, position.z);
    pd3dDevice->SetTransform(D3DTS_WORLD, &world);

    mesh = new Mesh;
    mesh->Load(pd3dDevice, L"Resources/", L"Missile.X");
    
    meshShader = new Shader;
    meshShader->Load(pd3dDevice, L"meshShader.fx");

    deferred = new Shader;
    deferred->Load(pd3dDevice, L"lightAccumulate.fx");

    blend = new Shader;
    blend->Load(pd3dDevice, L"Blend.fx");

    Initialize(pd3dDevice);

    pd3dDevice->SetRenderState(D3DRS_LIGHTING, false);

    return S_OK;
}

HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                    void* pUserContext )
{
    return S_OK;
}

void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    if (DXUTIsKeyDown('W'))
        position.z += 3.f * fElapsedTime;
    if (DXUTIsKeyDown('S'))
        position.z -= 3.f * fElapsedTime;
    if (DXUTIsKeyDown('A'))
        position.x -= 3.f * fElapsedTime;
    if (DXUTIsKeyDown('D'))
        position.x += 3.f * fElapsedTime;
    D3DXMATRIX trans;
    D3DXMatrixTranslation(&trans, position.x, position.y, position.z);
    
    D3DXMATRIX rot;
    if (DXUTIsKeyDown('Q'))
        rotValue += 3.f * fElapsedTime;
    
    if (DXUTIsKeyDown('E'))
        rotValue -= 3.f * fElapsedTime;

    D3DXMatrixRotationZ(&rot, rotValue);

    world = rot * trans;
}

void RenderMesh(LPDIRECT3DDEVICE9 pd3dDevice)
{
    renderTargets[DIFFUSE].Clear(pd3dDevice);
    renderTargets[NORMAL].Clear(pd3dDevice);
    
    renderTargets[DIFFUSE].SetRenderTarget(pd3dDevice, 0);
    renderTargets[NORMAL].SetRenderTarget(pd3dDevice, 1);

    const LPD3DXEFFECT effect = meshShader->GetEffect();
    D3DXMATRIX result = world * view * proj;
    effect->SetMatrix((D3DXHANDLE)"WVP", &result);
    effect->SetMatrix((D3DXHANDLE)"W", &world);

    effect->Begin(NULL, 0);
    effect->BeginPass(0);

    mesh->Render(meshShader);

    effect->EndPass();
    effect->End();

    renderTargets[DIFFUSE].End(pd3dDevice, 0);
    renderTargets[NORMAL].End(pd3dDevice, 1);
}

void AccumulateLight(LPDIRECT3DDEVICE9 pd3dDevice)
{
    renderTargets[LIGHT].Clear(pd3dDevice);
    renderTargets[LIGHT].SetRenderTarget(pd3dDevice, 0);

    const LPD3DXEFFECT effect = deferred->GetEffect();
    effect->SetTexture((D3DXHANDLE)"NormalTexture", renderTargets[NORMAL].renderTarget);
    effect->SetValue((D3DXHANDLE)"lightDir", D3DXVECTOR4(0, -1.f, 0, 0), sizeof(D3DXVECTOR4));

    effect->Begin(NULL, 0);
    effect->BeginPass(0);

    lightBuffer->Render(pd3dDevice);

    effect->EndPass();
    effect->End();

    renderTargets[LIGHT].End(pd3dDevice, 0);
}

void Blend(LPDIRECT3DDEVICE9 pd3dDevice)
{
    const LPD3DXEFFECT effect = blend->GetEffect();
    effect->SetTexture((D3DXHANDLE)"diffuseTexture", renderTargets[DIFFUSE].renderTarget);
    effect->SetTexture((D3DXHANDLE)"lightAccumulatedTexture", renderTargets[LIGHT].renderTarget);

    effect->Begin(NULL, 0);
    effect->BeginPass(0);

    blendBuffer->Render(pd3dDevice);

    effect->EndPass();
    effect->End();
}

void CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    HRESULT hr;

    V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 45, 50, 170), 1.0f, 0 ) );

    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
        RenderMesh(pd3dDevice);

        AccumulateLight(pd3dDevice);

        Blend(pd3dDevice);

        for (int i = 0; i < END; ++i)
        {
            pd3dDevice->SetTexture(0, renderTargets[i].renderTarget);
            renderTargets[i].buffer->Render(pd3dDevice);
        }

        V( pd3dDevice->EndScene() );
    }
}

LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                          bool* pbNoFurtherProcessing, void* pUserContext )
{
    return 0;
}

void CALLBACK OnD3D9LostDevice( void* pUserContext )
{
}

void CALLBACK OnD3D9DestroyDevice( void* pUserContext )
{
    for (int i = 0; i < END; ++i)
    {
        delete renderTargets[i].buffer;
        renderTargets[i].renderTarget->Release();
        renderTargets[i].surface->Release();
    }
    
    delete lightBuffer;
    delete blendBuffer;

    delete meshShader;
    delete deferred;
    delete blend;

    delete mesh;
}

INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    DXUTSetCallbackD3D9DeviceAcceptable( IsD3D9DeviceAcceptable );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
    DXUTSetCallbackD3D9DeviceCreated( OnD3D9CreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnD3D9ResetDevice );
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackD3D9FrameRender( OnD3D9FrameRender );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackD3D9DeviceLost( OnD3D9LostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnD3D9DestroyDevice );

    DXUTInit( true, true );
    DXUTSetHotkeyHandling( true, true, true );
    DXUTSetCursorSettings( true, true );
    DXUTCreateWindow( L"DeferredRendering" );
    DXUTCreateDevice( true, screenWidth, screenHeight );

    DXUTMainLoop();

    return DXUTGetExitCode();
}