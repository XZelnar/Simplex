#pragma once

#include "../stdafx.h"

class Mesh;
class Texture;
class Material;
class SpherePopup;

class SimplexSphereGenerator
{
public:
	void Initialize();
	void Dispose();
	void PostInput();
	void Update();
	void Render();
	void GUI();
	void IncComplexity();
	void DecComplexity();
	inline int GetComplexity() { return sphereComplexity; }
	inline int GetGenerationTime() { return iterationTime; }
	inline int GetVertexCount() { return verticesCount; }
	inline void ToggleWireframeVisibility() { lines.isDesiredVisible = !lines.isDesiredVisible; }
	inline void ToggleTrianglesVisibility() { triangles.isDesiredVisible = !triangles.isDesiredVisible; }
	inline bool GetWireframeDesiredVisibility() { return lines.isDesiredVisible; }
	inline bool GetTrianglesDesiredVisibility() { return triangles.isDesiredVisible; }
	inline bool GetPopupVisibility() { return popupsVisible; }
	inline void SetPopupVisibility(bool v) { popupsVisible = v; }
	inline void OnResolutionChanged() { destroyPopups(); }

private:
	void destroyPopups();
	void spawnPopup();
	void generateStep();
	void subdivide(const D3DXVECTOR3 &v1, const D3DXVECTOR3 &v2, const D3DXVECTOR3 &v3, std::vector<D3DXVECTOR3> &sphere_points, const unsigned int depth);
	void initialize_sphere(std::vector<D3DXVECTOR3> &sphere_points, const unsigned int depth);

private://static
	static void genThreadFunc(SimplexSphereGenerator* ssg);

private:
	enum AnimationState
	{
		Appear,
		Disappear,
		Idle
	};
	struct AnimatedTerrain
	{
		Mesh* meshMain, *meshSwap;
		Material* material;
		float animatingState;
		AnimationState state;
		bool isDesiredVisible;
	};

private:
	AnimatedTerrain lines, triangles;
	std::vector<D3DXVECTOR3> spherePoints;
	D3DXVECTOR2* tarrtriangle;
	D3DXVECTOR3* parrtriangle;
	D3DXVECTOR3* narrtriangle;
	D3DXVECTOR4* carrtriangle;
	D3DXVECTOR2* tarrline;
	D3DXVECTOR3* parrline;
	D3DXVECTOR3* narrline;
	D3DXVECTOR4* carrline;
	D3DXMATRIX matWorld;

	std::mutex mutex;
	std::mutex popupsMutex;
	std::thread* genThread;
	bool finishedGenerating;
	bool generate;
	bool threadExit;
	float simplexw;
	int sphereComplexity;
	float rot;
	bool rotate;

	const float animationSpeed = 0.07f;
	int iterationTime;
	int verticesCount;
	int oldSphereComplexity;

	const int POPUPS_COUNT = 3;
	SpherePopup* popups[3];
	int timeSinceLastPopup;
	bool popupsVisible;
};