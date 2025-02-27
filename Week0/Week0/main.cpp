#include <windows.h>

// 여기에 아래 코드를 추가 합니다.

// D3D 사용에 필요한 라이브러리들을 링크합니다.	
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

// D3D 사용에 필요한 헤더파일들을 포함합니다.
#include <d3d11.h>
#include <d3dcompiler.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// 1. Define the triangle vertices
struct FVertexSimple
{
	float x, y, z;		// Position	
	float r, g, b, a;		// Color
};

// Structure for a 3D vector
struct FVector3
{
	float x, y, z;
	FVector3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}

	FVector3 operator+(const FVector3& v) const
	{
		return FVector3(x + v.x, y + v.y, z + v.z);
	}

	FVector3 operator-(const FVector3& v) const
	{
		return FVector3(x - v.x, y - v.y, z - v.z);
	}

	FVector3 operator+=(const FVector3& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	FVector3 operator-= (const FVector3& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	FVector3 operator*(float f) const
	{
		return FVector3(x * f, y * f, z * f);
	}
	
	FVector3 operator/(float f) const
	{
		return FVector3(x / f, y / f, z / f);
	}

	static float dot(const FVector3& a, const FVector3& b)
	{
		return a.x * b.x + a.y * b.y;
	}

	float length() const
	{
		return sqrt(x * x + y * y);
	}

	// 2D에서 직교 벡터(회전 ±90°)
	// p1 = (-y, x), p2 = (y, -x)
	FVector3 perpCW() const { // Clockwise: (y, -x)
		return FVector3(y, -x);
	}
	FVector3 perpCCW() const { // CounterClockwise: (-y, x)
		return FVector3(-y, x);
	}

	// 정규화

	static FVector3 normalize(const FVector3& v)
	{
		float len = v.length();
		return len > 0 ? v / len : FVector3();
	}
};

// Sphere 배열을 추가합니다.
#include "Sphere.h"

// 삼각형을 하드 코딩
FVertexSimple triangle_vertices[] =
{
	{  0.0f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f }, // Top vertex (red)
	{  1.0f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f, 1.0f }, // Bottom-right vertex (green)
	{ -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f }  // Bottom-left vertex (blue)
};

FVertexSimple cube_vertices[] =
{
	// Front face (Z+)
	{ -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 1.0f }, // Bottom-left (red)
	{ -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 1.0f }, // Top-left (yellow)
	{  0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 1.0f }, // Bottom-right (green)
	{ -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 1.0f }, // Top-left (yellow)
	{  0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 1.0f }, // Top-right (blue)
	{  0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 1.0f }, // Bottom-right (green)

	// Back face (Z-)
	{ -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f, 1.0f }, // Bottom-left (cyan)
	{  0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f, 1.0f }, // Bottom-right (magenta)
	{ -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 1.0f }, // Top-left (blue)
	{ -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 1.0f }, // Top-left (blue)
	{  0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f, 1.0f }, // Bottom-right (magenta)
	{  0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f, 1.0f }, // Top-right (yellow)

	// Left face (X-)
	{ -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f, 1.0f }, // Bottom-left (purple)
	{ -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 1.0f }, // Top-left (blue)
	{ -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 1.0f }, // Bottom-right (green)
	{ -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 1.0f }, // Top-left (blue)
	{ -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 1.0f }, // Top-right (yellow)
	{ -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 1.0f }, // Bottom-right (green)

	// Right face (X+)
	{  0.5f, -0.5f, -0.5f,  1.0f, 0.5f, 0.0f, 1.0f }, // Bottom-left (orange)
	{  0.5f, -0.5f,  0.5f,  0.5f, 0.5f, 0.5f, 1.0f }, // Bottom-right (gray)
	{  0.5f,  0.5f, -0.5f,  0.5f, 0.0f, 0.5f, 1.0f }, // Top-left (purple)
	{  0.5f,  0.5f, -0.5f,  0.5f, 0.0f, 0.5f, 1.0f }, // Top-left (purple)
	{  0.5f, -0.5f,  0.5f,  0.5f, 0.5f, 0.5f, 1.0f }, // Bottom-right (gray)
	{  0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.5f, 1.0f }, // Top-right (dark blue)

	// Top face (Y+)
	{ -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.5f, 1.0f }, // Bottom-left (light green)
	{ -0.5f,  0.5f,  0.5f,  0.0f, 0.5f, 1.0f, 1.0f }, // Top-left (cyan)
	{  0.5f,  0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f }, // Bottom-right (white)
	{ -0.5f,  0.5f,  0.5f,  0.0f, 0.5f, 1.0f, 1.0f }, // Top-left (cyan)
	{  0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.0f, 1.0f }, // Top-right (brown)
	{  0.5f,  0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f }, // Bottom-right (white)

	// Bottom face (Y-)
	{ -0.5f, -0.5f, -0.5f,  0.5f, 0.5f, 0.0f, 1.0f }, // Bottom-left (brown)
	{ -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 1.0f }, // Top-left (red)
	{  0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.5f, 1.0f }, // Bottom-right (purple)
	{ -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 1.0f }, // Top-left (red)
	{  0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 1.0f }, // Top-right (green)
	{  0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.5f, 1.0f }, // Bottom-right (purple)
};

class URenderer
{
public:
	// Direct3D 11 장치(Device)와 장치 건텍스트(Device Context) 및 스왑 체인(Swap Chain)을 관리하기 위한 포인터들
	ID3D11Device* Device = nullptr; // GPU와 통신하기 위한 Direct3D 장치
	ID3D11DeviceContext* DeviceContext = nullptr; // GPU 명령 실행을 담당하는 컨텍스트
	IDXGISwapChain* SwapChain = nullptr; // 프레임 버퍼를 교체하는데 사용되는 스왑 체인

	// 렌더링에 필요한 리소스 및 상태를 관리하기 위한 변수들
	ID3D11Texture2D* FrameBuffer = nullptr; // 화면 출력용 텍스쳐
	ID3D11RenderTargetView* FrameBufferRTV = nullptr; // 텍스쳐를 렌더 타겟으로 사용하는 뷰
	ID3D11RasterizerState* RasterizerState = nullptr; // 래스터라이저 상태(컬링, 채우기 모드 등 정의)
	ID3D11Buffer* ConstantBuffer = nullptr; // 쉐이더에 데이터를 전할하기 위한 상수 버퍼

	FLOAT ClearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f }; // 화면을 초기화(clear)할 때 사용할 색상 (RGBA)
	D3D11_VIEWPORT ViewportInfo; // 렌더링 영역을 정희하는 뷰포트 정보

public:
	// 렌더러 초기화 함수
	void Create(HWND hWindow)
	{
		// Direct3D 장치 및 스왑 체인 생성
		CreateDeviceAndSwapChain(hWindow);

		// 프레임 버퍼 생성
		CreateFrameBuffer();

		// 레스터라이저 상태 생성
		CreateRasterizerState();

		// depth stencil buffer, blend state 는 이 코드에서는 다루지 않음
	}

	// Direct3D 장치 및 스왑 체인 생성 함수
	void CreateDeviceAndSwapChain(HWND hWindow)
	{
		// 지원하는 Direct3D 기능 레벨을 정의
		D3D_FEATURE_LEVEL featurelevels[] = { D3D_FEATURE_LEVEL_11_0 };

		// 스왑 체인 설정 구조체 초기화
		DXGI_SWAP_CHAIN_DESC swapchaindesc = {};
		swapchaindesc.BufferDesc.Width = 0; // 창 크기에 맞게 자동으로 설정
		swapchaindesc.BufferDesc.Height = 0; // 창 크기에 맞게 자동으로 설정
		swapchaindesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // 색상 포맷
		swapchaindesc.SampleDesc.Count = 1; // 멀티 샘플링 비활성화
		swapchaindesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 렌더 타겟으로 사용
		swapchaindesc.BufferCount = 2; // 더블 샘플링
		swapchaindesc.OutputWindow = hWindow; // 렌더링할 창 핸들
		swapchaindesc.Windowed = TRUE; // 창 모드
		swapchaindesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // 스왑 방식

		// Direct3D 장치와 스왑 체인을 생성
		D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
			D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG,
			featurelevels, ARRAYSIZE(featurelevels), D3D11_SDK_VERSION,
			&swapchaindesc, &SwapChain, &Device, nullptr, &DeviceContext);

		// 생성된 스왑 체인의 정보 가져오기
		SwapChain->GetDesc(&swapchaindesc);

		// 뷰포트 정보 설정
		ViewportInfo = { 0.0f, 0.0f, (float)swapchaindesc.BufferDesc.Width, (float)swapchaindesc.BufferDesc.Height, 0.0f, 1.0f };
	}

	// Direct3D 장치 및 스왑 체인을 해제하는 함수
	void ReleaseDeviceAndSwapChain()
	{
		if (DeviceContext)
		{
			DeviceContext->Flush(); // 남아있는 GPU 명령 실행
		}

		if (SwapChain)
		{
			SwapChain->Release();
			SwapChain = nullptr;
		}

		if (Device)
		{
			Device->Release();
			Device = nullptr;
		}

		if (DeviceContext)
		{
			DeviceContext->Release();
			DeviceContext = nullptr;
		}
	}

	// 프레임 버퍼를 생성하는 함수
	void CreateFrameBuffer()
	{
		// 스왑 체인으로부터 백 버퍼 텍스처 가져오기
		SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&FrameBuffer);

		// 렌더 타겟 뷰 생성
		D3D11_RENDER_TARGET_VIEW_DESC framebufferRTVdesc = {};
		framebufferRTVdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; // 색상 포맷
		framebufferRTVdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처

		Device->CreateRenderTargetView(FrameBuffer, &framebufferRTVdesc, &FrameBufferRTV);
	}

	// 프레임 버퍼를 해제하는 함수
	void ReleaseFrameBuffer()
	{
		if (FrameBuffer)
		{
			FrameBuffer->Release();
			FrameBuffer = nullptr;
		}

		if (FrameBufferRTV)
		{
			FrameBufferRTV->Release();
			FrameBufferRTV = nullptr;
		}
	}

	// 레스터라이저 상태를 생성하는 함수
	void CreateRasterizerState()
	{
		D3D11_RASTERIZER_DESC rasterizerdesc = {};
		rasterizerdesc.FillMode = D3D11_FILL_SOLID; // 채우기 모드
		rasterizerdesc.CullMode = D3D11_CULL_BACK; // 백 페이스 컬링

		Device->CreateRasterizerState(&rasterizerdesc, &RasterizerState);
	}

	// 레스터라이저 상태를 해제하는 함수
	void ReleaseRasterizerState()
	{
		if (RasterizerState)
		{
			RasterizerState->Release();
			RasterizerState = nullptr;
		}
	}

	// 렌더러에 사용된 모든 리소스를 해제하는 함수
	void Release()
	{
		RasterizerState->Release();

		// 렌더 타겟을 초기화
		DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

		ReleaseFrameBuffer();
		ReleaseDeviceAndSwapChain();
	}

	// 스왑 체인의 백 버퍼와 프론트 버퍼를 교체하여 화면에 출력
	void SwapBuffer()
	{
		SwapChain->Present(1, 0); // 1 : Vsync 활성화, 0 : Vsync 비활성화
	}

