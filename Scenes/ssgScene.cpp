#include "ssgScene.h"

#include "../Graphics/graphicsEngine.h"
#include "../Graphics/cameraProjection.h"
#include "../Graphics/cameraOrtho.h"
#include "../Graphics/cameraManager.h"
#include "../Graphics/shaderManager.h"
#include "../Graphics/texture.h"
#include "../Graphics/graphicsResourceManager.h"
#include "../Graphics/renderHelper.h"
#include "../Graphics/material.h"
#include "../Graphics/shader.h"
#include "../Input/inputManager.h"
#include "../Graphics/textPrinter.h"
#include "../Simplex/simplexSphereGenerator.h"
#include "../Simplex/bgNebula.h"
#include "../Simplex/bgStars.h"
#include "../window.h"

void SSGScene::Initialize()
{
	showInfo = false;

	c = new CameraProjection();
	c->SetClipPlanes(.1f, 100);
	CameraManager::SetActiveCamera((Camera*)c);
	c->SetPosition(D3DXVECTOR3(0, 0, 12), D3DXVECTOR3(0, 0, 0));

	c2 = new CameraOrtho();
	CameraManager::SetActive2DCamera(c2);

	ssg = new SimplexSphereGenerator();
	ssg->Initialize();

	bgn = new BGNebula();
	bgn->Initialize();

	bgs = new BGStars();
	bgs->Initialize();


	infoOverlay = new Material(GraphicsResourceManager::GetInstance().GetShader(L"Resources/Shaders/text.fx"));
	infoOverlay->SetTexture(GraphicsResourceManager::GetInstance().GetTexture(L"Resources/pixel.png"));
	infoOverlay->SetPixelCBuffer(0, D3DXVECTOR4(0, 0, 0, .6f));
	static D3D11_INPUT_ELEMENT_DESC ied3d[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	infoOverlay->SetInputLayout(ied3d, _countof(ied3d));
}

void SSGScene::Dispose()
{
	bgn->Dispose();
	delete bgn;
	bgs->Dispose();
	delete bgs;
	ssg->Dispose();
	delete ssg;
	delete c;
	delete c2;
}

void SSGScene::PostInput()
{
	if (InputManager::kbIsKeyDown(DIK_I))
		showInfo = !showInfo;

	if (InputManager::kbIsKeyDown(DIK_A))
		ssg->DecComplexity();
	if (InputManager::kbIsKeyDown(DIK_S))
		ssg->IncComplexity();
	if (InputManager::kbIsKeyDown(DIK_Q))
		bgn->DecScale();
	if (InputManager::kbIsKeyDown(DIK_W))
		bgn->IncScale();

	if (InputManager::kbIsKeyDown(DIK_Z))
		ssg->ToggleTrianglesVisibility();
	if (InputManager::kbIsKeyDown(DIK_X))
		ssg->ToggleWireframeVisibility();
	if (InputManager::kbIsKeyDown(DIK_C))
		ssg->SetPopupVisibility(!ssg->GetPopupVisibility());

	if (InputManager::kbIsKeyDown(DIK_ESCAPE))
		PostQuitMessage(0);
	if (InputManager::kbIsKeyDown(DIK_F))
		if (Window::IsFullscreen())
			Window::SetSize(1280, 760, false);
		else
			Window::SetSize(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), true);

	ssg->PostInput();
}

void SSGScene::Update()
{
	ssg->Update();
	bgn->Update();
	bgs->Update();
}

void SSGScene::Render()
{
	bgn->Render();
	bgs->Render();
	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);
	ShaderManager::SetMatrixWorld(&identity);
	ssg->Render();
}

void SSGScene::OnResolutionChanged(int neww, int newh, int oldw, int oldh)
{
	ssg->OnResolutionChanged();
}

