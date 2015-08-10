#pragma once

#include "../stdafx.h"

using namespace std;

class Scene;

class SceneManager
{
public:
	static void Initialize();
	static void Dispose();

	static void PreInput();
	static void PostInput();
	static void PreUpdate();
	static void Update();
	static void PostUpdate();
	static void PrePhysicsStep();
	static void PostPhysicsStep();
	static void PreRender();
	static void Render();
	static void PostRender();
	static void PreGUI();
	static void GUI();
	static void PostGUI();

	static void OnResolutionChanged(int neww, int newh, int oldw, int oldh);

	static const Scene* GetActiveScene() { return activeScene; }
	static void SetActiveScene(char* name);

private:
	static Scene* activeScene;
	static map<char*, Scene*> loadedScenes;
	static int scenesCount;
};