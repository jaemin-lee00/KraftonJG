// Utility.h
#pragma once

#include <windows.h>
#include <d3d11.h>

// 필요한 라이브러리를 링크합니다.
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

// D3D 사용에 필요한 헤더파일들을 포함합니다.
#include <d3dcompiler.h>

// 공통으로 사용하는 구조체와 열거형을 정의합니다.
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
    float Pad; // 16바이트 정렬을 위한 패딩
};

enum ETypePrimitive
{
    EPT_Triangle,
    EPT_Cube,
    EPT_Sphere,
    EPT_Max
};
