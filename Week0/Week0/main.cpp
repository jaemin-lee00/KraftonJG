#include <windows.h>

// ���⿡ �Ʒ� �ڵ带 �߰� �մϴ�.

// D3D ��뿡 �ʿ��� ���̺귯������ ��ũ�մϴ�.	
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

// D3D ��뿡 �ʿ��� ������ϵ��� �����մϴ�.
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

	// 2D���� ���� ����(ȸ�� ��90��)
	// p1 = (-y, x), p2 = (y, -x)
	FVector3 perpCW() const { // Clockwise: (y, -x)
		return FVector3(y, -x);
	}
	FVector3 perpCCW() const { // CounterClockwise: (-y, x)
		return FVector3(-y, x);
	}

	// ����ȭ

	static FVector3 normalize(const FVector3& v)
	{
		float len = v.length();
		return len > 0 ? v / len : FVector3();
	}
};

// Sphere �迭�� �߰��մϴ�.
#include "Sphere.h"

// �ﰢ���� �ϵ� �ڵ�
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
	// Direct3D 11 ��ġ(Device)�� ��ġ ���ؽ�Ʈ(Device Context) �� ���� ü��(Swap Chain)�� �����ϱ� ���� �����͵�
	ID3D11Device* Device = nullptr; // GPU�� ����ϱ� ���� Direct3D ��ġ
	ID3D11DeviceContext* DeviceContext = nullptr; // GPU ��� ������ ����ϴ� ���ؽ�Ʈ
	IDXGISwapChain* SwapChain = nullptr; // ������ ���۸� ��ü�ϴµ� ���Ǵ� ���� ü��

	// �������� �ʿ��� ���ҽ� �� ���¸� �����ϱ� ���� ������
	ID3D11Texture2D* FrameBuffer = nullptr; // ȭ�� ��¿� �ؽ���
	ID3D11RenderTargetView* FrameBufferRTV = nullptr; // �ؽ��ĸ� ���� Ÿ������ ����ϴ� ��
	ID3D11RasterizerState* RasterizerState = nullptr; // �����Ͷ����� ����(�ø�, ä��� ��� �� ����)
	ID3D11Buffer* ConstantBuffer = nullptr; // ���̴��� �����͸� �����ϱ� ���� ��� ����

	FLOAT ClearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f }; // ȭ���� �ʱ�ȭ(clear)�� �� ����� ���� (RGBA)
	D3D11_VIEWPORT ViewportInfo; // ������ ������ �����ϴ� ����Ʈ ����

