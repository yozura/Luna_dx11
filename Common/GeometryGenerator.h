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
    /// �־��� ������ �������� ������ �߽����� �ϴ� �ڽ��� �����մϴ�.
    /// </summary>
    void CreateBox(float width, float height, float depth, MeshData& meshData);

    /// <summary>
    /// �־��� �������� �������� ������ �߽����� �ϴ� ���� �����մϴ�.
    /// sliceCount, stackCount ������ �׼����̼� ������ �����մϴ�.
    /// </summary>
    void CreateSphere(float radius, UINT sliceCount, UINT stackCount, MeshData& meshData);

    /// <summary>
    /// �־��� �������� �������� ������ �߽����� �ϴ� �������Ǿ �����մϴ�.
    /// ���̴� �׼����̼� ������ �����մϴ�.
    /// </summary>
    void CreateGeosphere(float radius, UINT numSubdivisions, MeshData& meshData);

    /// <summary>
    /// y�� �����ϸ� ������ �߽����� �ϴ� ������� �����մϴ�.
    /// bottomRadius, topRadius ������ ������� ����� ������ų �� �ֽ��ϴ�.
    /// sliceCount, stackCount ������ �׼����̼� ������ �����մϴ�.
    /// </summary>
    void CreateCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData);

    /// <summary>
    /// ������ �ʺ�� ���̸� ���� ������ �߽����� m���� ��� n���� ���� xz ��鿡 m*n ���ڸ� ����ϴ�.
    /// </summary>
    void CreateGrid(float width, float depth, UINT m, UINT n, MeshData& meshData);
    
    /// <summary>
    /// NDC ��ǥ�� ȭ���� ���� ���带 ����ϴ�.
    /// ��ó�� ȿ���� �����մϴ�.
    /// </summary>
    void CreateFullscreenQuad(MeshData& meshData);

private:
    void Subdivide(MeshData& meshData);
    void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData);
    void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData);
};

#endif /* GEOMETRY_GENERATOR_H */