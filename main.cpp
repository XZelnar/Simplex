#include "stdafx.h"
#include "window.h"
#include "Graphics/graphicsEngine.h"
#include "Graphics/graphicsResourceManager.h"
#include "Scenes/sceneManager.h"
#include "Input/inputManager.h"

inline bool checkFile(const char* name);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (!checkFile("Resources/pixel.png")) return 1;
	if (!checkFile("Resources/gradient.png")) return 1;
	if (!checkFile("Resources/gradientNeon.png")) return 1;
	if (!checkFile("Resources/Fonts/Anonymous_32_b.fnt")) return 1;
	if (!checkFile("Resources/Fonts/Anonymous_32_b_0.png")) return 1;
	if (!checkFile("Resources/Fonts/Aurek-Besh Hand_32.fnt")) return 1;
	if (!checkFile("Resources/Fonts/Aurek-Besh Hand_32_0.png")) return 1;

	try
	{
		Window::Initialize(hInstance, nCmdShow);
		GraphicsEngine::Initialize(Window::GetHWND());
		SceneManager::Initialize();
		InputManager::Initialize(hInstance);

		//Window::SetSize(1920, 1080, true);

		DWORD timeStep = 1000.0f / 60.0f;
		DWORD timeLast = timeGetTime();

		bool firstpaint = true;
		MSG msg;
		while (1)
		{
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				if (msg.message == WM_QUIT)
					goto applicationExit;
			}

			SceneManager::PreInput();
			InputManager::Update();
			SceneManager::PostInput();

			SceneManager::PreUpdate();
			SceneManager::Update();
			GraphicsEngine::Update();
			SceneManager::PostUpdate();

			SceneManager::PreRender();
			GraphicsEngine::BeginFrame();
			if (GraphicsEngine::skipFrame)
			{
				GraphicsEngine::skipFrame = false;
				goto frameEnd;
			}
			SceneManager::Render();
			SceneManager::PostRender();

			SceneManager::PreGUI();
			GraphicsEngine::BeginGUI();
			SceneManager::GUI();
			SceneManager::PostGUI();

			GraphicsEngine::EndFrame();

			frameEnd:
			while (timeGetTime() < timeLast + timeStep);
			timeLast = timeGetTime();
		}
		applicationExit:

		InputManager::Dispose();
		SceneManager::Dispose();
		GraphicsResourceManager::GetInstance().Dispose();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));

#if DEBUG
		ID3D11Debug* m_d3dDebug;
		GraphicsEngine::GetDevice()->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&m_d3dDebug));
		m_d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
#endif

		GraphicsEngine::Dispose();
	}
	catch (...)
	{
		try
		{
			std::rethrow_exception(current_exception());
		}
		catch (std::exception& e)
		{
			MessageBoxA(null, e.what(), null, MB_OK);
		}
	}

	return 0;
}

inline bool checkFile(const char* name)
{
	if (FILE *file = fopen(name, "r"))
	{
		fclose(file);
		return true;
	}
	else
	{
		char c[200] = "File missing: \"";
		strcat(c, name);
		strcat(c, "\"\nTerminating...");
		MessageBoxA(null, c, "Resource missing", MB_OK);
		return false;
	}
}