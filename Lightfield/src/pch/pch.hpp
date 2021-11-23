#pragma once

// main
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <random>

#include <string>
#include <sstream>
#include <iostream>

// containers
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <bitset>
#include <optional>

// utils
#include "utils/Helpers.hpp"

#ifdef Win32
	#include <Windows.h>

	// DirectX 11
	#pragma comment(lib, "d3d11.lib")
	#include <d3d11.h>
	#include <wrl.h> // smart pointers for COM objects
	#include <DirectXMath.h>

	// DirectX 11 Debugging
	#ifdef _DEBUG
	#pragma comment(lib, "dxgi.lib")
	#pragma comment(lib, "dxguid.lib")
	#include <dxgidebug.h>
	#include <dxgi1_3.h>
	#endif
	
	// Shader Compiler
	#pragma comment(lib, "D3DCompiler.lib")
	#include <d3dcompiler.h>
#endif