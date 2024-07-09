#include <algorithm>
#include <vector>

#include "Waves.h"

Waves::Waves()
    : mNumRows(0), mNumCols(0), mVertexCount(0), mTriangleCount(0),
    mK1(0.0f), mK2(0.0f), mK3(0.0f), mTimeStep(0.0f), mSpatialStep(0.0f),
    mPrevSolution(0), mCurrSolution(0)
{
}

Waves::~Waves()
{
    delete[] mPrevSolution;
    delete[] mCurrSolution;
}

UINT Waves::RowCount() const
{
    return mNumRows;
}

UINT Waves::ColumnCount() const
{
    return mNumCols;
}

UINT Waves::VertexCount() const
{
    return mVertexCount;
}

UINT Waves::TriangleCount() const
{
    return mTriangleCount;
}

void Waves::Init(UINT m, UINT n, float dx, float dt, float speed, float damping)
{
    mNumRows = m;
    mNumCols = n;

    mVertexCount = m * n;
    mTriangleCount = (m - 1) * (n - 1) * 2;

    mTimeStep = dt;
    mSpatialStep = dx;

    float d = damping * dt + 2.0f;
    float e = (speed * speed) * (dt * dt) / (dx * dx);
    mK1 = (damping * dt - 2.0f) / d;
    mK2 = (4.0f - 8.0f * e) / d;
    mK3 = (2.0f * e) / d;

    if (mPrevSolution)
    {
        delete[] mPrevSolution;
    }

    if (mCurrSolution)
    {
        delete[] mCurrSolution;
    }

    mPrevSolution = new XMFLOAT3[m * n];
    mCurrSolution = new XMFLOAT3[m * n];

    float halfWidth = (n - 1) * dx * 0.5f;
    float halfDepth = (m - 1) * dx * 0.5f;
    for (UINT i = 0; i < m; ++i)
    {
        float z = halfDepth - i * dx;
        for (UINT j = 0; j < n; ++j)
        {
            float x = -halfWidth + j * dx;

            mPrevSolution[i * n + j] = XMFLOAT3(x, 0.0f, z);
            mCurrSolution[i * n + j] = XMFLOAT3(x, 0.0f, z);
        }
    }
}

void Waves::Update(float dt)
{
    static float t = 0;

    // 시간 누적
    t += dt;

    // 지정된 시간 간격에 따라 실행됨.
    if (t >= mTimeStep)
    {
        for (DWORD i = 1; i < mNumRows - 1; ++i)
        {
            for (DWORD j = 1; j < mNumCols - 1; ++j)
            {
                mPrevSolution[i * mNumCols + j].y = mK1 * mPrevSolution[i * mNumCols + j].y
                                                  + mK2 * mCurrSolution[i * mNumCols + j].y
                                                  + mK3 * (mCurrSolution[(i + 1) * mNumCols + j].y 
                                                        +  mCurrSolution[(i - 1) * mNumCols + j].y
                                                        +  mCurrSolution[i * mNumCols + j + 1].y
                                                        +  mCurrSolution[i * mNumCols + j - 1].y);
            }
        }

        // 이전 솔루션에 현재 솔루션 덮어쓰기
        std::swap(mPrevSolution, mCurrSolution);

        // 시간 리셋
        t = 0.0f;
    }
}

void Waves::Disturb(UINT i, UINT j, float magnitude)
{
    assert(i > 1 && i < mNumRows - 2);
    assert(j > 1 && j < mNumCols - 2);

    float halfMag = 0.5f * magnitude;

    mCurrSolution[i * mNumCols + j].y       += magnitude;
    mCurrSolution[i * mNumCols + j + 1].y   += halfMag;
    mCurrSolution[i * mNumCols + j - 1].y   += halfMag;
    mCurrSolution[(i + 1) * mNumCols + j].y += halfMag;
    mCurrSolution[(i - 1) * mNumCols + j].y += halfMag;
}