public :
	// 기존 Urnderer Class 에 추가된 함수들

	ID3D11VertexShader* SimpleVertexShader;
	ID3D11PixelShader* SimplePixelShader;
	ID3D11InputLayout* SimpleInputLayout;
	unsigned int Stride;

	void CreateShader()
	{
		ID3DBlob* vertexshaderCSO;
		ID3DBlob* pixelshaderCSO;

		D3DCompileFromFile(L"ShaderW0.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &vertexshaderCSO, nullptr);

		Device->CreateVertexShader(vertexshaderCSO->GetBufferPointer(), vertexshaderCSO->GetBufferSize(), nullptr, &SimpleVertexShader);

		D3DCompileFromFile(L"ShaderW0.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &pixelshaderCSO, nullptr);

		Device->CreatePixelShader(pixelshaderCSO->GetBufferPointer(), pixelshaderCSO->GetBufferSize(), nullptr, &SimplePixelShader);

		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		Device->CreateInputLayout(layout, ARRAYSIZE(layout), vertexshaderCSO->GetBufferPointer(), vertexshaderCSO->GetBufferSize(), &SimpleInputLayout);

		Stride = sizeof(FVertexSimple);

		vertexshaderCSO->Release();
		pixelshaderCSO->Release();
	}

	void ReleaseShader()
	{
		if (SimpleInputLayout)
		{
			SimpleInputLayout->Release();
			SimpleInputLayout = nullptr;
		}

		if (SimplePixelShader)
		{
			SimplePixelShader->Release();
			SimpleInputLayout = nullptr;
		}

		if (SimpleVertexShader)
		{
			SimpleVertexShader->Release();
			SimpleVertexShader = nullptr;
		}
	}


	// Urenderer Class에 아래 함수를 추가합니다.
	void Prepare()
	{
		DeviceContext->ClearRenderTargetView(FrameBufferRTV, ClearColor);

		DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		DeviceContext->RSSetViewports(1, &ViewportInfo);
		DeviceContext->RSSetState(RasterizerState);

		DeviceContext->OMSetRenderTargets(1, &FrameBufferRTV, nullptr);
		DeviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	}

	void PrepareShader()
	{
		DeviceContext->VSSetShader(SimpleVertexShader, nullptr, 0);
		DeviceContext->PSSetShader(SimplePixelShader, nullptr, 0);
		DeviceContext->IASetInputLayout(SimpleInputLayout);

		// 여기에 추가하세요.
		// 버텍스 쉐이더에 상수 버퍼를 설정합니다.
		if (ConstantBuffer)
		{
			DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
		}
	}

	void RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices)
	{
		UINT offset = 0;
		DeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &Stride, &offset);

		DeviceContext->Draw(numVertices, 0);
	}

	ID3D11Buffer* CreateVertexBuffer(FVertexSimple* vertices, UINT byteWidth)
	{
		// 2. Create the vertex buffer
		D3D11_BUFFER_DESC vertexbufferdesc = {};
		vertexbufferdesc.ByteWidth = byteWidth;
		vertexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		D3D11_SUBRESOURCE_DATA vertexbufferSRD = { vertices };
		ID3D11Buffer* vertexBuffer;
		Device->CreateBuffer(&vertexbufferdesc, &vertexbufferSRD, &vertexBuffer);
		return vertexBuffer;
	}

	void ReleaseVertexBuffer(ID3D11Buffer* vertexBuffer)
	{
		if (vertexBuffer)
		{
			vertexBuffer->Release();
		}
	}

	struct FConstantBuffer
	{
		FVector3 Offset;
		// 여기에 추가합니다.
		float Pad;
	};
	
	void CreateConstantBuffer()
	{
		D3D11_BUFFER_DESC constantbufferdesc = {};
		constantbufferdesc.ByteWidth = sizeof(FConstantBuffer) + 0xf & 0xfffffff0; // ensure constant buffer size is 16-byte
		constantbufferdesc.Usage = D3D11_USAGE_DYNAMIC; // will be updated from CPU every frame
		constantbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		constantbufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		Device->CreateBuffer(&constantbufferdesc, nullptr, &ConstantBuffer);
	}

	void ReleaseConstantBuffer()
	{
		if (ConstantBuffer)
		{
			ConstantBuffer->Release();
			ConstantBuffer = nullptr;
		}
	}

	void UpdateConstant(FVector3 Offset)
	{
		if (ConstantBuffer)
		{
			D3D11_MAPPED_SUBRESOURCE constantbufferMSR;

			DeviceContext->Map(ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR); // update constant buffer every frame
			FConstantBuffer* constants = (FConstantBuffer*)constantbufferMSR.pData;
			{
				constants->Offset = Offset;
			}
			DeviceContext->Unmap(ConstantBuffer, 0);
		}
	}
};

