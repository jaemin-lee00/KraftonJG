// URenderer.h
#pragma once

#include "Utility.h"

// ImGui 헤더 파일
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

class URenderer
{
public:
    // Direct3D 11 장치, 컨텍스트 및 스왑 체인
    ID3D11Device* Device = nullptr;
    ID3D11DeviceContext* DeviceContext = nullptr;
    IDXGISwapChain* SwapChain = nullptr;

    // 렌더링에 필요한 리소스 및 상태 변수들
    ID3D11Texture2D* FrameBuffer = nullptr;
    ID3D11RenderTargetView* FrameBufferRTV = nullptr;
    ID3D11RasterizerState* RasterizerState = nullptr;
    ID3D11Buffer* ConstantBuffer = nullptr;

    FLOAT ClearColor[4];
    D3D11_VIEWPORT ViewportInfo;

    // 셰이더 관련 변수들
    ID3D11VertexShader* SimpleVertexShader = nullptr;
    ID3D11PixelShader* SimplePixelShader = nullptr;
    ID3D11InputLayout* SimpleInputLayout = nullptr;
    UINT Stride = 0;

public:
    URenderer();
    ~URenderer();

    void Create(HWND hWindow);
    void Release();

    void Prepare();
    void PrepareShader();

    void CreateShader();
    void ReleaseShader();

    void CreateConstantBuffer();
    void ReleaseConstantBuffer();
    void UpdateConstant(FVector3 Offset);

    void SwapBuffer();

    ID3D11Buffer* CreateVertexBuffer(const FVertexSimple* vertices, UINT byteWidth);
    void ReleaseVertexBuffer(ID3D11Buffer* vertexBuffer);
    void RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices);

private:
    void CreateDeviceAndSwapChain(HWND hWindow);
    void ReleaseDeviceAndSwapChain();

    void CreateFrameBuffer();
    void ReleaseFrameBuffer();

    void CreateRasterizerState();
    void ReleaseRasterizerState();
};
