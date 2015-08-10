#pragma once

#include <Windows.h>
#include <WindowsX.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcommon.h>
#include <d3dx10math.h>
#include <dinput.h>
#include <dinputd.h>
#include <utility>
#include <algorithm>
#include <random>
#include <string.h>
#include <stdlib.h>
#include <map>
#include <stack>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <D3D11Shader.h>
#include <D3Dcompiler.h>
#include <thread>
#include <mutex>
#include <ctime>
#include <queue>
#include <DXGI.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3d10.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "dxgi.lib")


#define null NULL
#define sign(a) (a > 0 ? 1 : a < 0 ? -1 : 0)