// UBall Class를 통해 Ball의 속성을 정의합니다.
const float SCALE_MOD = 0.1f;
const float GRAVITY = -0.0980665f;
const float ELASTICITY = 0.8f;

class UBall
{
public:
	// 클래스 이름과, 아래 두개의 변수 이름은 변경하지 않습니다.
	FVector3 Position;
	FVector3 Velocity;
	float Radius;
	float Mass;

	// 이후 추가할 변수와 변수 이름은 자유롭게 정하세요.

public:
	//InitializeRandomValues(), SetMassProportionalToSize() 함수를 통해
	//초기 설정 및 초기 설정에 따른 Mass 값을 설정합니다.
	
	UBall()
	{
		InitializeRandomValues();
		SetMassProportionalToSize();
	}

private :

	void InitializeRandomValues() 
	{
		// 여기에 추가하세요.
		// Position, Velocity, Radius를 랜덤하게 설정합니다.
		// 사이드 물체가 발생하는 케이스를 방지하기 위해 랜덤값을 조정합니다.
		Position.x = ((float)(rand() % 50  + 25)) / 100.0f;
		Position.y = ((float)(rand() % 50  + 25)) / 100.0f;
		Position.z = 0.0f;
		Velocity.x = ((float)(rand() % 100 - 50)) / 500.0f;
		Velocity.y = ((float)(rand() % 100 - 50)) / 500.0f;
		Velocity.z = 0.0f;
		Radius = ((float)(rand() % 100)) / 100.0f * 1.5f + 0.5f;

	}

