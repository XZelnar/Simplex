#pragma once

#include "../stdafx.h"

class Material;
class Mesh;

class SpherePopup
{
	friend class SimplexSphereGenerator;

public:
	void Initialize();
	void Dispose();
	void Update();
	void GUILines();
	void GUIWindow();
	void SetVars(D3DXVECTOR2 _windowPosition, D3DXVECTOR3 _worldPoint, int _sphereIndex);
	inline bool IsIn(int x, int y) { return (x >= windowPos.x - 32) && (y >= windowPos.y - 32) && (x <= windowPos2.x + 32) && (y <= windowPos2.y + 32); }

private:
	D3DXVECTOR2 windowPos, windowPos2;
	D3DXVECTOR3 worldPos;
	D3DXVECTOR2 windowWP;
	int sphereInd;
	D3DXMATRIX projection, view, world;
	bool isDead;
	D3D10_VIEWPORT viewport;
	float animationState;
	float animationMultiplier;

	Material* matLines, *matWindow;
	Mesh* meshWindow, *meshLines;
	float vLines[20], vWindow[8];

	std::string text, numbers;
	int visibleTextLength;
	int timeSinceUpdate;
};