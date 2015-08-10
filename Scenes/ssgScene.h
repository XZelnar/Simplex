#pragma once

#include "../stdafx.h"
#include "scene.h"

class SimplexSphereGenerator;
class CameraProjection;
class CameraOrtho;
class BGNebula;
class BGStars;
class Material;

class SSGScene : public Scene
{
protected:
	virtual void Initialize();
	virtual void Dispose();
	virtual void PostInput();
	virtual void Update();
	virtual void Render();
	virtual void GUI();

	virtual void OnResolutionChanged(int neww, int newh, int oldw, int oldh);

private:
	SimplexSphereGenerator* ssg;
	CameraProjection* c;
	CameraOrtho* c2;
	BGNebula* bgn;
	BGStars* bgs;
	bool showInfo;
	Material* infoOverlay;
};