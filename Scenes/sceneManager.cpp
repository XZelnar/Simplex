#include "sceneManager.h"

#include "scene.h"
#include "ssgScene.h"

Scene* SceneManager::activeScene = null;
map<char*, Scene*> SceneManager::loadedScenes;
int SceneManager::scenesCount = 0;

void SceneManager::Initialize()
{
	Scene* s;

	s = new SSGScene();
	s->Initialize();
	loadedScenes["default"] = s;
	scenesCount++;

	SetActiveScene("default");
}

void SceneManager::Dispose()
{
	for (auto i = loadedScenes.begin(); i != loadedScenes.end(); i++)
	{
		i->second->Dispose();
		delete i->second;
	}
	loadedScenes.clear();
	scenesCount = 0;
}



void SceneManager::PreInput()
{
	activeScene->PreInput();
}

void SceneManager::PostInput()
{
	activeScene->PostInput();
}

void SceneManager::PreUpdate()
{
	activeScene->PreUpdate();
}

void SceneManager::Update()
{
	activeScene->Update();
}

void SceneManager::PostUpdate()
{
	activeScene->PostUpdate();
}

void SceneManager::PrePhysicsStep()
{
	activeScene->PrePhysicsStep();
}

void SceneManager::PostPhysicsStep()
{
	activeScene->PostPhysicsStep();
}

void SceneManager::PreRender()
{
	activeScene->PreRender();
}

void SceneManager::Render()
{
	activeScene->Render();
}

void SceneManager::PostRender()
{
	activeScene->PostRender();
}

void SceneManager::PreGUI()
{
	activeScene->PreGUI();
}

void SceneManager::GUI()
{
	activeScene->GUI();
}

void SceneManager::PostGUI()
{
	activeScene->PostGUI();
}

void SceneManager::OnResolutionChanged(int neww, int newh, int oldw, int oldh)
{
	for (auto i = loadedScenes.begin(); i != loadedScenes.end(); i++)
		i->second->OnResolutionChanged(neww, newh, oldw, oldh);
}



void SceneManager::SetActiveScene(char* name)
{
	if (loadedScenes[name] == null)
		return;
	if (activeScene)
		activeScene->OnDeactivated();
	activeScene = loadedScenes[name];
	activeScene->OnActivated();
}