	void SetMassProportionalToSize()
	{
		// 질량을 반지름의 세제곱에 비례하도록 설정
		// 밀도를 1로 가정하면, 질량 = (4/3) * π * r^3
		Mass = 4.0f / 3.0f * 3.141592f * Radius * Radius * Radius;
	}

public:
	UINT numVerticesUBall = sizeof(sphere_vertices) / sizeof(FVertexSimple);
	ID3D11Buffer* vertexBufferUBall = nullptr;
	FVertexSimple uball_vertices[sizeof(sphere_vertices) / sizeof(FVertexSimple)];

	// 예:1
	float Index;

	// 예:2
	int NumHits;

	// 예:3
	void Render(URenderer* renderer)
	{
		if (!vertexBufferUBall)
		{
			for (UINT i = 0; i < numVerticesUBall; ++i)
			{
				uball_vertices[i].x = sphere_vertices[i].x * Radius;
				uball_vertices[i].y = sphere_vertices[i].y * Radius;
				uball_vertices[i].z = sphere_vertices[i].z * Radius;

				uball_vertices[i].r = sphere_vertices[i].r;
				uball_vertices[i].g = sphere_vertices[i].g;
				uball_vertices[i].b = sphere_vertices[i].b;
				uball_vertices[i].a = sphere_vertices[i].a;
			}
			vertexBufferUBall = renderer->CreateVertexBuffer(uball_vertices, sizeof(uball_vertices));
		}
	}

	// 예:4
	void DoRender(URenderer& renderer)
	{
		renderer.UpdateConstant(Position);
		renderer.RenderPrimitive(vertexBufferUBall, numVerticesUBall);
	}

	void ReleaseRender(URenderer& renderer) {
		if (vertexBufferUBall)
		{
			renderer.ReleaseVertexBuffer(vertexBufferUBall);
		}
	}
	// 예:5
	void Move()
	{
		Position.x += Velocity.x;
		Position.y += Velocity.y;
	}

	void BorderCollision()
	{
		if (Position.x + (Radius * SCALE_MOD) > 1.0f || Position.x - (Radius * SCALE_MOD) < -1.0f)
		{
			Velocity.x = -Velocity.x;
		}
		if (Position.y + (Radius * SCALE_MOD) > 1.0f)
		{
			Velocity.y = -Velocity.y;
		}
	}

	void FloorCollision(bool bGravity)
	{

		//// 바닥 충돌 탄성 처리 (예: y=0을 바닥으로 가정)
		if (Position.y - (Radius * SCALE_MOD) < -1.0f) {
			if (bGravity == true)
			{
				Position.y = -1.0f + Radius * SCALE_MOD;
				Velocity.y = -Velocity.y * ELASTICITY;
			}
			else if (bGravity == false)
			{
				Velocity.y = -Velocity.y;
			}
		}
	}


	static bool CollisionCheck(const UBall& a, const UBall& b) {
		float distance = FVector3::dot(a.Position - b.Position, a.Position - b.Position);
		float radiusSum = ((a.Radius + b.Radius) * (a.Radius + b.Radius) );
		radiusSum *= (SCALE_MOD * SCALE_MOD);
		return distance < radiusSum;
	}

	static void ResolveCollision(UBall& a, UBall& b) 
	{
		FVector3 positionDiff = a.Position - b.Position;
		FVector3 velocityDiff = a.Velocity - b.Velocity;

		float dotProduct = FVector3::dot(velocityDiff, positionDiff);
		float distanceSquared = positionDiff.x * positionDiff.x + positionDiff.y * positionDiff.y;

		float massCoefficient = (2 * b.Mass) / (a.Mass + b.Mass);

		
		FVector3 velocityChange = positionDiff * (massCoefficient * dotProduct / distanceSquared);

		a.Velocity = a.Velocity - velocityChange;
		b.Velocity = b.Velocity + velocityChange * (a.Mass / b.Mass);

		// 겹침 방지
		float distance = positionDiff.length();
		float overlap = (a.Radius + b.Radius) * SCALE_MOD - distance;
		if (overlap > 0)
		{
			FVector3 separationVector = FVector3::normalize(positionDiff) * (overlap / 2.0f);
			a.Position += separationVector;
			b.Position -= separationVector;
		}
	}

	// 중력 처리를 합니다. delta time 을 이용한 중력 처리를 일반적으로 많이 합니다.
	// 다만 이번 실습에서는 추가적인 chrono 헤더를 쓰지 못하기 떄문에 간단하게만 구현합니다.
	// 예시 코드 : Position.y -= Velocity.y * deltaTime +  GRAVITY * deltaTime * deltaTime;
	void UpdateGravity()
	{
		Velocity.y += GRAVITY*SCALE_MOD;
	}

	// 예:6
	void Update()
	{
		BorderCollision();
		Move();
	}
};

struct TriangleFlags
{
	bool Edgeflag[3];
};

struct CollsiionInfo
{
	CollsiionInfo(bool tempCollide, int temptCount) : bCollided(tempCollide), tCount(temptCount) {}

	bool bCollided;
	int tCount;
};
// UTriangle Class를 통해 Triangle의 속성을 정의합니다.
class UTriangle {
public:
	// 삼각형의 꼭짓점 (배열 순서는 시계 혹은 반시계 순서를 가정)
	FVector3 vertices[3];
	FVector3 centroid; // 삼각형의 기하학적 중심

	UTriangle() {
		vertices[0].x = triangle_vertices[0].x * SCALE_MOD;
		vertices[0].y = triangle_vertices[0].y * SCALE_MOD;
		vertices[0].z = triangle_vertices[0].z * SCALE_MOD;


		vertices[1].x = triangle_vertices[1].x * SCALE_MOD;
		vertices[1].y = triangle_vertices[1].y * SCALE_MOD;
		vertices[1].z = triangle_vertices[1].z * SCALE_MOD;


		vertices[2].x = triangle_vertices[2].x * SCALE_MOD;
		vertices[2].y = triangle_vertices[2].y * SCALE_MOD;
		vertices[2].z = triangle_vertices[2].z * SCALE_MOD;

		centroid = (vertices[0] + vertices[1] + vertices[2]) / 3.0f;
	}

public :

