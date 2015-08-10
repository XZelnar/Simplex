#include "spherePopup.h"

#include "../Graphics/renderHelper.h"
#include "../Graphics/cameraManager.h"
#include "../Graphics/camera.h"
#include "../Graphics/mesh.h"
#include "../Graphics/material.h"
#include "../window.h"
#include "../Graphics/graphicsEngine.h"
#include "../Graphics/graphicsResourceManager.h"
#include "../Graphics/shaderManager.h"
#include "../Graphics/textPrinter.h"

void SpherePopup::Initialize()
{
	isDead = false;
	animationState = 0;
	animationMultiplier = 1;
	text = "";
	numbers = "";
	visibleTextLength = 0;
	timeSinceUpdate = 100;

	projection = CameraManager::GetActiveCamera()->GetProjectionMatrix();
	view = CameraManager::GetActiveCamera()->GetViewMatrix();

	matLines = new Material(GraphicsResourceManager::GetInstance().GetShader(L"Resources/Shaders/color.fx"));
	matLines->SetTexture(GraphicsResourceManager::GetInstance().GetTexture(L"Resources/pixel.png"));
	matLines->SetPixelCBuffer(0, D3DXVECTOR4(0, .5f, .5f, 1));
	matWindow = new Material(GraphicsResourceManager::GetInstance().GetShader(L"Resources/Shaders/color.fx"));
	matWindow->SetTexture(GraphicsResourceManager::GetInstance().GetTexture(L"Resources/pixel.png"));
	matWindow->SetPixelCBuffer(0, D3DXVECTOR4(0, 0, .2f, 0.8f));

	static D3D11_INPUT_ELEMENT_DESC ied3d[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	matLines->SetInputLayout(ied3d, _countof(ied3d));

	for (int i = 0; i < 8; i++)
		vWindow[i] = 0;
	for (int i = 0; i < 20; i++)
		vLines[i] = 0;

	meshWindow = Mesh::CreateFromRawData(vWindow, 8, 8 * sizeof(float), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 2 * sizeof(float));
	meshLines = Mesh::CreateFromRawData(vLines, 20, 20 * sizeof(float), D3D11_PRIMITIVE_TOPOLOGY_LINELIST, 2 * sizeof(float));
#if DEBUG
	meshWindow->SetName("w");
	meshLines->SetName("l");
#endif

	viewport.MaxDepth = 1;
	viewport.MinDepth = 0;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	int l = 2 + rand() % 3;
	for (int i = 0; i < l; i++)
		text += 'a' + rand() % ('z' - 'a');
	text += "\n";

	l = 6 + rand() % 3;
	for (int i = 0; i < l; i++)
		text += 'a' + rand() % ('z' - 'a');
	text += "\n";

	char buf[20];
	_itoa((int)(worldPos.x * 100), buf, 10);
	text += buf;
	text += ";";
	_itoa((int)(worldPos.y * 100), buf, 10);
	text += buf;
	text += ";";
	_itoa((int)(worldPos.z * 100), buf, 10);
	text += buf;
}

void SpherePopup::Dispose()
{
	meshWindow->Dispose();
	meshLines->Dispose();

	delete matWindow;
	delete matLines;
}

#define lerp(a, b, t) ( t <= 0 ? a : t >= 1 ? b : (a + (b - a) * t))

void SpherePopup::Update()
{
	D3DXVECTOR3 t;
	auto vp = GraphicsEngine::GetViewport();
	viewport.Width = vp.Width;
	viewport.Height = vp.Height;
	D3DXVec3Project(&t, &worldPos, &viewport, &projection, &view, &world);
	windowWP.x = t.x;
	windowWP.y = t.y;

	D3DXVECTOR4 t4;
	t = worldPos;
	t.y = 0;
	D3DXVec3Normalize(&t, &t);
	D3DXVec3Transform(&t4, &t, &world);
	
	if (animationState > .99f && t4.z <= .5f)
	{
		animationMultiplier = -1;
	}

	if (animationState < 0)
		isDead = true;

	if (animationState < 0.3333f)
	{
		if (windowPos.x < Window::GetWidth() / 2)//line should go to the right top corner
			animationState += animationMultiplier * .3333f * 10 / D3DXVec2Length(&(windowWP - windowPos2));
		else
			animationState += animationMultiplier * .3333f * 10 / D3DXVec2Length(&(windowWP - windowPos));
	}
	else if (animationState < 0.6666f)
	{
		animationState += animationMultiplier * .3333f * 10 / 192;
	}
	else
	{
		animationState += animationMultiplier * .3333f * 10 / 128;
		if (animationState > 1)
		{
			animationState = 1;
			animationMultiplier = 0;
		}
	}

	float s1 = animationState * 3,
		s2 = animationState * 3 - 1,
		s3 = animationState * 3 - 2;

	vLines[0] = windowWP.x;
	vLines[1] = windowWP.y;
	vLines[3] = lerp(windowWP.y, windowPos.y, s1);
	vLines[5] = vLines[7] = vLines[9] = vLines[19] = vWindow[1] = vWindow[5] = windowPos.y;
	vLines[11] = vLines[13] = vLines[15] = vLines[17] = vWindow[3] = vWindow[7] = lerp(windowPos.y, windowPos2.y, s3);

	if (windowPos.x < Window::GetWidth() / 2)//line should go to the right top corner
	{
		vLines[2] = lerp(windowWP.x, windowPos2.x, s1);
		vLines[4] = vLines[14] = vLines[16] = vLines[18] = vWindow[4] = vWindow[6] = windowPos2.x;
		vLines[6] = vLines[8] = vLines[10] = vLines[12] = vWindow[0] = vWindow[2] = lerp(windowPos2.x, windowPos.x, s2);
	}
	else
	{
		vLines[2] = lerp(windowWP.x, windowPos.x, s1);
		vLines[4] = vLines[14] = vLines[16] = vLines[18] = vWindow[0] = vWindow[2] = windowPos.x;
		vLines[6] = vLines[8] = vLines[10] = vLines[12] = vWindow[4] = vWindow[6] = lerp(windowPos.x, windowPos2.x, s2);
	}

	if (s3 > 0)
	{
		vWindow[3]--;
		vWindow[4]--;
		vWindow[6]--;
		vWindow[7]--;
	}

	meshLines->SetRawData(vLines, 20, 20 * sizeof(float), 2 * sizeof(float));
	meshWindow->SetRawData(vWindow, 8, 8 * sizeof(float), 2 * sizeof(float));
}

void SpherePopup::GUILines()
{
	static D3DXMATRIX identity = *D3DXMatrixIdentity(&identity);
	matLines->Activate();
	ShaderManager::SetMatrixWorld(&identity);
	meshLines->Render();
}

void SpherePopup::GUIWindow()
{
	static D3DXMATRIX identity = *D3DXMatrixIdentity(&identity);
	matWindow->Activate();
	ShaderManager::SetMatrixWorld(&identity);
	meshWindow->Render();

	if (animationMultiplier == -1)
	{
		int t = animationState * 3 * 4 - 8;
		if (t == 0)
			visibleTextLength = 0;
		else if (t == 1)
			visibleTextLength = text.find('\n');
		else if (t == 2)
			visibleTextLength = text.find_last_of('\n');
		else if (t == 3)
			visibleTextLength = text.length();
	}

	std::string t;
	t = text.substr(0, min(visibleTextLength, text.length()));
	if (t.length() > 0)
		TextPrinter::Print(t, "Aurek-Besh Hand", 32, FONT_STYLE::FS_None, windowPos.x, windowPos.y - 2, D3DXVECTOR4(1, 1, 1, 1), ALIGN_Left, 100, false);

	if (visibleTextLength > text.length())
	{
		if (timeSinceUpdate++ > 20)
		{
			timeSinceUpdate = 0;
			numbers = "";
			char buf[20];
			_itoa((int)(windowWP.x * 100), buf, 10);
			numbers += buf;
			numbers += ";";
			_itoa((int)(windowWP.y * 100), buf, 10);
			numbers += buf;
		}
		t = numbers.substr(0, min(numbers.length(), visibleTextLength - text.length()));
		if (t.length() > 0)
			TextPrinter::Print(t, "Aurek-Besh Hand", 32, FONT_STYLE::FS_None, windowPos.x, windowPos.y + 94, D3DXVECTOR4(1, 1, 1, 1), ALIGN_Left, 100, false);
	}

	if (animationState >= 0.99f)
		visibleTextLength++;
}

void SpherePopup::SetVars(D3DXVECTOR2 _windowPosition, D3DXVECTOR3 _worldPoint, int _sphereIndex)
{
	windowPos = _windowPosition;
	windowPos2 = windowPos + D3DXVECTOR2(192, 128);

	worldPos = _worldPoint;
	sphereInd = _sphereIndex;
}