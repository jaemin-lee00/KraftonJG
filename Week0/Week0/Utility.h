// Utility.h
#pragma once

#include <windows.h>
#include <d3d11.h>

// �ʿ��� ���̺귯���� ��ũ�մϴ�.
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

// D3D ��뿡 �ʿ��� ������ϵ��� �����մϴ�.
#include <d3dcompiler.h>

// �������� ����ϴ� ����ü�� �������� �����մϴ�.
struct FVertexSimple
{
    float x, y, z;          // Position
    float r, g, b, a;       // Color
};

struct FVector3
{
    float x, y, z;
    FVector3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}
};

struct FConstantBuffer
{
    FVector3 Offset;
    float Pad; // 16����Ʈ ������ ���� �е�
};

enum ETypePrimitive
{
    EPT_Triangle,
    EPT_Cube,
    EPT_Sphere,
    EPT_Max
};
