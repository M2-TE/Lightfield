#pragma once

// main
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <random>

#define _USE_MATH_DEFINES
#include <math.h>

#include <string>
#include <sstream>
#include <iostream>

// containers
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <bitset>
#include <optional>

#ifdef Win32
	#include <Windows.h>

	// DirectX 11
	#pragma comment(lib, "d3d11.lib")
	#include <d3d11.h>
	#include <wincodec.h> // file conversion codecs
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

	// DirectXTK (only really need this to save textures to image files)
	//#include "BufferHelpers.h"
	//#include "CommonStates.h"
	//#include "DDSTextureLoader.h"
	//#include "DirectXHelpers.h"
	//#include "Effects.h"
	//#include "GamePad.h"
	//#include "GeometricPrimitive.h"
	//#include "GraphicsMemory.h"
	//#include "Keyboard.h"
	//#include "Model.h"
	//#include "Mouse.h"
	//#include "PostProcess.h"
	//#include "PrimitiveBatch.h"
	#include "ScreenGrab.h"
	//#include "SimpleMath.h"
	//#include "SpriteBatch.h"
	//#include "SpriteFont.h"
	//#include "VertexTypes.h"
	//#include "WICTextureLoader.h"
#endif

// utils
#include "utils/Helpers.hpp"
#include "utils/Time.hpp"
#include "utils/TempStringConverter.hpp"