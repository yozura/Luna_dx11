#ifndef TERRAIN_APP_H
#define TERRAIN_APP_H

#include "D3DApp.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "MathHelper.h"
#include "Effects.h"
#include "Vertex.h"
#include "Camera.h"
#include "Sky.h"
#include "Terrain.h"

class TerrainApp : public D3DApp
{
public:
    TerrainApp(HINSTANCE hInstance);
    ~TerrainApp();

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

#endif /* TERRAIN_APP_H */
