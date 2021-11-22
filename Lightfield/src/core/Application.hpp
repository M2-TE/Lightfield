#pragma once

#include "Input/Input.hpp"

class Application
{
public:
	Application() : hInstance(GetModuleHandle(nullptr))
	{
		WNDCLASSEX wc = { 0 };
		wc.cbSize = sizeof(wc);
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = WindowProcSetup;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hIcon = nullptr;
		wc.hCursor = nullptr;
		wc.hbrBackground = nullptr;
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = wndTitle;
		wc.hIconSm = nullptr;

		RegisterClassEx(&wc);

		// create actual window
		{
			constexpr long windowStyle = WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU;

			// adjust rect with window borders in mind
			RECT windowRect;
			windowRect.left = 0;
			windowRect.right = width + windowRect.left;
			windowRect.top = 0;
			windowRect.bottom = height + windowRect.top;
			if (AdjustWindowRect(&windowRect, windowStyle, FALSE) == 0)
			{
				throw std::runtime_error("Window Rect Adjustment failed");
			}

			int defaultThingy = CW_USEDEFAULT;
			int x = 0;
			int y = 0;

			// create window and save handle
			hWnd = CreateWindowW(
				wndTitle, wndTitle,
				windowStyle,
				defaultThingy, defaultThingy,
				windowRect.right - windowRect.left,
				windowRect.bottom - windowRect.top,
				nullptr, nullptr, hInstance, this);

			//SetWindowLong(hWnd, GWL_STYLE, 0); //remove all window styles

			if (hWnd == nullptr) throw std::runtime_error("Window could not be created");

			// Setup input for window
			input.hWnd = hWnd;

			SetWindowPos(hWnd, HWND_TOP, width / 4u, height / 4u, width, height, 0u);
			ShowWindow(hWnd, SW_SHOWDEFAULT);
		}
	}
	~Application()
	{
		DestroyWindow(hWnd);
		UnregisterClass(wndTitle, hInstance);
	}

	int Run()
	{
		bRunning = true;

		std::pair<bool, int> msgInfo;
		while (bRunning)
		{
			msgInfo = ReadMessages();
			if (!msgInfo.first) return msgInfo.second;

			//Update();
		}
		return 0;
	}

private:
	std::pair<bool, int> ReadMessages()
	{
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				return std::make_pair(false, static_cast<int>(msg.wParam));
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		return std::make_pair(true, 0);
	}
	void FlushInputs()
	{
		input.kbd.FlushOldInputs();
		input.mouse.FlushOldInputs();
	}

	static LRESULT CALLBACK WindowProcSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		if (msg == WM_NCCREATE)
		{
			// extract ptr to window class from creation data
			const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
			Application* const pWnd = static_cast<Application*>(pCreate->lpCreateParams);

			// set WinAPI-managed user data to store ptr to window instance
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));

			// set message proc to normal (non-setup) handler now that setup is finished
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&WindowProc));

			// forward message to window instance handler
			return pWnd->HandleMessage(hWnd, msg, wParam, lParam);
		}
		// if we get a message before the WM_NCCREATE message, handle with default handler
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		// retrieve ptr to window instance
		Application* const pWnd = reinterpret_cast<Application*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		// forward message to window instance handler
		return pWnd->HandleMessage(hWnd, msg, wParam, lParam);
	}
	LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		switch (msg)
		{
			case WM_DESTROY:
			{
				PostQuitMessage(0);
				break;
			}

			case WM_KILLFOCUS:
			{
				input.isWindowActive = false;
				input.UpdateCursorConfinement();
				input.UpdateCursorVisibility();

				input.kbd.FlushAll();
				input.mouse.FlushAll();
				break;
			}
			case WM_SETFOCUS:
			{
				input.isWindowActive = true;
				input.UpdateCursorConfinement();
				input.UpdateCursorVisibility();
				break;
			}

			/////////////// KEYBOARD ///////////////
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			{
				if (!(lParam & 0x40000000)) // filter autorepeat
				{
					input.kbd.RegisterKeyPress(static_cast<unsigned char>(wParam));
				}
				break;
			}

			case WM_KEYUP:
			case WM_SYSKEYUP:
			{
				input.kbd.RegisterKeyRelease(static_cast<unsigned char>(wParam));
				break;
			}

			case WM_CHAR:
			{
				input.kbd.RegisterChar(static_cast<unsigned char>(wParam));
				break;
			}

			//////////////// MOUSE /////////////////

			case WM_INPUT:
			{
				if (input.isWindowActive)
				{
					input.RegisterRawInput(lParam);
				}
				break;
			}
			case WM_LBUTTONDOWN:
			{
				input.mouse.RegisterMouseButtonPress(MouseButtons::LEFT);
				break;
			}
			case WM_RBUTTONDOWN:
			{
				input.mouse.RegisterMouseButtonPress(MouseButtons::RIGHT);
				break;
			}
			case WM_MBUTTONDOWN:
			{
				input.mouse.RegisterMouseButtonPress(MouseButtons::MIDDLE);
				break;
			}

			case WM_LBUTTONUP:
			{
				input.mouse.RegisterMouseButtonRelease(MouseButtons::LEFT);
				break;
			}
			case WM_RBUTTONUP:
			{
				input.mouse.RegisterMouseButtonRelease(MouseButtons::RIGHT);
				break;
			}
			case WM_MBUTTONUP:
			{
				input.mouse.RegisterMouseButtonRelease(MouseButtons::MIDDLE);
				break;
			}

			case WM_MOUSEWHEEL:
			{
				input.mouse.RegisterMouseWheelDelta(GET_WHEEL_DELTA_WPARAM(wParam));
				break;
			}
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

private:
	static constexpr LPCWSTR wndTitle = L"Lightfield";
	static constexpr int height = 720, width = 1280;
	const HINSTANCE hInstance;
	HWND hWnd;

	bool bRunning = false;
	Input input;
};