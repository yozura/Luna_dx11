#ifndef PARTICLE_APP_H
#define PARTICLE_APP_H

#include "D3DApp.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "MathHelper.h"
#include "Effects.h"
#include "Vertex.h"
#include "Camera.h"
#include "Sky.h"
#include "Terrain.h"
#include "RenderStates.h"

class ParticleApp : public D3DApp
{
public:
    ParticleApp(HINSTANCE hInstance);
    ~ParticleApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
    Sky* mSky;
    Terrain mTerrain;

    DirectionalLight mDirLights[3];

    Camera mCam;

    bool mWalkCamMode;

    POINT mLastMousePos;
};

#endif /* PARTICLE_APP_H */
