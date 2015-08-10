#include "simplexSphereGenerator.h"

#include "../Graphics/texture.h"
#include "../Graphics/mesh.h"
#include "../Graphics/graphicsResourceManager.h"
#include "../Graphics/shaderManager.h"
#include "../Graphics/graphicsEngine.h"
#include "../Graphics/material.h"
#include "../window.h"
#include "../Input/inputManager.h"
#include "simplexnoise.h"
#include "spherePopup.h"

void SimplexSphereGenerator::Initialize()
{
	finishedGenerating = false;
	generate = true;
	threadExit = false;
	simplexw = 0;
	sphereComplexity = oldSphereComplexity = 5;
	iterationTime = 0;
	verticesCount = 0;
	timeSinceLastPopup = 0;
	popupsVisible = true;
	rot = 0;
	rotate = true;

	triangles.animatingState = 6;
	triangles.state = AnimationState::Appear;
	triangles.isDesiredVisible = true;
	lines.animatingState = 5;
	lines.state = AnimationState::Appear;
	lines.isDesiredVisible = true;

	triangles.meshMain = triangles.meshSwap = NULL;
	lines.meshMain = lines.meshSwap = NULL;

	triangles.material = new Material(GraphicsResourceManager::GetInstance().GetShader(L"Resources/Shaders/triangleAnimation.fx"));
	triangles.material->SetTexture(GraphicsResourceManager::GetInstance().GetTexture(L"Resources/gradient.png"));
	lines.material = new Material(GraphicsResourceManager::GetInstance().GetShader(L"Resources/Shaders/lineAnimation.fx"));
	lines.material->SetTexture(GraphicsResourceManager::GetInstance().GetTexture(L"Resources/gradientNeon.png"));

	tarrtriangle = null;
	parrtriangle = null;
	narrtriangle = null;
	carrtriangle = null;

	tarrline = null;
	parrline = null;
	narrline = null;
	carrline = null;

	D3DXMatrixIdentity(&matWorld);
	
	for (int i = 0; i < POPUPS_COUNT; i++)
		popups[i] = null;

	genThread = new std::thread(SimplexSphereGenerator::genThreadFunc, this);
}

void SimplexSphereGenerator::Dispose()
{
	threadExit = true;
	genThread->join();

	if (triangles.meshMain)
	{
		triangles.meshMain->Dispose();
		lines.meshMain->Dispose();

		delete triangles.meshMain;
		delete lines.meshMain;
	}

	delete triangles.material;
	delete lines.material;

	if (triangles.meshSwap)
	{
		triangles.meshSwap->Dispose();
		lines.meshSwap->Dispose();

		delete triangles.meshSwap;
		delete lines.meshSwap;
	}

	delete[] parrtriangle;
	delete[] narrtriangle;
	delete[] carrtriangle;
	delete[] tarrtriangle;

	delete[] parrline;
	delete[] narrline;
	delete[] carrline;
	delete[] tarrline;

	destroyPopups();
}

void SimplexSphereGenerator::PostInput()
{
	rotate = !InputManager::mIsButtonPressed(IM_BUTTON::Left);
	if (!rotate)
		rot += -InputManager::mGetDeltaX() / 300;
}

