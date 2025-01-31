#include <windows.h>
#include "Utility.h"
#include "FVertexSimple.h"
#include "URenderer.h"
#include "Sphere.h"

#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
    {
        return true;
    }

    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

void InitializeImGui(HWND hWnd, URenderer& renderer)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplWin32_Init((void*)hWnd);
    ImGui_ImplDX11_Init(renderer.Device, renderer.DeviceContext);
}

void CleanupImGui()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void RenderImGui(bool& bBoundBallToScreen, bool& bPinballMovement)
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Jungle Property Window");
    ImGui::Text("Hello, Jungle!");
    ImGui::Checkbox("Bound Ball to Screen", &bBoundBallToScreen);
    ImGui::Checkbox("Pinball Movement", &bPinballMovement);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void HandleKeyboardInput(FVector3& offset, bool bBoundBallToScreen, float scaleMod, float sphereRadius, const float leftBorder, const float rightBorder, const float topBorder, const float bottomBorder)
{
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (msg.message == WM_QUIT)
        {
            PostQuitMessage(0);
            return;
        }
        else if (msg.message == WM_KEYDOWN)
        {
            if (msg.wParam == VK_LEFT) offset.x -= 0.01f;
            else if (msg.wParam == VK_RIGHT) offset.x += 0.01f;
            else if (msg.wParam == VK_UP) offset.y += 0.01f;
            else if (msg.wParam == VK_DOWN) offset.y -= 0.01f;

            if (bBoundBallToScreen)
            {
                float renderRadius = sphereRadius * scaleMod;
                if (offset.x < leftBorder + renderRadius) offset.x = leftBorder + renderRadius;
                if (offset.x > rightBorder - renderRadius) offset.x = rightBorder - renderRadius;
                if (offset.y < topBorder + renderRadius) offset.y = topBorder + renderRadius;
                if (offset.y > bottomBorder - renderRadius) offset.y = bottomBorder - renderRadius;
            }
        }
    }
}

void UpdatePinballMovement(FVector3& offset, FVector3& velocity, float scaleMod, float sphereRadius, const float leftBorder, const float rightBorder, const float topBorder, const float bottomBorder)
{
    offset.x += velocity.x;
    offset.y += velocity.y;
    offset.z += velocity.z;

    float renderRadius = sphereRadius * scaleMod;
    if (offset.x < leftBorder + renderRadius) velocity.x *= -1.0f;
    if (offset.x > rightBorder - renderRadius) velocity.x *= -1.0f;
    if (offset.y < topBorder + renderRadius) velocity.y *= -1.0f;
    if (offset.y > bottomBorder - renderRadius) velocity.y *= -1.0f;
}

void SetVertexBufferData(URenderer& renderer, ID3D11Buffer* vertexBuffers[], UINT numVertices[], ETypePrimitive typePrimitive)
{
    renderer.RenderPrimitive(vertexBuffers[typePrimitive], numVertices[typePrimitive]);
}

void ReleaseResources(URenderer& renderer, ID3D11Buffer* vertexBuffers[], int bufferCount)
{
    for (int i = 0; i < bufferCount; ++i)
    {
        renderer.ReleaseVertexBuffer(vertexBuffers[i]);
    }
    renderer.ReleaseConstantBuffer();
    renderer.ReleaseShader();
    renderer.Release();
}

