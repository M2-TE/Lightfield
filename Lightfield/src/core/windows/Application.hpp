#pragma once

#include "input/Input.hpp"
#include "dx11/Renderer.hpp"

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
		constexpr long windowStyle = WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU;

		// adjust rect with window borders in mind
		RECT windowRect = {};
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

		// Setup renderer and input for window
		pRenderer = std::make_unique<Renderer>(hWnd, width, height);
		input.hWnd = hWnd;

		input.SetCursorConfinement(true);
		input.SetCursorVisibility(false);

		SetWindowPos(hWnd, HWND_TOP, width / 4u, height / 4u, width, height, 0u);
		ShowWindow(hWnd, SW_SHOWDEFAULT);
	}
	~Application()
	{
		DestroyWindow(hWnd);
		UnregisterClass(wndTitle, hInstance);
	}

	int Run()
	{
		Start();
		std::pair<bool, int> msgInfo;
		while (true)
		{
			msgInfo = ReadMessages();
			if (!msgInfo.first) return msgInfo.second;

			Update();
		}
		return 0;
	}

private:
	void Start()
	{
		auto* pDevice = pRenderer->GetDevice();
		auto& renderObjects = pRenderer->GetRenderObjects();

		// move cam a bit further back to see the entire scene
		pRenderer->GetCamera().GetTransform().Translate(0.0f, 0.0f, -10.0f);

		// create some objects to view
		{
			renderObjects.emplace_back(std::make_unique<RenderObject>(pDevice, Primitive::Cube));
			auto& transform = renderObjects.back()->GetTransform();
			transform.Translate(0.0f, 0.0f, 0.0f);
			transform.SetScale(0.3f, 0.3f, 0.3f);
		}
		{
			renderObjects.emplace_back(std::make_unique<RenderObject>(pDevice, Primitive::Quad));
			auto& transform = renderObjects.back()->GetTransform();
			transform.Translate(-0.01f, -3.5f, 0.0f);
			transform.RotateEuler(static_cast<float>(M_PI_2), 0.0f, 0.0f);
			transform.SetScale(10.0f, 10.0f, 1.0f);
		}
		//{
		//	renderObjects.emplace_back(std::make_unique<RenderObject>(pDevice, "Bevel_Floor"));
		//	auto& transform = renderObjects.back()->GetTransform();
		//	transform.Translate(0.0f, -3.49f, 0.0f);
		//	transform.SetScale(5.0f, 1.0f, 5.0f);
		//}
		//{
		//	renderObjects.emplace_back(std::make_unique<RenderObject>(pDevice, "PLANTS_ON_TABLE_10k"));
		//	auto& transform = renderObjects.back()->GetTransform();
		//	transform.Translate(3.8f, -3.5f, -1.5f);
		//	transform.RotateEuler(0.0f, static_cast<float>(M_PI_2) - 0.25f, 0.0f);
		//	transform.SetScale(1.0f, 1.0f, 1.0f);
		//}
		//{
		//	renderObjects.emplace_back(std::make_unique<RenderObject>(pDevice, "Medieval_boxes"));
		//	auto& transform = renderObjects.back()->GetTransform();
		//	transform.Translate(-4.0f, -3.49f, -4.0f);
		//	transform.RotateEuler(0.0f, static_cast<float>(M_PI_2) * 0.5f, 0.0f);
		//	transform.SetScale(2.0f, 2.0f, 2.0f);
		//}
		//{
		//	renderObjects.emplace_back(std::make_unique<RenderObject>(pDevice, "Patio_Set"));
		//	auto& transform = renderObjects.back()->GetTransform();
		//	transform.Translate(3.0f, -3.49f, -3.5f);
		//	transform.RotateEuler(0.0f, static_cast<float>(M_PI_2) + 0.1f, 0.0f);
		//	transform.SetScale(0.01f, 0.01f, 0.01f);
		//}
	}
	void Update()
	{
		Time::Get().Mark();
		HandleInput();

		pRenderer->SimulateScene();
		pRenderer->DeduceDepth();
		pRenderer->Present();
	}
	void HandleInput()
	{
		if (input.IsKeyPressed(VK_F1)) pRenderer->SetPresentationMode(Renderer::PresentationMode::eColor);
		else if (input.IsKeyPressed(VK_F2)) pRenderer->SetPresentationMode(Renderer::PresentationMode::eSimulatedDepth);
		else if (input.IsKeyPressed(VK_F3)) pRenderer->SetPresentationMode(Renderer::PresentationMode::eOutputDepth);
		else if (input.IsKeyPressed(VK_F4)) pRenderer->CyclePreviewCam();
		if (input.IsKeyPressed(VK_F9)) pRenderer->Screenshot();
		HandleCameraMovement();

		// flush one-frame inputs "pressed" and "released"
		input.kbd.FlushOldInputs();
		input.mouse.FlushOldInputs();
	}
	void HandleCameraMovement()
	{
		const float deltaTime = Time::Get().deltaTime;
		auto& cam = pRenderer->GetCamera();
		auto& camTransform = cam.GetTransform();

		// camera rotation
		constexpr float rotationSpeed = .0025f;
		camTransform.SetRotationEuler(input.GetMousePosY() * rotationSpeed, input.GetMousePosX() * rotationSpeed, .0f);

		// movement
		{
			float speed = 3.0f;
			if (input.IsKeyDown(VK_SHIFT)) speed *= 4.0f;
			if (input.IsKeyDown(VK_CONTROL)) speed *= .5f;
			const float magn = deltaTime * speed;
			if (input.IsKeyDown('W'))
			{
				const auto fwd = camTransform.GetForward();
				camTransform.Translate(fwd.x * magn, fwd.y * magn, fwd.z * magn);
			}
			else if (input.IsKeyDown('S'))
			{
				const auto fwd = camTransform.GetForward();
				camTransform.Translate(fwd.x * -magn, fwd.y * -magn, fwd.z * -magn);
			}
			if (input.IsKeyDown('A'))
			{
				const auto right = camTransform.GetRight();
				camTransform.Translate(right.x * -magn, right.y * -magn, right.z * -magn);
			}
			else if (input.IsKeyDown('D'))
			{
				const auto right = camTransform.GetRight();
				camTransform.Translate(right.x * magn, right.y * magn, right.z * magn);
			}
			if (input.IsKeyDown('Q'))
			{
				const auto up = camTransform.GetUp();
				camTransform.Translate(up.x * -magn, up.y * -magn, up.z * -magn);
			}
			else if (input.IsKeyDown('E'))
			{
				const auto up = camTransform.GetUp();
				camTransform.Translate(up.x * magn, up.y * magn, up.z * magn);
			}
		}
		cam.UpdatePos(pRenderer->GetDeviceContext());
	}

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

	std::unique_ptr<Renderer> pRenderer;
	Input input;
};