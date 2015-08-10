#pragma once

#include "../stdafx.h"

using namespace std;

class GameObject;

class Scene
{
	friend class SceneManager;

public:
	Scene();

	void AddSceneObject(GameObject* object);
	void RemoveSceneObject(GameObject* object);

protected:
	virtual void Initialize();
	virtual void Dispose();

	virtual void PreInput();
	virtual void PostInput();
	virtual void PreUpdate();
	virtual void Update();
	virtual void PostUpdate();
	virtual void PrePhysicsStep();
	virtual void PostPhysicsStep();
	virtual void PreRender();
	virtual void Render();
	virtual void PostRender();
	virtual void PreGUI();
	virtual void GUI();
	virtual void PostGUI();

	virtual void OnResolutionChanged(int neww, int newh, int oldw, int oldh);

	virtual void OnActivated();
	virtual void OnDeactivated();

private:
	vector<GameObject*> sceneObjects;
	int sceneObjectsCount;
};