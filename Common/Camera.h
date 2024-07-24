#ifndef CAMERA_H
#define CAMERA_H

#include "D3DUtil.h"

class Camera
{
public:
    Camera();
    ~Camera();

    // Get/Set World camera position.
    DirectX::XMVECTOR GetPosistionXM() const;
    DirectX::XMFLOAT3 GetPosition() const;
    void SetPosition(float x, float y, float z);
    void SetPosition(const DirectX::XMFLOAT3& v);

    // Get Camera basis vectors.
    DirectX::XMVECTOR GetRightXM() const;
    DirectX::XMFLOAT3 GetRight() const;
    DirectX::XMVECTOR GetUpXM() const;
    DirectX::XMFLOAT3 GetUp() const;
    DirectX::XMVECTOR GetLookXM() const;
    DirectX::XMFLOAT3 GetLook() const;

    // Get frustum properties.
    float GetNearZ() const;
    float GetFarZ() const;
    float GetAspect() const;
    float GetFovY() const;
    float GetFovX() const;

    // Get near and far plane dimensions in view space coordinates.
    float GetNearWindowWidth() const;
    float GetNearWindowHeight() const;
    float GetFarWindowWidth() const;
    float GetFarWindowHeight() const;

    // Set frustum.
    void SetLens(float fovY, float aspect, float nearZ, float farZ);

    // Define camera space via Look At param.
    void LookAt(DirectX::FXMVECTOR pos, DirectX::FXMVECTOR target, DirectX::FXMVECTOR worldUp);
    void LookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up);

    // Get View/Proj matrices.
    DirectX::XMMATRIX View() const;
    DirectX::XMMATRIX Proj() const;
    DirectX::XMMATRIX ViewProj() const;

    // Strafe/Walk the camera a distance d.
    void Strafe(float d);
    void Walk(float d);

    // Rotate the camera.
    void Pitch(float angle);
    void RotateY(float angle);

    // After modifying camera position/orientation, call to rebuild the view matrix once per frame.
    void UpdateViewMatrix();

private:
    // Camera coordinates system with coordinates relative to world space.
    DirectX::XMFLOAT3 mPosition; // View Space Origin.
    DirectX::XMFLOAT3 mRight;    // View Space x-axis.
    DirectX::XMFLOAT3 mUp;       // View Space y-axis.
    DirectX::XMFLOAT3 mLook;     // View Space z-axis.

    // Cache View/Proj matrices.
    DirectX::XMFLOAT4X4 mView;
    DirectX::XMFLOAT4X4 mProj;

    // Cache frustum properties.
    float mNearZ;
    float mFarZ;
    float mAspect;
    float mFovY;
    float mNearWindowHeight;
    float mFarWindowHeight;
};

#endif /* CAMERA_H */
