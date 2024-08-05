#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include "D3DUtil.h"

class Camera;
class ParticleEffect;

class ParticleSystem
{
public:
    ParticleSystem();
    ~ParticleSystem();
    
    // Time elapsed since the system was reset.
    float GetAge() const;
    
    void SetEyePos(const DirectX::XMFLOAT3& eyePosW);
    void SetEmitPos(const DirectX::XMFLOAT3& emitPosW);
    void SetEmitDir(const DirectX::XMFLOAT3& emitDirW);

    void Init(ID3D11Device* device,
              ParticleEffect* fx,
              ID3D11ShaderResourceView* texArraySRV,
              ID3D11ShaderResourceView* randomTexSRV,
              UINT maxParticles);
    void Reset();
    void Update(float dt, float gameTime);
    void Draw(ID3D11DeviceContext* dc, const Camera& cam);

private:
    void BuildVB(ID3D11Device* device);

    ParticleSystem(const ParticleSystem& rhs);
    ParticleSystem& operator=(const ParticleSystem& rhs);

private:
    UINT mMaxParticles;
    bool mFirstRun;

    float mGameTime;
    float mTimeStep;
    float mAge;

    DirectX::XMFLOAT3 mEyePosW;
    DirectX::XMFLOAT3 mEmitPosW;
    DirectX::XMFLOAT3 mEmitDir;

    ParticleEffect* mFX;

    ID3D11Buffer* mInitVB;
    ID3D11Buffer* mDrawVB;
    ID3D11Buffer* mStreamOutVB;

    ID3D11ShaderResourceView* mTexArraySRV;
    ID3D11ShaderResourceView* mRandomTexSRV;
};

#endif /* PARTICLE_SYSTEM_H */