void SSGScene::GUI()
{
	ssg->GUI();

	int w = Window::GetWidth(), h = Window::GetHeight();
	static std::string s;

	if (showInfo)
	{
		infoOverlay->Activate();
		RenderHelper::RenderSquare(0, 0, Window::GetWidth(), Window::GetHeight(), null);
	}

	static float t1w = TextPrinter::MeasureString("[I] to show info", "Anonymous", 32, FONT_STYLE::FS_Bold, true).x;
	TextPrinter::Print(showInfo ? "[I] to hide info" : "[I] to show info", "Anonymous", 32, FONT_STYLE::FS_Bold, w - t1w, 0, D3DXVECTOR4(1, 1, 1, 1), ALIGN_Left, 100, true);

	if (!showInfo)
		return;

	TextPrinter::Print("FPS:", "Anonymous", 32, FONT_STYLE::FS_Bold, 0, 0, D3DXVECTOR4(1, 1, 1, 1), ALIGN_Left, 100, true);
	static int fps;
	static char buf[20];
	fps = GraphicsEngine::GetFPS();
	TextPrinter::Print(_itoa(fps, buf, 10), "Anonymous", 32, FONT_STYLE::FS_Bold, 70, 0, fps >= 30 ? D3DXVECTOR4(0, 1, 0, 1) : D3DXVECTOR4(1, 0, 0, 1), ALIGN_Left, 100, true);

#if DEBUG
	fps = GraphicsEngine::GetViewport().Width;
	fps = GetSystemMetrics(SM_CXSCREEN);
	TextPrinter::Print(_itoa(fps, buf, 10), "Anonymous", 32, FONT_STYLE::FS_Bold, 170, 0, D3DXVECTOR4(1, 1, 1, 1), ALIGN_Left, 100, true);
#endif

	TextPrinter::Print("Terrain vertices:", "Anonymous", 32, FONT_STYLE::FS_Bold, 0, 60, D3DXVECTOR4(1, 1, 1, 1), ALIGN_Left, 100, true);
	TextPrinter::Print(_itoa(ssg->GetVertexCount() * 2, buf, 10), "Anonymous", 32, FONT_STYLE::FS_Bold, 305, 60, D3DXVECTOR4(1, 1, 1, 1), ALIGN_Left, 100, true);

	TextPrinter::Print("Terrain complexity:", "Anonymous", 32, FONT_STYLE::FS_Bold, 0, 90, D3DXVECTOR4(1, 1, 1, 1), ALIGN_Left, 100, true);
	TextPrinter::Print(_itoa(ssg->GetComplexity(), buf, 10), "Anonymous", 32, FONT_STYLE::FS_Bold, 345, 90, D3DXVECTOR4(1, 1, 1, 1), ALIGN_Left, 100, true);

	TextPrinter::Print("Background resolution:", "Anonymous", 32, FONT_STYLE::FS_Bold, 0, 150, D3DXVECTOR4(1, 1, 1, 1), ALIGN_Left, 100, true);
	TextPrinter::Print(bgn->GetScale() == BGNebula::TextureScale::x025 ? "x0.25" : bgn->GetScale() == BGNebula::TextureScale::x05 ? "x0.5" : "x1", "Anonymous", 32,
		FONT_STYLE::FS_Bold, 400, 150, D3DXVECTOR4(1, 1, 1, 1), ALIGN_Left, 100, true);

	TextPrinter::Print("Wireframe:", "Anonymous", 32, FONT_STYLE::FS_Bold, 0, 210, D3DXVECTOR4(1, 1, 1, 1), ALIGN_Left, 100, true);
	TextPrinter::Print("Fill:", "Anonymous", 32, FONT_STYLE::FS_Bold, 0, 240, D3DXVECTOR4(1, 1, 1, 1), ALIGN_Left, 100, true);
	TextPrinter::Print("Popups:", "Anonymous", 32, FONT_STYLE::FS_Bold, 0, 270, D3DXVECTOR4(1, 1, 1, 1), ALIGN_Left, 100, true);
	static bool b;
	b = ssg->GetWireframeDesiredVisibility();
	TextPrinter::Print(b ? "Visible" : "Invisible", "Anonymous", 32, FONT_STYLE::FS_Bold, 220, 210, b ? D3DXVECTOR4(0, 1, 0, 1) : D3DXVECTOR4(1, 0, 0, 1), ALIGN_Left, 100, true);
	b = ssg->GetTrianglesDesiredVisibility();
	TextPrinter::Print(b ? "Visible" : "Invisible", "Anonymous", 32, FONT_STYLE::FS_Bold, 220, 240, b ? D3DXVECTOR4(0, 1, 0, 1) : D3DXVECTOR4(1, 0, 0, 1), ALIGN_Left, 100, true);
	b = ssg->GetPopupVisibility();
	TextPrinter::Print(b ? "Visible" : "Invisible", "Anonymous", 32, FONT_STYLE::FS_Bold, 220, 270, b ? D3DXVECTOR4(0, 1, 0, 1) : D3DXVECTOR4(1, 0, 0, 1), ALIGN_Left, 100, true);

	static std::string keys = "LMB and drag to rotate\n[F] - Fullscreen\n[C] - Toggle popups\n[X] - Toggle wireframe\n[Z] - Toggle terrain filling\n[A][S] - Change terrain complexity\n[Q][W] - Change background resolution\n[Esc] - Exit";
	static D3DXVECTOR2 keysSize = TextPrinter::MeasureString(keys, "Anonymous", 32, FONT_STYLE::FS_Bold, false);
	TextPrinter::Print(keys, "Anonymous", 32, FONT_STYLE::FS_Bold, 0, Window::GetHeight() - keysSize.y, D3DXVECTOR4(1, 1, 1, 1), ALIGN_Left, keysSize.x, true);

	static std::string time = "Terrain generation time\nThread 1:        \n\nBackground generation time\nThread 1:        \nThread 2:        \nThread 3:        \nThread 4:        \nThread 5:        ";
	static D3DXVECTOR2 timeSize = TextPrinter::MeasureString(time, "Anonymous", 32, FONT_STYLE::FS_Bold, false);
	TextPrinter::Print(time, "Anonymous", 32, FONT_STYLE::FS_Bold, w - timeSize.x, h - timeSize.y, D3DXVECTOR4(1, 1, 1, 1), ALIGN_Right, timeSize.x, true);

	s = "";
	_itoa((int)ssg->GetGenerationTime(), buf, 10);
	s.append(buf);
	s.append(" ms");
	TextPrinter::Print(s, "Anonymous", 32, FONT_STYLE::FS_Bold, w - 200, h - timeSize.y + 32, D3DXVECTOR4(1, 1, 1, 1), ALIGN_Right, 200, true);

	_itoa((int)bgn->GetGenerationTime(0), buf, 10);
	s = "";
	s.append(buf);
	s.append(" ms");
	TextPrinter::Print(s, "Anonymous", 32, FONT_STYLE::FS_Bold, w - 200, h - timeSize.y + 128, D3DXVECTOR4(1, 1, 1, 1), ALIGN_Right, 200, true);
	_itoa((int)bgn->GetGenerationTime(1), buf, 10);
	s = "";
	s.append(buf);
	s.append(" ms");
	TextPrinter::Print(s, "Anonymous", 32, FONT_STYLE::FS_Bold, w - 200, h - timeSize.y + 160, D3DXVECTOR4(1, 1, 1, 1), ALIGN_Right, 200, true);
	_itoa((int)bgn->GetGenerationTime(2), buf, 10);
	s = "";
	s.append(buf);
	s.append(" ms");
	TextPrinter::Print(s, "Anonymous", 32, FONT_STYLE::FS_Bold, w - 200, h - timeSize.y + 192, D3DXVECTOR4(1, 1, 1, 1), ALIGN_Right, 200, true);
	_itoa((int)bgn->GetGenerationTime(3), buf, 10);
	s = "";
	s.append(buf);
	s.append(" ms");
	TextPrinter::Print(s, "Anonymous", 32, FONT_STYLE::FS_Bold, w - 200, h - timeSize.y + 224, D3DXVECTOR4(1, 1, 1, 1), ALIGN_Right, 200, true);
	_itoa((int)bgs->GetGenerationTime(), buf, 10);
	s = "";
	s.append(buf);
	s.append(" ms");
	TextPrinter::Print(s, "Anonymous", 32, FONT_STYLE::FS_Bold, w - 200, h - timeSize.y + 256, D3DXVECTOR4(1, 1, 1, 1), ALIGN_Right, 200, true);

}