	UINT numVerticesUTriangle = sizeof(triangle_vertices) / sizeof(FVertexSimple);
	ID3D11Buffer* vertexBufferUTriangle = nullptr;
	FVertexSimple utriangle_vertices[sizeof(triangle_vertices) / sizeof(FVertexSimple)];

	float temp_r = 0;
	float temp_g = 0;
	float temp_b = 0;

	void Render(URenderer* renderer)
	{

		//if (!vertexBufferUTriangle)
		{
			for (UINT i = 0; i < numVerticesUTriangle; ++i)
			{
				utriangle_vertices[i].x = triangle_vertices[i].x * SCALE_MOD;
				utriangle_vertices[i].y = triangle_vertices[i].y * SCALE_MOD;
				utriangle_vertices[i].z = triangle_vertices[i].z * SCALE_MOD;

				utriangle_vertices[i].r = triangle_vertices[i].r * temp_r;
				utriangle_vertices[i].g = triangle_vertices[i].g * temp_g;
				utriangle_vertices[i].b = triangle_vertices[i].b * temp_b;
				utriangle_vertices[i].a = triangle_vertices[i].a;
			}
			vertexBufferUTriangle = renderer->CreateVertexBuffer(utriangle_vertices, sizeof(utriangle_vertices));
		}
	}

	// 예:4
	void DoRender(URenderer& renderer)
	{
		renderer.UpdateConstant(centroid);
		renderer.RenderPrimitive(vertexBufferUTriangle, numVerticesUTriangle);
	}

	void ReleaseRender(URenderer& renderer) {
		if (vertexBufferUTriangle)
		{
			renderer.ReleaseVertexBuffer(vertexBufferUTriangle);
		}
	}

	// 각 선분(에지)에 대해, 외부(삼각형 외부)를 가리키는 법선을 구하고,
	// 그 법선 방향에 대해 원의 중점와의 관계로 T/F 플래그를 결정.
	// T: 원 center가 에지의 외부 방향에 위치
	// F: 그렇지 않음.
	TriangleFlags computeEdgeFlags(const UBall& circle) const {
		TriangleFlags flags;
		
		for (int i = 0; i < 3; i++) {
			const FVector3& A = vertices[i];
			const FVector3& B = vertices[(i + 1) % 3];

			//선분의 중점 M
			FVector3 M = (A + B) * 0.5f;
			// 삼각형 내부(centroid)로 향하는 벡터
			FVector3 vIn = centroid - M;

			// 선분의 벡터
			FVector3 edge = B - A;
			// 두 후보 법선 벡터
			FVector3 n1 = FVector3::normalize(edge.perpCCW());
			FVector3 n2 = FVector3::normalize(edge.perpCW());

			// 선택: n_in should point toward interior if possible.
			// 즉, 선택한 n_in satisfies dot(n_in, vIn) > 0.
			FVector3 n_in = FVector3::dot(n1, vIn) > 0 ? n1 : n2;

			// 외부 법선: n_out = - n_in
			FVector3 n_out = n_in * -1;

			// 결정: 원 center와 선분 중점 M의 벡터
			FVector3 vEC = circle.Position - M;
			// 만일 vEC가 n_out의 방향(즉, 외부 쪽)과 같은 방향이면 dot > 0
			flags.Edgeflag[i] = (FVector3::dot(n_out, vEC) > 0);
		}
		return flags;
	}

	// 선분(엣지)와 점(p) 사이의 최소 거리 (선분 AB)
	static float distancePointToSegment(const FVector3& p, const FVector3& A, const FVector3& B) {
		FVector3 AB = B - A;
		float ab2 = FVector3::dot(AB, AB);
		if (ab2 == 0)
			return (p - A).length();
		float t = FVector3::dot(p - A, AB) / ab2;
		if (t < 0) t = 0;
		if (t > 1) t = 1;
		FVector3 projection = A + AB * t;
		return (p - projection).length();
	}

	// 충돌 판별 알고리즘 (귀하가 제시한 조건 기반)
	CollsiionInfo collidesWith(const UBall& circle) const {
		// 1. 각 선분에 대해 T/F 플래그 계산
		TriangleFlags flags = computeEdgeFlags(circle);

		// ①: 모든 선분 T (TTT) → 충돌
		if (flags.Edgeflag[0] && flags.Edgeflag[1] && flags.Edgeflag[2]) 
			return CollsiionInfo(true, -1);

		// ②: 삼각형의 무게중심(centroid)이 원의 내부에 있다면 충돌
		float dCentroid = (centroid - circle.Position).length();
		if (dCentroid < circle.Radius * SCALE_MOD)
			return CollsiionInfo(true, 0);

		// Count T 개수
		int tCount = 0;
		int tIndex = -1;
		for (int i = 0; i < 3; i++) {
			if (flags.Edgeflag[i]) { tCount++; tIndex = i; }
		}

		// ③: T가 하나인 경우 (TFF) → 그 T인 에지와 원 중심 사이 거리 검사
		if (tCount == 1) {
			// 해당 에지: vertices[tIndex] ~ vertices[(tIndex+1)%3]
			const FVector3& A = vertices[tIndex];
			const FVector3& B = vertices[(tIndex + 1) % 3];
			float dEdge = distancePointToSegment(circle.Position, A, B);
			if (dEdge < circle.Radius * SCALE_MOD)
				return CollsiionInfo(true, tCount);
		}

		// ④: T가 2개인 경우 (TTF)
		if (tCount == 2) {
			// 두 T 엣지가 공유하는 꼭짓점가 충돌점으로 취급됨.
			// 에지 index들이 T일 때, 그 두 선분이 공유하는 꼭짓점의 인덱스를 찾는다.
			int commonVertex = -1;
			// 두 엣지: i와 j (T인 인덱스)를 찾음
			int tEdges[2];
			int idx = 0;
			for (int i = 0; i < 3; i++) {
				if (flags.Edgeflag[i]) {
					tEdges[idx++] = i;
				}
			}
			// 에지 tEdges[0]는 (vertices[i], vertices[i+1])이고 tEdges[1] 역시 비슷하다.
			// 두 에지의 공유 vertex는 vertices[tEdges[0]+1]와 vertices[tEdges[1]] 또는 반대일 수 있음.
			// 간단하게, 3개의 정점을 모두 검사하여 두 해당 에지에 포함되는 vertex를 찾자.
			for (int v = 0; v < 3; v++) {
				bool inFirst = isVertexInEdge(v, tEdges[0]);
				bool inSecond = isVertexInEdge(v, tEdges[1]);
				if (inFirst && inSecond) {
					commonVertex = v;
					break;
				}
			}
			if (commonVertex != -1) {
				float dVertex = (vertices[commonVertex] - circle.Position).length();
				if (dVertex < circle.Radius * SCALE_MOD) {
					return CollsiionInfo(true, tCount);
				}
			}
		}
		return CollsiionInfo(false, -1);
	}