void SimplexSphereGenerator::Update()
{
	bool b = false;
	mutex.lock();
	if (finishedGenerating)
	{
		swap(triangles.meshMain, triangles.meshSwap);
		swap(lines.meshMain, lines.meshSwap);
		b = true;

		if ((timeSinceLastPopup += iterationTime) >= 3000 && (lines.isDesiredVisible || triangles.isDesiredVisible) && rotate)
		{
			spawnPopup();
			timeSinceLastPopup = 0;
		}

		popupsMutex.lock();
		for (int i = 0; i < POPUPS_COUNT; i++)
			if (popups[i] && popups[i]->sphereInd < spherePoints.size())
				popups[i]->worldPos = parrtriangle[popups[i]->sphereInd];
		popupsMutex.unlock();

		finishedGenerating = false;
	}
	mutex.unlock();

	if (triangles.meshMain == null)
		return;

	if (b)
	{
		triangles.meshMain->updateVBO();
		lines.meshMain->updateVBO();
	}

	if (lines.isDesiredVisible)
		lines.state = AnimationState::Appear;
	else
		lines.state = AnimationState::Disappear;

	if (lines.state == AnimationState::Appear)
	{
		lines.animatingState -= animationSpeed * 2;
		if (lines.animatingState < -14)
		{
			lines.animatingState = -14;
			lines.state = AnimationState::Idle;
		}
	}
	else if (lines.state == AnimationState::Disappear)
	{
		lines.animatingState += animationSpeed * 2;
		if (lines.animatingState > 6)
		{
			lines.animatingState = 6;
			lines.state = AnimationState::Idle;
		}
	}

	if (triangles.isDesiredVisible)
		triangles.state = AnimationState::Appear;
	else
		triangles.state = AnimationState::Disappear;

	if (triangles.state == AnimationState::Appear)
	{
		triangles.animatingState -= animationSpeed * 2;
		if (triangles.animatingState < -7.5f)
		{
			triangles.animatingState = -7.5f;
			triangles.state = AnimationState::Idle;
		}
	}
	else if (triangles.state == AnimationState::Disappear)
	{
		triangles.animatingState += animationSpeed * 2;
		if (triangles.animatingState > 2.5f)
		{
			triangles.animatingState = 2.5f;
			triangles.state = AnimationState::Idle;
		}
	}

	triangles.material->SetGeometryCBuffer(0, D3DXVECTOR4(triangles.animatingState, 3, 5, 0));
	lines.material->SetGeometryCBuffer(0, D3DXVECTOR4(lines.animatingState, 6, 5, 0));

	rot += 0.002f;
	if (rot > D3DX_PI * 2)
		rot -= D3DX_PI * 2;
	D3DXMatrixRotationY(&matWorld, rot);

	popupsMutex.lock();
	if (!lines.isDesiredVisible && !triangles.isDesiredVisible)
	{
		for (int i = 0; i < POPUPS_COUNT; i++)
			if (popups[i])
			{
				popups[i]->Dispose();
				delete popups[i];
				popups[i] = null;
			}
	}
	else
		for (int i = 0; i < POPUPS_COUNT; i++)
			if (popups[i])
			{
				popups[i]->world = matWorld;
				popups[i]->Update();
				if (popups[i]->isDead)
				{
					popups[i]->Dispose();
					delete popups[i];
					popups[i] = null;
				}
			}
	popupsMutex.unlock();
}

void SimplexSphereGenerator::IncComplexity()
{
	sphereComplexity++;
	if (sphereComplexity > 7)
	{
		sphereComplexity = 7;
		return;
	}
	destroyPopups();
}

void SimplexSphereGenerator::DecComplexity()
{
	sphereComplexity--;
	if (sphereComplexity < 4)
	{
		sphereComplexity = 4;
		return;
	}
	destroyPopups();
}

void SimplexSphereGenerator::destroyPopups()
{
	popupsMutex.lock();
	for (int i = 0; i < POPUPS_COUNT; i++)
		if (popups[i])
		{
		popups[i]->Dispose();
		delete popups[i];
		popups[i] = null;
		}
	popupsMutex.unlock();
}

void SimplexSphereGenerator::spawnPopup()
{
	srand(time(NULL));
	uniform_int_distribution<int> rand2(0, spherePoints.size() - 1);
	static default_random_engine e_rand;
	int iterCount = 1000;

	while (iterCount-- > 0)
	{
		int ind = rand2(e_rand);
		D3DXVECTOR3 p = spherePoints[ind];
		p.y = 0;
		D3DXVec3Normalize(&p, &p);
		D3DXVECTOR4 t;
		D3DXVec3Transform(&t, &p, &matWorld);

		if (t.z > .5f && t.x < -0.2f)
		{
			popupsMutex.lock();
			for (int i = 0; i < POPUPS_COUNT; i++)
				if (!popups[i])
				{
					int x, y;
					while (true)
					{
						windowGenStart:
						x = (int)Window::GetWidth() / 6;
						x = rand() % (x * 2) - x;
						if (abs(x) < 40)
							x = 40 * sign(x);
						if (x < 0)
							x += Window::GetWidth() - 192;
						y = rand() % ((int)Window::GetHeight() - 80 - 128) + 40;
						for (int i = 0; i < POPUPS_COUNT; i++)
							if (popups[i])
								if (popups[i]->IsIn(x, y) || popups[i]->IsIn(x + 192, y) || popups[i]->IsIn(x, y + 128) || popups[i]->IsIn(x + 192, y + 128))
									goto windowGenStart;
						break;
					}
					auto p = new SpherePopup();
					p->SetVars(D3DXVECTOR2(x, y), parrtriangle[ind], ind);
					p->Initialize();
					popups[i] = p;
					break;
				}
			popupsMutex.unlock();
			break;
		}
	}
}

