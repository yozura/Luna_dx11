#ifndef MATH_HELPER_H
#define MATH_HELPER_H

#include <Windows.h>
#include <DirectXMath.h>

using namespace DirectX;

class MathHelper
{
public:
    // [0, 1) �� ���� ��ȯ.
    static float RandF()
    {
        return (float)(rand()) / (float)RAND_MAX;
    }

    // [a, b) �� ���� ��ȯ.
    static float RandF(float a, float b)
    {
        return a + RandF() * (b - a);
    }

    template<typename T>
    static T Min(const T& a, const T& b)
    {
        return a < b ? a : b;
    }

    template<typename T>
    static T Max(const T& a, const T& b)
    {
        return a > b ? a : b;
    }

    template<typename T>
    static T Lerp(const T& a, const T& b, float t)
    {
        return a + t * (b - a);
    }

    template<typename T>
    static T Clamp(const T& x, const T& low, const T& high)
    {
        return x < low ? low : (x > high ? high : x);
    }

    // [0, 2*PI) �� ���� ��(x, y)�� ����ǥ ������ ��ȯ.
    static float AngleFromXY(float x, float y);

    static XMMATRIX InverseTranspose(CXMMATRIX M)
    {
        // Inverse-transpose is just applied to normals.  So zero out 
        // translation row so that it doesn't get into our inverse-transpose
        // calculation--we don't want the inverse-transpose of the translation.
        XMMATRIX A = M;
        A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

        XMVECTOR det = XMMatrixDeterminant(A);
        return XMMatrixTranspose(XMMatrixInverse(&det, A));
    }

    static XMVECTOR RandUnitVec3();
    static XMVECTOR RandHemisphereUnitVec3(XMVECTOR n);

    static const float Infinity;
    static const float Pi;
};


#endif /* MATH_HELPER_H */