	void UpdateCollision(CollsiionInfo info)
	{
		info.bCollided = true;
		info.tCount = info.tCount;

		// collision 확인을 위한 일시적인 렌더링 색 변화
		if (info.bCollided == true)
		{
			switch (info.tCount)
			{
			case 0:
				temp_r = 0;
				temp_g = 255;
				temp_b = 0;
				break;
			case 1:
				temp_r = 0;
				temp_g = 0;
				temp_b = 255;
				break;
			case 2:
				temp_r = 255;
				temp_g = 0;
				temp_b = 0;
				break;
			default:
				temp_r = 0;
				temp_g = 0;
				temp_b = 0;
				break;
			};

			return;
		}

		else
		{
			temp_r = 0;
			temp_g = 0;
			temp_b = 0;
		}

	}

private:
	// 해당 vertex index v가 에지 index edgeIdx에 포함되는지 확인.
	// 에지 edgeIdx는 vertices[edgeIdx] ~ vertices[(edgeIdx+1)%3]
	bool isVertexInEdge(int v, int edgeIdx) const {
		int v0 = edgeIdx;
		int v1 = (edgeIdx + 1) % 3;
		return (v == v0 || v == v1);
	}
};

// UBallList Class를 통해 Ball의 리스트를 관리합니다.
class UBallList {
public:
	UBallList() : BallList(nullptr), capacity(0), size(0) {}
	~UBallList() {
		for (UINT i = 0; i < size; i++) {
			delete BallList[i];
		}
		delete[] BallList;
	}

	void addUBall() {
		if (size == capacity) {
			UINT newCapacity = (capacity == 0) ? 1 : capacity * 2;
			UBall** newList = new UBall * [newCapacity];
			for (UINT i = 0; i < size; i++) {
				newList[i] = BallList[i];
			}
			delete[] BallList;
			BallList = newList;
			capacity = newCapacity;
		}
		BallList[size++] = new UBall();
	}

	void deleteBall(UINT index) {
		if (index >= size) return;
		delete BallList[index];
		for (UINT i = index; i < size - 1; i++) {
			BallList[i] = BallList[i + 1];
		}
		size--;
	}

	UINT countBalls() {
		return size;
	}

	UBall* getBall(UINT index) {
		if (index >= size) return nullptr;
		return BallList[index];
	}

private:
	UBall** BallList;
	UINT capacity;
	UINT size;
};

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 각종 메세지를 처리할 함수
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
	{
		return true;
	}

	switch (message)
	{
	case WM_DESTROY:
		//Signal that the app sould quit
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
};