public:
	// ������ �ʱ�ȭ �Լ�
	void Create(HWND hWindow)
	{
		// Direct3D ��ġ �� ���� ü�� ����
		CreateDeviceAndSwapChain(hWindow);

		// ������ ���� ����
		CreateFrameBuffer();

		// �����Ͷ����� ���� ����
		CreateRasterizerState();

		// depth stencil buffer, blend state �� �� �ڵ忡���� �ٷ��� ����
	}

	// Direct3D ��ġ �� ���� ü�� ���� �Լ�
	void CreateDeviceAndSwapChain(HWND hWindow)
	{
		// �����ϴ� Direct3D ��� ������ ����
		D3D_FEATURE_LEVEL featurelevels[] = { D3D_FEATURE_LEVEL_11_0 };

		// ���� ü�� ���� ����ü �ʱ�ȭ
		DXGI_SWAP_CHAIN_DESC swapchaindesc = {};
		swapchaindesc.BufferDesc.Width = 0; // â ũ�⿡ �°� �ڵ����� ����
		swapchaindesc.BufferDesc.Height = 0; // â ũ�⿡ �°� �ڵ����� ����
		swapchaindesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // ���� ����
		swapchaindesc.SampleDesc.Count = 1; // ��Ƽ ���ø� ��Ȱ��ȭ
		swapchaindesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // ���� Ÿ������ ���
		swapchaindesc.BufferCount = 2; // ���� ���ø�
		swapchaindesc.OutputWindow = hWindow; // �������� â �ڵ�
		swapchaindesc.Windowed = TRUE; // â ���
		swapchaindesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // ���� ���

		// Direct3D ��ġ�� ���� ü���� ����
		D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
			D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG,
			featurelevels, ARRAYSIZE(featurelevels), D3D11_SDK_VERSION,
			&swapchaindesc, &SwapChain, &Device, nullptr, &DeviceContext);

		// ������ ���� ü���� ���� ��������
		SwapChain->GetDesc(&swapchaindesc);

		// ����Ʈ ���� ����
		ViewportInfo = { 0.0f, 0.0f, (float)swapchaindesc.BufferDesc.Width, (float)swapchaindesc.BufferDesc.Height, 0.0f, 1.0f };
	}

	// Direct3D ��ġ �� ���� ü���� �����ϴ� �Լ�
	void ReleaseDeviceAndSwapChain()
	{
		if (DeviceContext)
		{
			DeviceContext->Flush(); // �����ִ� GPU ��� ����
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

	// ������ ���۸� �����ϴ� �Լ�
	void CreateFrameBuffer()
	{
		// ���� ü�����κ��� �� ���� �ؽ�ó ��������
		SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&FrameBuffer);

		// ���� Ÿ�� �� ����
		D3D11_RENDER_TARGET_VIEW_DESC framebufferRTVdesc = {};
		framebufferRTVdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; // ���� ����
		framebufferRTVdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D �ؽ�ó

		Device->CreateRenderTargetView(FrameBuffer, &framebufferRTVdesc, &FrameBufferRTV);
	}

	// ������ ���۸� �����ϴ� �Լ�
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

	// �����Ͷ����� ���¸� �����ϴ� �Լ�
	void CreateRasterizerState()
	{
		D3D11_RASTERIZER_DESC rasterizerdesc = {};
		rasterizerdesc.FillMode = D3D11_FILL_SOLID; // ä��� ���
		rasterizerdesc.CullMode = D3D11_CULL_BACK; // �� ���̽� �ø�

		Device->CreateRasterizerState(&rasterizerdesc, &RasterizerState);
	}

	// �����Ͷ����� ���¸� �����ϴ� �Լ�
	void ReleaseRasterizerState()
	{
		if (RasterizerState)
		{
			RasterizerState->Release();
			RasterizerState = nullptr;
		}
	}

	// �������� ���� ��� ���ҽ��� �����ϴ� �Լ�
	void Release()
	{
		RasterizerState->Release();

		// ���� Ÿ���� �ʱ�ȭ
		DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

		ReleaseFrameBuffer();
		ReleaseDeviceAndSwapChain();
	}

	// ���� ü���� �� ���ۿ� ����Ʈ ���۸� ��ü�Ͽ� ȭ�鿡 ���
	void SwapBuffer()
	{
		SwapChain->Present(1, 0); // 1 : Vsync Ȱ��ȭ, 0 : Vsync ��Ȱ��ȭ
	}

public :
	// ���� Urnderer Class �� �߰��� �Լ���

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


	// Urenderer Class�� �Ʒ� �Լ��� �߰��մϴ�.
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

		// ���⿡ �߰��ϼ���.
		// ���ؽ� ���̴��� ��� ���۸� �����մϴ�.
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
		// ���⿡ �߰��մϴ�.
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

// UBall Class�� ���� Ball�� �Ӽ��� �����մϴ�.
const float SCALE_MOD = 0.1f;
const float GRAVITY = -0.0980665f;
const float ELASTICITY = 0.8f;

class UBall
{
public:
	// Ŭ���� �̸���, �Ʒ� �ΰ��� ���� �̸��� �������� �ʽ��ϴ�.
	FVector3 Position;
	FVector3 Velocity;
	float Radius;
	float Mass;

	// ���� �߰��� ������ ���� �̸��� �����Ӱ� ���ϼ���.

public:
	//InitializeRandomValues(), SetMassProportionalToSize() �Լ��� ����
	//�ʱ� ���� �� �ʱ� ������ ���� Mass ���� �����մϴ�.
	
	UBall()
	{
		InitializeRandomValues();
		SetMassProportionalToSize();
	}

private :

	void InitializeRandomValues() 
	{
		// ���⿡ �߰��ϼ���.
		// Position, Velocity, Radius�� �����ϰ� �����մϴ�.
		// ���̵� ��ü�� �߻��ϴ� ���̽��� �����ϱ� ���� �������� �����մϴ�.
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
		// ������ �������� �������� ����ϵ��� ����
		// �е��� 1�� �����ϸ�, ���� = (4/3) * �� * r^3
		Mass = 4.0f / 3.0f * 3.141592f * Radius * Radius * Radius;
	}

public:
	UINT numVerticesUBall = sizeof(sphere_vertices) / sizeof(FVertexSimple);
	ID3D11Buffer* vertexBufferUBall = nullptr;
	FVertexSimple uball_vertices[sizeof(sphere_vertices) / sizeof(FVertexSimple)];

	// ��:1
	float Index;

	// ��:2
	int NumHits;

	// ��:3
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

	// ��:4
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
	// ��:5
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

		//// �ٴ� �浹 ź�� ó�� (��: y=0�� �ٴ����� ����)
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

		// ��ħ ����
		float distance = positionDiff.length();
		float overlap = (a.Radius + b.Radius) * SCALE_MOD - distance;
		if (overlap > 0)
		{
			FVector3 separationVector = FVector3::normalize(positionDiff) * (overlap / 2.0f);
			a.Position += separationVector;
			b.Position -= separationVector;
		}
	}

	// �߷� ó���� �մϴ�. delta time �� �̿��� �߷� ó���� �Ϲ������� ���� �մϴ�.
	// �ٸ� �̹� �ǽ������� �߰����� chrono ����� ���� ���ϱ� ������ �����ϰԸ� �����մϴ�.
	// ���� �ڵ� : Position.y -= Velocity.y * deltaTime +  GRAVITY * deltaTime * deltaTime;
	void UpdateGravity()
	{
		Velocity.y += GRAVITY*SCALE_MOD;
	}

	// ��:6
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
// UTriangle Class�� ���� Triangle�� �Ӽ��� �����մϴ�.
class UTriangle {
public:
	// �ﰢ���� ������ (�迭 ������ �ð� Ȥ�� �ݽð� ������ ����)
	FVector3 vertices[3];
	FVector3 centroid; // �ﰢ���� �������� �߽�

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

	// ��:4
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

	// �� ����(����)�� ����, �ܺ�(�ﰢ�� �ܺ�)�� ����Ű�� ������ ���ϰ�,
	// �� ���� ���⿡ ���� ���� �������� ����� T/F �÷��׸� ����.
	// T: �� center�� ������ �ܺ� ���⿡ ��ġ
	// F: �׷��� ����.
	TriangleFlags computeEdgeFlags(const UBall& circle) const {
		TriangleFlags flags;
		
		for (int i = 0; i < 3; i++) {
			const FVector3& A = vertices[i];
			const FVector3& B = vertices[(i + 1) % 3];

			//������ ���� M
			FVector3 M = (A + B) * 0.5f;
			// �ﰢ�� ����(centroid)�� ���ϴ� ����
			FVector3 vIn = centroid - M;

			// ������ ����
			FVector3 edge = B - A;
			// �� �ĺ� ���� ����
			FVector3 n1 = FVector3::normalize(edge.perpCCW());
			FVector3 n2 = FVector3::normalize(edge.perpCW());

			// ����: n_in should point toward interior if possible.
			// ��, ������ n_in satisfies dot(n_in, vIn) > 0.
			FVector3 n_in = FVector3::dot(n1, vIn) > 0 ? n1 : n2;

			// �ܺ� ����: n_out = - n_in
			FVector3 n_out = n_in * -1;

			// ����: �� center�� ���� ���� M�� ����
			FVector3 vEC = circle.Position - M;
			// ���� vEC�� n_out�� ����(��, �ܺ� ��)�� ���� �����̸� dot > 0
			flags.Edgeflag[i] = (FVector3::dot(n_out, vEC) > 0);
		}
		return flags;
	}

	// ����(����)�� ��(p) ������ �ּ� �Ÿ� (���� AB)
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

	// �浹 �Ǻ� �˰��� (���ϰ� ������ ���� ���)
	CollsiionInfo collidesWith(const UBall& circle) const {
		// 1. �� ���п� ���� T/F �÷��� ���
		TriangleFlags flags = computeEdgeFlags(circle);

		// ��: ��� ���� T (TTT) �� �浹
		if (flags.Edgeflag[0] && flags.Edgeflag[1] && flags.Edgeflag[2]) 
			return CollsiionInfo(true, -1);

		// ��: �ﰢ���� �����߽�(centroid)�� ���� ���ο� �ִٸ� �浹
		float dCentroid = (centroid - circle.Position).length();
		if (dCentroid < circle.Radius * SCALE_MOD)
			return CollsiionInfo(true, 0);

		// Count T ����
		int tCount = 0;
		int tIndex = -1;
		for (int i = 0; i < 3; i++) {
			if (flags.Edgeflag[i]) { tCount++; tIndex = i; }
		}

		// ��: T�� �ϳ��� ��� (TFF) �� �� T�� ������ �� �߽� ���� �Ÿ� �˻�
		if (tCount == 1) {
			// �ش� ����: vertices[tIndex] ~ vertices[(tIndex+1)%3]
			const FVector3& A = vertices[tIndex];
			const FVector3& B = vertices[(tIndex + 1) % 3];
			float dEdge = distancePointToSegment(circle.Position, A, B);
			if (dEdge < circle.Radius * SCALE_MOD)
				return CollsiionInfo(true, tCount);
		}

		// ��: T�� 2���� ��� (TTF)
		if (tCount == 2) {
			// �� T ������ �����ϴ� �������� �浹������ ��޵�.
			// ���� index���� T�� ��, �� �� ������ �����ϴ� �������� �ε����� ã�´�.
			int commonVertex = -1;
			// �� ����: i�� j (T�� �ε���)�� ã��
			int tEdges[2];
			int idx = 0;
			for (int i = 0; i < 3; i++) {
				if (flags.Edgeflag[i]) {
					tEdges[idx++] = i;
				}
			}
			// ���� tEdges[0]�� (vertices[i], vertices[i+1])�̰� tEdges[1] ���� ����ϴ�.
			// �� ������ ���� vertex�� vertices[tEdges[0]+1]�� vertices[tEdges[1]] �Ǵ� �ݴ��� �� ����.
			// �����ϰ�, 3���� ������ ��� �˻��Ͽ� �� �ش� ������ ���ԵǴ� vertex�� ã��.
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

		// collision Ȯ���� ���� �Ͻ����� ������ �� ��ȭ
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
	// �ش� vertex index v�� ���� index edgeIdx�� ���ԵǴ��� Ȯ��.
	// ���� edgeIdx�� vertices[edgeIdx] ~ vertices[(edgeIdx+1)%3]
	bool isVertexInEdge(int v, int edgeIdx) const {
		int v0 = edgeIdx;
		int v1 = (edgeIdx + 1) % 3;
		return (v == v0 || v == v1);
	}
};

// UBallList Class�� ���� Ball�� ����Ʈ�� �����մϴ�.
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

// ���� �޼����� ó���� �Լ�
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
	// ������ Ŭ���� �̸�
	WCHAR WindowClass[] = L"JungleWindowClass";

	// ������ Ÿ��Ʋ�ٿ� ǥ�õ� �̸�
	WCHAR Title[] = L"Game Tech Lab";

	// ���� �޼����� ó���� �Լ��� WndProc�� �Լ� �����͸� WindowClass ����ü�� �ִ´�.
	WNDCLASSW wndclass = { 0,  WndProc, 0, 0, 0, 0, 0, 0, 0, WindowClass };

	// ������ Ŭ���� ���
	RegisterClassW(&wndclass);

	// 1024 x 1024 ũ�⿡ ������ ����
	HWND hWnd = CreateWindowExW(0, WindowClass, Title, WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1024, 1024,
		nullptr, nullptr, hInstance, nullptr);

	// Renderere Class�� �����մϴ�
	URenderer renderer;

	// D3D11 �����ϴ� �Լ��� ȣ���մϴ�.
	renderer.Create(hWnd);

	// ������ ���� ���Ŀ� ���̴��� �����ϴ� �Լ��� ȣ���մϴ�.
	renderer.CreateShader();
	// ���⿡ ���� �Լ��� �߰��մϴ�.
	renderer.CreateConstantBuffer();


	// ���⿡�� ImGui�� �����մϴ�.
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init((void*)hWnd);
	ImGui_ImplDX11_Init(renderer.Device, renderer.DeviceContext);

	// �ǽ�2 �������忡�� �ﰢ���� �׸� �� ���ؽ� ���� ���� �ϴ� �κ��� �Ʒ��� ���� �����մϴ�.
	// Vertex �迭 ���� �̸��� triangle_vertices���� cube_vertices�� ����Ǿ����ϴ�.

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

	//���� �����ϴ� �ڵ带 ���⿡ �߰��մϴ�.
	// ����
	//ETypePrimitive typePrimitive = EPT_Triangle;
	ETypePrimitive typePrimitive = EPT_Sphere;

	// ������ ������ ������ ���� offset ������ Main ���� �ٷ� �տ� ���� �ϼ���.
	FVector3 offset(0.0f);
	// ���⿡ �߰��մϴ�.
	FVector3 velocity(0.0f);

	const float leftBorder = -1.0f;
	const float rightBorder = 1.0f;
	const float topBorder = -1.0f;
	const float bottomBorder = 1.0f;
	const float sphereRadius = 1.0f;

	bool bBoundBallToScreen = true;
	// ���⿡ �߰��մϴ�.
	bool bPinballMovement = false;
	// ���⿡ �߰��մϴ�.
	bool bGravity = true;

	bool bTriangleExist = false;


	const float ballSpeed = 0.001f;
	velocity.x = ((float)(rand() % 100 - 50)) * ballSpeed;
	velocity.y = ((float)(rand() % 100 - 50)) * ballSpeed;
	// ���⿡ �߰��մϴ�.
	//int numBalls ������ ���� ���� ������ Ball�� ���ڸ� �����մϴ�.
	int numBalls = 0;
	UBallList ballList;
	
	UTriangle* triangle = new UTriangle();

	// FPS ������ ���� ����
	const int targetFPS = 60;
	const double targetFrameTime = 1000.0 / targetFPS; // �� �������� ��ǥ �ð� (�и��� ����)


	// ���� Ÿ�̸� �ʱ�ȭ
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	LARGE_INTEGER startTime, endTime;
	double elapsedTime = 0.0;

	// Main Loop (Quit Message�� ������ ������ �Ʒ� Loop�� ������ �����ϰ� ��)
	// Quit Message�� ������ ������ �Ʒ� Loop�� ������ �����ϰ� ��
	while (bIsExit == false)
	{
		// ���⿡ �߰��մϴ�.
		// ���� ���� �ð� ���
		QueryPerformanceCounter(&startTime);

		MSG msg;

		// ó���� �޼����� �� �̻� ������ ���� ����
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			// Ű �Է� �޼����� ����
			TranslateMessage(&msg);
			
			// �޼����� ������ ������ ���ν����� ����, �޼����� ������ ����� WndProc ���� ����
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
			{
				bIsExit = true;
				break;
			}
			else if (msg.message == WM_KEYDOWN) // Ű���� ������ ��
			{
				// ���� Ű�� ����Ű��� �ش� ���⿡ ���缭
				// offset ������ x,y ��� ������ ���� �����մϴ�.
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
				// ���⿡ �߰��մϴ�.
				// Ű���� ó�� ���Ŀ� �ϸ� ���� ����ٸ� ȭ�� �������� ��ġ��Ų��.
				// ȭ���� ����� �ʾƾ� �Ѵٸ�
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

		// �ɺ� �������� ���� �ִٸ�
		//if (bPinballMovement)
		//{
		//	// �ӵ��� ����ġ�� ���� ���� ���������� ������
		//	offset.x += velocity.x;
		//	offset.y += velocity.y;
		//	offset.z += velocity.z;

		//	// ���� �浹 ���θ� üũ�ϰ� �浹�� �ӵ��� ������ ���� ������ �ٲ�.
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
		// �Ź� ����Ǵ� �ڵ带 ���⿡ �߰��մϴ�.

		// �غ� �۾�
		renderer.Prepare();
		renderer.PrepareShader();

		// �Ʒ� �ڵ�� �����մϴ�.
		// ������ ���ؽ� ���۸� �Ѱ� �������� ������ ��û
		//renderer.RenderPrimitive(vertexBuffer, numVertices);

		// offset�� ��� ���۷� ������Ʈ �մϴ�.

		//ConstantBuffer�� ���� �̵� ��� �⺻���� �Լ�������
		// ���� ���α׷����� RenderPrimite�� �߰����� �Լ� ���� ����
		// Location + offset �� �������� �����մϴ�.
		// UBall�� ���� �����͸� ������ ��û�մϴ�.

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

		// ���� ImGui Ui ��Ʈ�� �߰��� ImGui::NewFrame()�� ImGui::Render() ������ ���⿡ ��ġ�մϴ�.
		ImGui::Begin("Jungle Property Window");
		
		ImGui::Text("Hello, Jungle!");

		// Hello Jungle World �Ʒ��� CheckBox�� bBoundBallToScreen ������ �����մϴ�.
		ImGui::Checkbox("Bound Ball to Screen", &bBoundBallToScreen);

		// Set Gravity
		ImGui::Checkbox("Gravity", &bGravity);
		ImGui::Checkbox("Triangle Exist", &bTriangleExist);

		//MainLoop ������ ImGui �� ���� ���¸� Ȯ�� ��,
		//���� ������ �ľ��մϴ�.
		//���� ����,�Ҹ� �۾��� List�� ���� ó���մϴ�.

		// InputInt ���� ���� ���� ���� ������ �����մϴ�.
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

		// ���⿡ �߰��մϴ�.
		//ImGui::Checkbox("Pinball Movement", &bPinballMovement);
		
		// �Ʒ� �ڵ�� �����մϴ�.
/*
		if (ImGui::Button("Quit this app")) 
		{
			// ���� �����쿡 Quit �޼����� �޼��� ť�� ����
			PostMessage(hWnd, WM_QUIT, 0, 0);
		}
*/


		// �Ʒ� �ڵ� Change Primitive ��ư ��ɵ� ���� �ϼ���.
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

		// �� �׷����� ���۸� ��ȯ
		// ���� ȭ�鿡 �������� ���ۿ� �׸��� �۾��� ���� ���۸� ���� ��ȯ�մϴ�.
		renderer.SwapBuffer();
		////////////////////////////////////////////////

		// ���⿡ �߰��մϴ�.
		do
		{
			Sleep(0);

			// ���� ���� �ð� ���
			QueryPerformanceCounter(&endTime);

			// �� �������� �ҿ��� �ð� ��� (�и��� ������ ��ȯ)
			elapsedTime = (endTime.QuadPart - startTime.QuadPart) * 1000.0 / frequency.QuadPart;

		} while (elapsedTime < targetFrameTime);
	}

	// ���⿡�� ImGui�� �����մϴ�.
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// ���ؽ� ���� �Ҹ��� Renderer �Ҹ����� ó���մϴ�.
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

	// ReleaseShader() ������ �Ҹ� �Լ��� �߰��մϴ�.
	renderer.ReleaseConstantBuffer();

	// ������ �Ҹ� ������ ���̴��� �Ҹ��Ű�� �Լ��� ȣ���մϴ�.
	renderer.ReleaseShader();

	// �Ҹ��ϴ� �ڵ带 ���⿡ �߰��մϴ�.
	renderer.Release();
	

	return 0;
}