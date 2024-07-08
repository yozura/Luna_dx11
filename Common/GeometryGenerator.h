#ifndef GEOMETRY_GENERATOR_H
#define GEOMETRY_GENERATOR_H

#include "D3DUtil.h"

class GeometryGenerator
{
public:
    struct Vertex
    {
        Vertex() {}
        Vertex(const XMFLOAT3& p, const XMFLOAT3& n, const XMFLOAT3& t, const XMFLOAT2& uv)
            : Position(p), Normal(n), TangentU(t), TexC(uv) {}
        Vertex(
            float px, float py, float pz,
            float nx, float ny, float nz,
            float tx, float ty, float tz,
            float u, float v)
            : Position(px, py, pz), Normal(nx, ny, nz),
              TangentU(tx, ty, tz), TexC(u, v) {}
        
        XMFLOAT3 Position;
        XMFLOAT3 Normal;
        XMFLOAT3 TangentU;
        XMFLOAT2 TexC;
    };

    struct MeshData
    {
        std::vector<Vertex> Vertices;
        std::vector<UINT>   Indices;
    };

    /// <summary>
    /// 주어진 차수를 기준으로 원점을 중심으로 하는 박스를 생성합니다.
    /// </summary>
    void CreateBox(float width, float height, float depth, MeshData& meshData);

    /// <summary>
    /// 주어진 반지름을 기준으로 원점을 중심으로 하는 구를 생성합니다.
    /// sliceCount, stackCount 변수로 테셀레이션 정도를 제어합니다.
    /// </summary>
    void CreateSphere(float radius, UINT sliceCount, UINT stackCount, MeshData& meshData);

    /// <summary>
    /// 주어진 반지름을 기준으로 원점을 중심으로 하는 지오스피어를 생성합니다.
    /// 깊이는 테셀레이션 레벨을 제어합니다.
    /// </summary>
    void CreateGeosphere(float radius, UINT numSubdivisions, MeshData& meshData);

    /// <summary>
    /// y축 평행하며 원점을 중심으로 하는 원기둥을 생성합니다.
    /// bottomRadius, topRadius 변수로 원기둥의 모양을 변형시킬 수 있습니다.
    /// sliceCount, stackCount 변수는 테셀레이션 정도를 제어합니다.
    /// </summary>
    void CreateCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData);

    /// <summary>
    /// 지정된 너비와 깊이를 가진 원점을 중심으로 m개의 행과 n개의 열로 xz 평면에 m*n 격자를 만듭니다.
    /// </summary>
    void CreateGrid(float width, float depth, UINT m, UINT n, MeshData& meshData);
    
    /// <summary>
    /// NDC 좌표로 화면을 덮는 쿼드를 만듭니다.
    /// 후처리 효과에 유용합니다.
    /// </summary>
    void CreateFullscreenQuad(MeshData& meshData);

private:
    void Subdivide(MeshData& meshData);
    void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData);
    void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData);
};

#endif /* GEOMETRY_GENERATOR_H */