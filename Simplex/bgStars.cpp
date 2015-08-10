#include "bgStars.h"

#include "../window.h"
#include "../Graphics/graphicsEngine.h"
#include "../Graphics/textureDynamic.h"
#include "../Graphics/renderHelper.h"
#include "../Graphics/material.h"
#include "../Graphics/shaderManager.h"
#include "../Graphics/graphicsResourceManager.h"
#include "simplexnoise.h"

void BGStars::Initialize()
{
	updateResolution = false;
	generate = true;
	threadExit = false;
	scaleChanged = false;
	simplexz = 0;
	iterationTime = 0;
	texture = null;
	colors = null;

	setupTexture();
	genStars();

	material = new Material(GraphicsResourceManager::GetInstance().GetShader(L"Resources/Shaders/simple.fx"));
	material->updateTxtures = false;
	static D3D11_INPUT_ELEMENT_DESC ied3d[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	material->SetInputLayout(ied3d, _countof(ied3d));

	genThread = new std::thread(BGStars::genThreadFunc, this);
}

void BGStars::Dispose()
{
	threadExit = true;
	genThread->join();

	texture->Dispose();
	delete texture;
	delete[] colors;
	delete material;
}

void BGStars::Update()
{
	if (updateResolution)
	{
		setupTexture();
		updateResolution = false;
	}
	texture->SetPixels(colors);
}

void BGStars::Render()
{
	static D3DXMATRIX projection, view;
	D3DXMatrixTranslation(&view, -.5f, -.5f, 0);
	D3DXMatrixOrthoRH(&projection, 1, 1, 0.01f, 100);

	material->Activate();
	ShaderManager::SetMatrixProjection(&projection);
	ShaderManager::SetMatrixView(&view);
	GraphicsEngine::DisableDepth();
	RenderHelper::RenderSquare(0, 0, 1, 1, texture);
	GraphicsEngine::EnableDepth();
}



void BGStars::genStars()
{
	int w = texture->GetWidth(), h = texture->GetHeight();
	stars.clear();
	uniform_int_distribution<int> rand(0, 1000);
	uniform_int_distribution<int> rand2(40, 170);
	default_random_engine e_rand;

	short x, y;
	for (y = 0; y < h; y++)
		for (x = 0; x < w; x++)
			if (rand(e_rand) > 998)
				stars.push_back({ x, y, (byte)rand2(e_rand) });
	starsCount = stars.size();
}

void BGStars::setupTexture()
{
	if (texture)
	{
		texture->Dispose();
		delete texture;
	}
	oldw = Window::GetWidth();
	oldh = Window::GetHeight();
	texture = new TextureDynamic(Window::GetWidth(), Window::GetHeight());
	colorsLength = texture->GetWidth() * texture->GetHeight() * 4;
	if (colors)
		delete[] colors;
	colors = new byte[colorsLength];
	for (int i = 0; i < colorsLength; )
	{
		colors[i++] = 255;//r
		colors[i++] = 255;//g
		colors[i++] = 255;//b
		colors[i++] = 0;//a
	}
}

void BGStars::genThreadFunc(BGStars* bgn)
{
	std::chrono::milliseconds t1, t2;
	while (!bgn->threadExit)
	{
		while (!bgn->generate && !bgn->threadExit)
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		t1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		bgn->genStep();
		t2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

		bgn->iterationTime = (int)(t2 - t1).count();
	}
}

void BGStars::genStep()
{
	static int w = texture->GetWidth(), h = texture->GetHeight();
	if (oldw != Window::GetWidth() || oldh != Window::GetHeight() || scaleChanged)
	{
		updateResolution = true;
		while (updateResolution && !threadExit)
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		scaleChanged = false;

		w = texture->GetWidth();
		h = texture->GetHeight();

		genStars();
	}

	simplexz += 0.002f * iterationTime;


	for (int i = 0; i < starsCount; i++)
		colors[(stars[i].x + stars[i].y * w) * 4 + 3] = stars[i].brightness + raw_noise_3d(stars[i].x, stars[i].y, simplexz) * 40;
}