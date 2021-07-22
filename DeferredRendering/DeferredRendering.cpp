#include "DXUT.h"
#include "Mesh.h"
#include "Shader.h"
#include "VIBuffer.h"
#include "resource.h"
#include <memory>

constexpr int screenWidth = 1280;
constexpr int screenHeight = 720;

D3DXVECTOR3 position = { 0, 0, 0 };

D3DXMATRIX world;
D3DXMATRIX view;
D3DXMATRIX proj;

Mesh* mesh;
Shader* shader;

VIBuffer* diffusebuffer;
VIBuffer* normalbuffer;

LPDIRECT3DTEXTURE9 diffuseRenderTarget;
LPDIRECT3DSURFACE9 diffusetargetSurface;

LPDIRECT3DTEXTURE9 normalRenderTarget;
LPDIRECT3DSURFACE9 normaltargetSurface;

LPDIRECT3DSURFACE9 backBuffer;

D3DCOLOR clearColor = D3DCOLOR_ARGB(0, 45, 50, 170);

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
    
    shader = new Shader;
    shader->Load(pd3dDevice, L"testShader.fx");

    pd3dDevice->SetRenderState(D3DRS_LIGHTING, false);

    D3DVIEWPORT9 viewPort;
    pd3dDevice->GetViewport(&viewPort);
    if (FAILED(D3DXCreateTexture(pd3dDevice, 
        viewPort.Width, viewPort.Height, 
        1, 
        D3DUSAGE_RENDERTARGET, 
        D3DFMT_A8B8G8R8, 
        D3DPOOL_DEFAULT, 
        &diffuseRenderTarget)))
    {
        MessageBoxA(NULL, "DIFFUSE_RENDER_TARGET_CREATE_FAILED", "FAIL", MB_OK);
        return E_FAIL;
    }

    if (FAILED(diffuseRenderTarget->GetSurfaceLevel(0, &diffusetargetSurface)))
        return E_FAIL;

    if (FAILED(D3DXCreateTexture(pd3dDevice,
        viewPort.Width, viewPort.Height,
        1,
        D3DUSAGE_RENDERTARGET,
        D3DFMT_A16B16G16R16F,
        D3DPOOL_DEFAULT,
        &normalRenderTarget)))
    {
        MessageBoxA(NULL, "NORMAL_RENDER_TARGET_CREATE_FAILED", "FAIL", MB_OK);
        return E_FAIL;
    }

    if (FAILED(normalRenderTarget->GetSurfaceLevel(0, &normaltargetSurface)))
        return E_FAIL;

    diffusebuffer = new VIBuffer;
    diffusebuffer->Load(pd3dDevice, 0, 0, 150, 150);

    normalbuffer = new VIBuffer;
    normalbuffer->Load(pd3dDevice, 0, 150, 150, 150);

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
    D3DXMatrixTranslation(&world, position.x, position.y, position.z);
}

void CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    HRESULT hr;

    V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 45, 50, 170), 1.0f, 0 ) );

    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
        pd3dDevice->GetRenderTarget(0, &backBuffer);
        pd3dDevice->SetRenderTarget(0, diffusetargetSurface);

        pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET, clearColor, 1.f, 0);

        pd3dDevice->SetRenderTarget(0, backBuffer);
        backBuffer->Release();

        pd3dDevice->GetRenderTarget(0, &backBuffer);
        pd3dDevice->SetRenderTarget(0, normaltargetSurface);

        pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET, clearColor, 1.f, 0);

        pd3dDevice->SetRenderTarget(0, backBuffer);
        backBuffer->Release();

        pd3dDevice->SetRenderTarget(1, diffusetargetSurface);
        pd3dDevice->SetRenderTarget(2, normaltargetSurface);



        const LPD3DXEFFECT effect = shader->GetEffect();
        D3DXMATRIX result = world * view * proj;
        effect->SetMatrix((D3DXHANDLE)"WVP", &result);
        effect->SetMatrix((D3DXHANDLE)"W", &world);

        effect->Begin(NULL, 0);
        effect->BeginPass(0);

        mesh->Render(shader);
        
        effect->EndPass();
        effect->End();

        pd3dDevice->SetTexture(0, diffuseRenderTarget);
        diffusebuffer->Render(pd3dDevice);

        pd3dDevice->SetTexture(0, normalRenderTarget);
        normalbuffer->Render(pd3dDevice);

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
    diffuseRenderTarget->Release();
    diffusetargetSurface->Release();
    
    normalRenderTarget->Release();
    normaltargetSurface->Release();

    delete shader;
    delete mesh;
    delete diffusebuffer;
    delete normalbuffer;
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
    DXUTCreateDevice( false, screenWidth, screenHeight );

    DXUTMainLoop();

    return DXUTGetExitCode();
}