void SimplexSphereGenerator::Render()
{
	if (triangles.meshMain == null)
		return;

	triangles.material->Activate();
	ShaderManager::SetMatrixWorld(&matWorld);
	triangles.meshMain->Render();

	lines.material->Activate();
	ShaderManager::SetMatrixWorld(&matWorld);
	lines.meshMain->Render();
}

void SimplexSphereGenerator::GUI()
{
	if (!popupsVisible)
		return;

	for (int i = 0; i < POPUPS_COUNT; i++)
		if (popups[i])
			popups[i]->GUILines();
	for (int i = 0; i < POPUPS_COUNT; i++)
		if (popups[i])
			popups[i]->GUIWindow();
}



void SimplexSphereGenerator::genThreadFunc(SimplexSphereGenerator* ssg)
{
	std::chrono::milliseconds t1, t2;
	while (!ssg->threadExit)
	{
		while (!ssg->generate)
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		t1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		ssg->generateStep();
		t2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

		ssg->iterationTime = (int)(t2 - t1).count();
	}
}

void SimplexSphereGenerator::generateStep()
{
	if (spherePoints.size() == 0 || sphereComplexity != oldSphereComplexity)
	{
		int c = sphereComplexity;
		spherePoints.clear();
		initialize_sphere(spherePoints, c);
		for (int i = 0; i < spherePoints.size(); i++)
			D3DXVec3Normalize(&spherePoints[i], &spherePoints[i]);
		verticesCount = spherePoints.size();
		oldSphereComplexity = c;

		if (tarrtriangle != null)
		{
			delete[] parrtriangle;
			delete[] narrtriangle;
			delete[] carrtriangle;
			delete[] tarrtriangle;
			delete[] parrline;
			delete[] narrline;
			delete[] carrline;
			delete[] tarrline;
			tarrtriangle = null;
		}
	}



	if (tarrtriangle == NULL)
	{
		//triangles
		parrtriangle = new D3DXVECTOR3[verticesCount];
		narrtriangle = new D3DXVECTOR3[verticesCount];
		carrtriangle = new D3DXVECTOR4[verticesCount];
		tarrtriangle = new D3DXVECTOR2[verticesCount];
		for (int i = 0; i < verticesCount; i++)
		{
			tarrtriangle[i] = D3DXVECTOR2(0, 0);
			narrtriangle[i] = spherePoints[i];
		}

		//lines
		parrline = new D3DXVECTOR3[verticesCount * 2];
		narrline = new D3DXVECTOR3[verticesCount * 2];
		carrline = new D3DXVECTOR4[verticesCount * 2];
		tarrline = new D3DXVECTOR2[verticesCount * 2];
		for (int i = 0; i < verticesCount * 2; i++)
			tarrline[i] = D3DXVECTOR2(0, 0);
		for (int i = 0; i < verticesCount; i += 3)
		{
			narrline[i * 2] = spherePoints[i];
			narrline[i * 2 + 1] = spherePoints[i + 1];
			narrline[i * 2 + 2] = spherePoints[i + 1];
			narrline[i * 2 + 3] = spherePoints[i + 2];
			narrline[i * 2 + 4] = spherePoints[i + 2];
			narrline[i * 2 + 5] = spherePoints[i];
		}
	}

	simplexw += 0.001f;

	static float s1, s2, s3;
	static int i2;
	static D3DXVECTOR3 v1, v2, v3;
	for (int i = 0; i < verticesCount; i += 3)
	{
		i2 = i * 2;
		v1 = spherePoints[i];
		v2 = spherePoints[i+1];
		v3 = spherePoints[i+2];

		s1 = scaled_octave_noise_4d(3, .5f, 1.3f, 0, 1, v1.x, v1.y, v1.z, simplexw);
		s2 = scaled_octave_noise_4d(3, .5f, 1.3f, 0, 1, v2.x, v2.y, v2.z, simplexw);
		s3 = scaled_octave_noise_4d(3, .5f, 1.3f, 0, 1, v3.x, v3.y, v3.z, simplexw);

		parrtriangle[i] = parrline[i2] = parrline[i2 + 5] = v1 * (1.5f + s1 * 3);
		parrtriangle[i + 1] = parrline[i2 + 1] = parrline[i2 + 2] = v2 * (1.5f + s2 * 3);
		parrtriangle[i + 2] = parrline[i2 + 3] = parrline[i2 + 4] = v3 * (1.5f + s3 * 3);

		carrtriangle[i] = carrline[i2] = carrline[i2 + 5] = D3DXVECTOR4(s1, s1, s1, 1);
		carrtriangle[i + 1] = carrline[i2 + 1] = carrline[i2 + 2] = D3DXVECTOR4(s2, s2, s2, 1);
		carrtriangle[i + 2] = carrline[i2 + 3] = carrline[i2 + 4] = D3DXVECTOR4(s3, s3, s3, 1);
	}


	mutex.lock();
	finishedGenerating = false;
	mutex.unlock();

	if (triangles.meshSwap == NULL)
		triangles.meshSwap = Mesh::CreateFromVertices(parrtriangle, narrtriangle, carrtriangle, tarrtriangle, verticesCount, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, false);
	else
		triangles.meshSwap->SetVertices(parrtriangle, narrtriangle, carrtriangle, tarrtriangle, verticesCount, false);
	if (lines.meshSwap == NULL)
		lines.meshSwap = Mesh::CreateFromVertices(parrline, narrline, carrline, tarrline, verticesCount * 2, D3D11_PRIMITIVE_TOPOLOGY_LINELIST, false);
	else
		lines.meshSwap->SetVertices(parrline, narrline, carrline, tarrline, verticesCount * 2, false);

	finishedGenerating = true;
}

