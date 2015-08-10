#include "bgNebula.h"

#include "../window.h"
#include "../Graphics/graphicsEngine.h"
#include "../Graphics/textureDynamic.h"
#include "../Graphics/renderHelper.h"
#include "../Graphics/material.h"
#include "../Graphics/shaderManager.h"
#include "../Graphics/graphicsResourceManager.h"
#include "simplexnoise.h"

void BGNebula::Initialize()
{
	updateResolution[0] = updateResolution[1] = updateResolution[2] = updateResolution[3] = false;
	generate = true;
	threadExit = false;
	scaleChanged = false;
	recombineArray = false;
	simplexz = 0;
	iterationTime[0] = iterationTime[1] = iterationTime[2] = iterationTime[3] = 0;
	scale = TextureScale::x05;
	texture = null;
	colors = null;

	setupTexture(scale, false);

	material = new Material(GraphicsResourceManager::GetInstance().GetShader(L"Resources/Shaders/simple.fx"));
	material->updateTxtures = false;
	static D3D11_INPUT_ELEMENT_DESC ied3d[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	material->SetInputLayout(ied3d, _countof(ied3d));

	genThread0 = new std::thread(BGNebula::genThreadFunc, this, 0);
	genThread1 = new std::thread(BGNebula::genThreadFunc, this, 1);
	genThread2 = new std::thread(BGNebula::genThreadFunc, this, 2);
	combineThread = new std::thread(BGNebula::combineThreadFunc, this);
}

void BGNebula::Dispose()
{
	threadExit = true;
	genThread0->join();
	genThread1->join();
	genThread2->join();
	combineThread->join();

	texture->Dispose();
	delete texture;
	delete[] colors;
	delete material;
}

void BGNebula::Update()
{
	if (updateResolution[0] && updateResolution[1] && updateResolution[2] && updateResolution[3])
	{
		setupTexture(scale, true);
		updateResolution[0] = updateResolution[1] = updateResolution[2] = updateResolution[3] = false;
	}
	texture->SetPixels(colors);
	recombineArray = true;
}

void BGNebula::Render()
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



void BGNebula::setupTexture(TextureScale s, bool copyColor)
{
	float multiplier = s == TextureScale::x025 ? .25f : s == TextureScale::x05 ? .5f : 1;
	int oldx = -1, oldy = -1;
	if (texture && copyColor)
	{
		oldx = texture->GetWidth();
		oldy = texture->GetHeight();
	}
	oldw = Window::GetWidth();
	oldh = Window::GetHeight();
	if (texture)
	{
		texture->Dispose();
		delete texture;
	}
	texture = new TextureDynamic(Window::GetWidth() * multiplier, Window::GetHeight() * multiplier);
	float w = texture->GetWidth(), h = texture->GetHeight();
	colorsLength = texture->GetWidth() * texture->GetHeight();
	auto a = new byte[colorsLength * 4];
	byte* r = new byte[colorsLength];
	byte* g = new byte[colorsLength];
	byte* b = new byte[colorsLength];
	if (copyColor && oldx != -1)
	{
		int tx, ty;
		for (float y = 0; y < h; y++)
			for (float x = 0; x < w; x++)
			{
				tx = x * oldx / w;
				ty = y * oldy / h;
				r[(int)(x + y * w)] = ra[(int)(tx + ty * oldx)];
				g[(int)(x + y * w)] = ga[(int)(tx + ty * oldx)];
				b[(int)(x + y * w)] = ba[(int)(tx + ty * oldx)];

				a[(int)(x + y * w) * 4 + 0] = colors[(int)(tx + ty * oldx) * 4 + 0];
				a[(int)(x + y * w) * 4 + 1] = colors[(int)(tx + ty * oldx) * 4 + 1];
				a[(int)(x + y * w) * 4 + 2] = colors[(int)(tx + ty * oldx) * 4 + 2];
				a[(int)(x + y * w) * 4 + 3] = 255;
			}
	}
	else
		for (int i = 0; i < colorsLength; i++)
		{
			r[i] = g[i] = b[i] = 0;
			a[i * 4] = 0;
			a[i * 4 + 1] = 0;
			a[i * 4 + 2] = 0;
			a[i * 4 + 3] = 255;
		}
	if (colors)
	{
		delete[] colors;
		delete[] ra;
		delete[] ga;
		delete[] ba;
	}
	ra = r;
	ga = g;
	ba = b;
	colors = a;
}

void BGNebula::genThreadFunc(BGNebula* bgn, int offset)
{
	std::chrono::milliseconds t1, t2;
	while (!bgn->threadExit)
	{
		while (!bgn->generate && !bgn->threadExit)
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		t1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		bgn->genStep(offset);
		t2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

		bgn->iterationTime[offset] = (int)(t2 - t1).count();
	}
}

void BGNebula::genStep(int offset)
{
	static int w = texture->GetWidth(), h = texture->GetHeight();
	if (oldw != Window::GetWidth() || oldh != Window::GetHeight() || scaleChanged)
	{
		updateResolution[offset] = true;
		while (updateResolution[offset] && !threadExit)
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		scaleChanged = false;

		w = texture->GetWidth();
		h = texture->GetHeight();
	}

	static float multiplier;
	static float z2;
	if (offset == 0)
	{
		multiplier = (scale == TextureScale::x025 ? 1 : scale == TextureScale::x05 ? 2 : 4);
		simplexz += 0.1f;
		z2 = simplexz / 2;
	}
	float muloffset = offset * 10000;
	
	short s;
	byte* arr = offset == 0 ? ra : offset == 1 ? ga : ba;

	//uniform_int_distribution<int> rand(0, 1000);
	//uniform_int_distribution<int> rand2(80, 170);
	//default_random_engine e_rand;
	
	int x;
	for (int y = 0; y < h; y++)
	{
		if (threadExit)
			return;
		for (x = 0; x < w; x++)
		{
			s = (short)scaled_octave_noise_3d(5, .7f, .01f, -250, 200, x / multiplier + muloffset, y / multiplier, simplexz);

			if (s < 0) s = 0;

			//if (rand(e_rand) > 995)
			//{
			//	s += rand2(e_rand) + raw_noise_2d(x, z2) * 40;
			//	if (s > 255) s = 255;
			//}

			arr[x + y * w] = (byte)s;
		}
	}
}

void BGNebula::combineThreadFunc(BGNebula* bgn)
{
	std::chrono::milliseconds t1, t2;
	while (!bgn->threadExit)
	{
		while (!bgn->generate && !bgn->threadExit)
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		t1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		bgn->combine();
		t2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

		bgn->iterationTime[3] = (int)(t2 - t1).count();
	}
}

void BGNebula::combine()
{
	if (oldw != Window::GetWidth() || oldh != Window::GetHeight() || scaleChanged)
	{
		updateResolution[3] = true;
		while (updateResolution[3] && !threadExit)
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	while (!recombineArray && !threadExit)
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	if (threadExit)
		return;
	int l = texture->GetWidth() * texture->GetHeight();

	static int i, i4;
	for (i = 0; i < l; i++)
	{
		i4 = i * 4;
		colors[i4 + 0] = ra[i];
		colors[i4 + 1] = ga[i];
		colors[i4 + 2] = ba[i];
	}

	recombineArray = false;
}