void MainLoop(HWND hWnd, URenderer& renderer, ID3D11Buffer* vertexBuffers[], UINT numVertices[], int bufferCount)
{
    bool bIsExit = false;
    ETypePrimitive typePrimitive = EPT_Sphere;

    FVector3 offset(0.0f);
    FVector3 velocity(0.0f);

    const float leftBorder = -1.0f;
    const float rightBorder = 1.0f;
    const float topBorder = -1.0f;
    const float bottomBorder = 1.0f;
    const float sphereRadius = 1.0f;

    bool bBoundBallToScreen = true;
    bool bPinballMovement = true;
    const float ballSpeed = 0.001f;
    velocity.x = ((float)(rand() % 100 - 50)) * ballSpeed;
    velocity.y = ((float)(rand() % 100 - 50)) * ballSpeed;

    const int targetFPS = 30; // 타겟 FPS를 30으로 설정
    const double targetFrameTime = 1.0 / targetFPS;
    const double fixedTimeStep = 1.0 / 120.0; // 고정 시간 스텝 (120 FPS) 초 단위

    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    LARGE_INTEGER startTime, endTime;
    double elapsedTime = 0.0;
    double accumulator = 0.0;

    while (!bIsExit)
    {
        QueryPerformanceCounter(&startTime);

        HandleKeyboardInput(offset, bBoundBallToScreen, 0.1f, sphereRadius, leftBorder, rightBorder, topBorder, bottomBorder);

        // 물리 업데이트
        while (accumulator >= fixedTimeStep)
        {
            if (bPinballMovement)
            {
                UpdatePinballMovement(offset, velocity, 0.1f, sphereRadius, leftBorder, rightBorder, topBorder, bottomBorder);
            }
            accumulator -= fixedTimeStep;
        }

        renderer.Prepare();
        renderer.PrepareShader();
        renderer.UpdateConstant(offset);

        SetVertexBufferData(renderer, vertexBuffers, numVertices, typePrimitive);

        RenderImGui(bBoundBallToScreen, bPinballMovement);
        renderer.SwapBuffer();

        QueryPerformanceCounter(&endTime);
        elapsedTime = (endTime.QuadPart - startTime.QuadPart) / static_cast<double>(frequency.QuadPart);
        accumulator += elapsedTime;

        do
        {
            Sleep(0);

            // 루프 종료 시간 기록
            QueryPerformanceCounter(&endTime);

            // 한 프레임이 소요된 시간 계산 (밀리초 단위로 변환)
            elapsedTime = (endTime.QuadPart - startTime.QuadPart) * 1000.0 / frequency.QuadPart;

        } while (elapsedTime < targetFrameTime);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    WCHAR WindowClass[] = L"JungleWindowClass";
    WCHAR Title[] = L"Game Tech Lab";

    WNDCLASSW wndclass = { 0, WndProc, 0, 0, 0, 0, 0, 0, 0, WindowClass };
    RegisterClassW(&wndclass);

    HWND hWnd = CreateWindowExW(0, WindowClass, Title, WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1024, 1024, nullptr, nullptr, hInstance, nullptr);

    URenderer renderer;
    renderer.Create(hWnd);
    renderer.CreateShader();
    renderer.CreateConstantBuffer();

    InitializeImGui(hWnd, renderer);

    UINT numVertices[] = {
        sizeof(triangle_vertices) / sizeof(FVertexSimple),
        sizeof(cube_vertices) / sizeof(FVertexSimple),
        sizeof(sphere_vertices) / sizeof(FVertexSimple)
    };

    float scaleMod = 0.1f;
    for (UINT i = 0; i < numVertices[2]; ++i)
    {
        sphere_vertices[i].x *= scaleMod;
        sphere_vertices[i].y *= scaleMod;
        sphere_vertices[i].z *= scaleMod;
    }

    ID3D11Buffer* vertexBuffers[] = {
        renderer.CreateVertexBuffer(triangle_vertices, sizeof(triangle_vertices)),
        renderer.CreateVertexBuffer(cube_vertices, sizeof(cube_vertices)),
        renderer.CreateVertexBuffer(sphere_vertices, sizeof(sphere_vertices))
    };

    MainLoop(hWnd, renderer, vertexBuffers, numVertices, ARRAYSIZE(vertexBuffers));

    CleanupImGui();
    ReleaseResources(renderer, vertexBuffers, ARRAYSIZE(vertexBuffers));

    return 0;
}