int	WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
          {
	// 윈도우 클래스 이름
	WCHAR WindowClass[] = L"JungleWindowClass";

	// 윈도우 타이틀바에 표시될 이름
	WCHAR Title[] = L"Game Tech Lab";

	// 각종 메세지를 처리할 함수인 WndProc의 함수 포인터를 WindowClass 구조체에 넣는다.
	WNDCLASSW wndclass = { 0,  WndProc, 0, 0, 0, 0, 0, 0, 0, WindowClass };

	// 윈도우 클래스 등록
	RegisterClassW(&wndclass);

	// 1024 x 1024 크기에 윈도우 생성
	HWND hWnd = CreateWindowExW(0, WindowClass, Title, WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1024, 1024,
		nullptr, nullptr, hInstance, nullptr);

	// Renderere Class를 생성합니다
	URenderer renderer;

	// D3D11 생성하는 함수를 호출합니다.
	renderer.Create(hWnd);

	// 렌더러 생성 직후에 쉐이더를 생성하는 함수를 호출합니다.
	renderer.CreateShader();
	// 여기에 생성 함수를 추가합니다.
	renderer.CreateConstantBuffer();


	// 여기에서 ImGui를 생성합니다.
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init((void*)hWnd);
	ImGui_ImplDX11_Init(renderer.Device, renderer.DeviceContext);

	// 실습2 마지막장에서 삼각형을 그릴 때 버텍스 버퍼 생성 하는 부분을 아래와 같이 수정합니다.
	// Vertex 배열 변수 이름이 triangle_vertices에서 cube_vertices로 변경되었습니다.

	UINT numVerticesTriangle = sizeof(triangle_vertices) / sizeof(FVertexSimple);
	UINT numVerticesCube = sizeof(cube_vertices) / sizeof(FVertexSimple);
	UINT numVerticesSphere = sizeof(sphere_vertices) / sizeof(FVertexSimple);

	UINT numVerticesUBall = sizeof(sphere_vertices) / sizeof(FVertexSimple);

	float scaleMod = 0.1f;

	for (UINT i = 0; i < numVerticesSphere; ++i)
	{
		sphere_vertices[i].x *= scaleMod;
		sphere_vertices[i].y *= scaleMod;
		sphere_vertices[i].z *= scaleMod;
	}

	ID3D11Buffer* vertexBufferTriangle = renderer.CreateVertexBuffer(triangle_vertices, sizeof(triangle_vertices));
	ID3D11Buffer* vertexBufferCube = renderer.CreateVertexBuffer(cube_vertices, sizeof(cube_vertices));
	ID3D11Buffer* vertexBufferSphere = renderer.CreateVertexBuffer(sphere_vertices, sizeof(sphere_vertices));

	ID3D11Buffer* vertexBufferUBall = renderer.CreateVertexBuffer(sphere_vertices, sizeof(sphere_vertices));
	
	bool bIsExit = false;

	enum ETypePrimitive
	{
		EPT_Triangle,
		EPT_Cube,
		EPT_Sphere,
		EPT_Max
	};

	//각종 생성하는 코드를 여기에 추가합니다.
	// 생성
	//ETypePrimitive typePrimitive = EPT_Triangle;
	ETypePrimitive typePrimitive = EPT_Sphere;

	// 도형의 움직임 정도를 담을 offset 변수를 Main 루프 바로 앞에 정의 하세요.
	FVector3 offset(0.0f);
	// 여기에 추가합니다.
	FVector3 velocity(0.0f);

	const float leftBorder = -1.0f;
	const float rightBorder = 1.0f;
	const float topBorder = -1.0f;
	const float bottomBorder = 1.0f;
	const float sphereRadius = 1.0f;

	bool bBoundBallToScreen = true;
	// 여기에 추가합니다.
	bool bPinballMovement = false;
	// 여기에 추가합니다.
	bool bGravity = true;

	bool bTriangleExist = false;


	const float ballSpeed = 0.001f;
	velocity.x = ((float)(rand() % 100 - 50)) * ballSpeed;
	velocity.y = ((float)(rand() % 100 - 50)) * ballSpeed;
	// 여기에 추가합니다.
	//int numBalls 변수를 통해 존재 가능한 Ball의 숫자를 조정합니다.
	int numBalls = 0;
	UBallList ballList;
	
	UTriangle* triangle = new UTriangle();

	// FPS 제한을 위한 설정
	const int targetFPS = 60;
	const double targetFrameTime = 1000.0 / targetFPS; // 한 프레임의 목표 시간 (밀리초 단위)


	// 고성능 타이머 초기화
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	LARGE_INTEGER startTime, endTime;
	double elapsedTime = 0.0;

	// Main Loop (Quit Message가 들어오기 전까지 아래 Loop를 무한히 실행하게 됨)
	// Quit Message가 들어오기 전까지 아래 Loop를 무한히 실행하게 됨
	while (bIsExit == false)
	{
		// 여기에 추가합니다.
		// 루프 시작 시간 기록
		QueryPerformanceCounter(&startTime);

		MSG msg;

		// 처리할 메세지가 더 이상 없을때 까지 수행
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			// 키 입력 메세지를 번역
			TranslateMessage(&msg);
			
			// 메세지를 적절한 윈도우 프로시저에 전달, 메세지가 위에서 등록한 WndProc 으로 전달
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
			{
				bIsExit = true;
				break;
			}
			else if (msg.message == WM_KEYDOWN) // 키보드 눌렀을 때
			{
				// 눌린 키가 방향키라면 해당 방향에 맞춰서
				// offset 변수의 x,y 멤버 변수의 값을 조정합니다.
				if (msg.wParam == VK_LEFT)
				{
					offset.x -= 0.01f;
				}
				else if (msg.wParam == VK_RIGHT)
				{
					offset.x += 0.01f;
				}
				else if (msg.wParam == VK_UP)
				{
					offset.y += 0.01f;
				}
				else if (msg.wParam == VK_DOWN)
				{
					offset.y -= 0.01f;
				}
				// 여기에 추가합니다.
				// 키보드 처리 직후에 하면 밖을 벗어났다면 화면 안쪽으로 위치시킨다.
				// 화면을 벗어나지 않아야 한다면
				if (bBoundBallToScreen)
				{
					float renderRadius = sphereRadius * scaleMod;
					if (offset.x < leftBorder + renderRadius)
					{
						offset.x = leftBorder + renderRadius;
					}
					if (offset.x > rightBorder - renderRadius)
					{
						offset.x = rightBorder - renderRadius;
					}
					if (offset.y < topBorder + renderRadius)
					{
						offset.y = topBorder + renderRadius;
					}
					if (offset.y > bottomBorder - renderRadius)
					{
						offset.y = bottomBorder - renderRadius;
					}
				}
			}
		}

		// 핀볼 움직임이 켜져 있다면
		//if (bPinballMovement)
		//{
		//	// 속도를 공위치에 더해 공을 실질적으로 움직임
		//	offset.x += velocity.x;
		//	offset.y += velocity.y;
		//	offset.z += velocity.z;

		//	// 벽과 충돌 여부를 체크하고 충돌시 속도에 음수를 곱해 방향을 바꿈.
		//	float renderRadius = sphereRadius * scaleMod;
		//	if (offset.x < leftBorder + renderRadius)
		//	{
		//		velocity.x *= -1.0f;
		//	}
		//	if (offset.x > rightBorder - renderRadius)
		//	{
		//		velocity.x *= -1.0f;
		//	}
		//	if (offset.y < topBorder + renderRadius)
		//	{
		//		velocity.y *= -1.0f;
		//	}
		//	if (offset.y > bottomBorder - renderRadius)
		//	{
		//		velocity.y *= -1.0f;
		//	}
		//
		//}

		if (ballList.countBalls() > 0)
		{
			for (UINT i = 0; i < ballList.countBalls(); i++)
			{
				UBall* ball = ballList.getBall(i);
				
				for (UINT j = 0; j < ballList.countBalls(); j++)
				{
					if (i == j) continue;
					UBall* otherBall = ballList.getBall(j);

					if (UBall::CollisionCheck(*ball, *otherBall)) 
					{
						UBall::ResolveCollision(*ball, *otherBall);
					}
				}

				if (bGravity)
				{
					ball->UpdateGravity();
				}
				ball->FloorCollision(bGravity);
				ball->Update();

				triangle->UpdateCollision(triangle->collidesWith(*ball));
			}
		}

		////////////////////////////////////////////////
		// 매번 실행되는 코드를 여기에 추가합니다.

		// 준비 작업
		renderer.Prepare();
		renderer.PrepareShader();

		// 아래 코드는 삭제합니다.
		// 생성한 버텍스 버퍼를 넘겨 실질적인 렌더링 요청
		//renderer.RenderPrimitive(vertexBuffer, numVertices);

		// offset을 상수 버퍼로 업데이트 합니다.

		//ConstantBuffer를 통한 이동 제어가 기본적인 함수이지만
		// 현재 프로그램에서 RenderPrimite의 추가적인 함수 설정 없이
		// Location + offset 의 형식으로 구성합니다.
		// UBall에 관한 데이터를 렌더링 요청합니다.

		if (ballList.countBalls() > 0)
		{
			for (UINT i = 0; i < ballList.countBalls(); i++)
			{
				UBall* ball = ballList.getBall(i);
				ball->Render(&renderer);
				ball->DoRender(renderer);
			}
		}

		if (bTriangleExist)
		{
			triangle->Render(&renderer);
			triangle->DoRender(renderer);
		}


		//renderer.UpdateConstant(offset);
		//renderer.RenderPrimitive(vertexBufferUBall, numVerticesUBall);


		//switch (typePrimitive)
		//{
		//case EPT_Triangle:
		//	renderer.RenderPrimitive(vertexBufferTriangle, numVerticesTriangle);
		//	break;
		//case EPT_Cube:
		//	renderer.RenderPrimitive(vertexBufferCube, numVerticesCube);
		//	break;
		//case EPT_Sphere:
		//	renderer.RenderPrimitive(vertexBufferSphere, numVerticesSphere);
		//	break;
		//}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// 이후 ImGui Ui 컨트롤 추가는 ImGui::NewFrame()과 ImGui::Render() 사이인 여기에 위치합니다.
		ImGui::Begin("Jungle Property Window");
		
		ImGui::Text("Hello, Jungle!");

		// Hello Jungle World 아래에 CheckBox와 bBoundBallToScreen 변수를 연결합니다.
		ImGui::Checkbox("Bound Ball to Screen", &bBoundBallToScreen);

		// Set Gravity
		ImGui::Checkbox("Gravity", &bGravity);
		ImGui::Checkbox("Triangle Exist", &bTriangleExist);

		//MainLoop 내에서 ImGui 의 변경 상태를 확인 후,
		//공의 증감을 파악합니다.
		//이후 생성,소멸 작업을 List를 통해 처리합니다.

		// InputInt 값을 통해 공의 갯수 증감을 설정합니다.
		if (ImGui::InputInt("Number of Balls", &numBalls))
		{
			if (numBalls < 0) numBalls = 0;

			UINT currentBalls = ballList.countBalls();
			for (UINT i = currentBalls; i < (UINT)numBalls; ++i)
			{
			    ballList.addUBall();
			}

			UINT ballsToRemove = currentBalls > numBalls ? currentBalls - numBalls : 0;
			for (UINT i = 0; i < ballsToRemove; ++i)
			{
				UINT randomIndex = rand() % ballList.countBalls();
				ballList.deleteBall(randomIndex);
			}
		}

		// 여기에 추가합니다.
		//ImGui::Checkbox("Pinball Movement", &bPinballMovement);
		
		// 아래 코드는 삭제합니다.
/*
		if (ImGui::Button("Quit this app")) 
		{
			// 현재 윈도우에 Quit 메세지를 메세지 큐로 보냄
			PostMessage(hWnd, WM_QUIT, 0, 0);
		}
*/


		// 아래 코드 Change Primitive 버튼 기능도 삭제 하세요.
/*
		if (ImGui::Button("Change primitive"))
		{
			switch (typePrimitive)
			{
			case EPT_Triangle:
				typePrimitive = EPT_Cube;
				break;
			case EPT_Cube:
				typePrimitive = EPT_Sphere;
				break;
			case EPT_Sphere:
				typePrimitive = EPT_Triangle;
				break;
			}
		}
*/

		ImGui::End();

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// 다 그렸으면 버퍼를 교환
		// 현재 화면에 보여지는 버퍼와 그리기 작업을 위한 버퍼를 서로 교환합니다.
		renderer.SwapBuffer();
		////////////////////////////////////////////////

		// 여기에 추가합니다.
		do
		{
			Sleep(0);

			// 루프 종료 시간 기록
			QueryPerformanceCounter(&endTime);

			// 한 프레임이 소요한 시간 계산 (밀리초 단위로 변환)
			elapsedTime = (endTime.QuadPart - startTime.QuadPart) * 1000.0 / frequency.QuadPart;

		} while (elapsedTime < targetFrameTime);
	}

	// 여기에서 ImGui를 해제합니다.
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// 버텍스 버퍼 소멸은 Renderer 소멸전에 처리합니다.
	//vertexBuffer->Release();
	
	if (ballList.countBalls() > 0)
	{
		for (UINT i = 0; i < ballList.countBalls(); i++)
		{
			UBall* ball = ballList.getBall(i);
			ball->ReleaseRender(renderer);
		}
	}

	triangle->ReleaseRender(renderer);

	renderer.ReleaseVertexBuffer(vertexBufferTriangle);
	renderer.ReleaseVertexBuffer(vertexBufferCube);
	renderer.ReleaseVertexBuffer(vertexBufferSphere);

	// ReleaseShader() 직전에 소멸 함수를 추가합니다.
	renderer.ReleaseConstantBuffer();

	// 렌더러 소멸 직전에 쉐이더를 소멸시키는 함수를 호출합니다.
	renderer.ReleaseShader();

	// 소멸하는 코드를 여기에 추가합니다.
	renderer.Release();
	

	return 0;
}