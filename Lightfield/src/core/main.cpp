#include "pch.hpp"
#include "windows/Application.hpp"

int CALLBACK WinMain(
	_In_		HINSTANCE	hInstance,
	_In_opt_	HINSTANCE	hPrevInstance,
	_In_		LPSTR		lpCmdLine,
	_In_		int			nCmdShow)
{
	auto app = new Application();

	int exitCode = -1;
	try
	{
		exitCode = app->Run();
	}
	catch (const std::exception& e)
	{
		MessageBox(
			nullptr, s2ws(e.what()).c_str(), L"Unhandled Exception",
			MB_OK | MB_ICONEXCLAMATION);
	}
	catch (...)
	{
		MessageBox(
			nullptr, L"An unkown exception has occurred", L"Unknown Exception",
			MB_OK | MB_ICONEXCLAMATION);
	}

	delete app;

	// track potentially live d3d11 objects
#ifdef _DEBUG
	IDXGIDebug* debugDev;
	HRESULT hr = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debugDev));
	debugDev->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
	debugDev->Release();
#endif

	return exitCode;
}