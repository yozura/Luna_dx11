#ifndef WAVES_H
#define WAVES_H

#include <Windows.h>
#include <DirectXMath.h>

class Waves
{
public:
    Waves();
    ~Waves();

    UINT RowCount() const;
    UINT ColumnCount() const;
    UINT VertexCount() const;
    UINT TriangleCount() const;
    float Width() const;
    float Depth() const;

    const DirectX::XMFLOAT3& operator[](int i) const { return mCurrSolution[i]; }

    const DirectX::XMFLOAT3& Normal(int i) const { return mNormals[i]; }

    const DirectX::XMFLOAT3& TangentX(int i) const { return mTangentX[i]; }

    void Init(UINT m, UINT n, float dx, float dt, float speed, float damping);
    void Update(float dt);
    void Disturb(UINT i, UINT j, float magnitude);

private:
    UINT mNumRows;
    UINT mNumCols;
    
    UINT mVertexCount;
    UINT mTriangleCount;

    float mK1;
    float mK2;
    float mK3;

    float mTimeStep;
    float mSpatialStep;

    DirectX::XMFLOAT3* mPrevSolution;
    DirectX::XMFLOAT3* mCurrSolution;
    DirectX::XMFLOAT3* mNormals;
    DirectX::XMFLOAT3* mTangentX;
};

#endif /* WAVES_H */