void SimplexSphereGenerator::subdivide(const D3DXVECTOR3 &v1, const D3DXVECTOR3 &v2, const D3DXVECTOR3 &v3, vector<D3DXVECTOR3> &sphere_points, const unsigned int depth) {
	if (depth == 0) {
		sphere_points.push_back(v1);
		sphere_points.push_back(v2);
		sphere_points.push_back(v3);
		return;
	}
	D3DXVECTOR3 v12; D3DXVec3Normalize(&v12, &(v1 + v2));
	D3DXVECTOR3 v23; D3DXVec3Normalize(&v23, &(v2 + v3));
	D3DXVECTOR3 v31; D3DXVec3Normalize(&v31, &(v3 + v1));
	subdivide(v1, v12, v31, sphere_points, depth - 1);
	subdivide(v2, v23, v12, sphere_points, depth - 1);
	subdivide(v3, v31, v23, sphere_points, depth - 1);
	subdivide(v12, v23, v31, sphere_points, depth - 1);
}

void SimplexSphereGenerator::initialize_sphere(vector<D3DXVECTOR3> &sphere_points, const unsigned int depth) {
	const double X = 0.525731112119133606;
	const double Z = 0.850650808352039932;
	const D3DXVECTOR3 vdata[12] = {
			D3DXVECTOR3( -X, 0.0, Z ), D3DXVECTOR3( X, 0.0, Z  ), D3DXVECTOR3( -X, 0.0, -Z ), D3DXVECTOR3( X, 0.0, -Z  ),
			D3DXVECTOR3( 0.0, Z, X  ), D3DXVECTOR3( 0.0, Z, -X ), D3DXVECTOR3( 0.0, -Z, X  ), D3DXVECTOR3( 0.0, -Z, -X ),
			D3DXVECTOR3( Z, X, 0.0  ), D3DXVECTOR3( -Z, X, 0.0 ), D3DXVECTOR3( Z, -X, 0.0  ), D3DXVECTOR3( -Z, -X, 0.0 )
	};
	int tindices[20][3] = {
			{ 0, 4, 1 }, { 0, 9, 4 }, { 9, 5, 4 }, { 4, 5, 8 }, { 4, 8, 1 },
			{ 8, 10, 1 }, { 8, 3, 10 }, { 5, 3, 8 }, { 5, 2, 3 }, { 2, 7, 3 },
			{ 7, 10, 3 }, { 7, 6, 10 }, { 7, 11, 6 }, { 11, 0, 6 }, { 0, 1, 6 },
			{ 6, 1, 10 }, { 9, 0, 11 }, { 9, 11, 2 }, { 9, 2, 5 }, { 7, 2, 11 }
	};
	for (int i = 0; i < 20; i++)
		subdivide(vdata[tindices[i][0]], vdata[tindices[i][1]], vdata[tindices[i][2]], sphere_points, depth);
}