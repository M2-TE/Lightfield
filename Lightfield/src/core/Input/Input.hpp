#pragma once

#include "Keyboard.hpp"
#include "Mouse.hpp"

class Input
{
	friend class Application;

public:
	Input() : kbd(), mouse(), hWnd(), rawInputBuffer(),
		isCursorConfined(false), isCursorVisible(true), isWindowActive(true)
	{

	}
	ROF_DELETE(Input);

	// Pressed checks if the key was pressed during this frame
	// Down checks if the key is currently down
	// Released checks if the key was released during this frame

	inline bool IsKeyPressed(unsigned char keycode) const noexcept
	{
		return kbd.keysPressed.find(keycode) != kbd.keysPressed.end();
	}
	inline bool IsKeyDown(unsigned char keycode) const noexcept
	{
		return kbd.keysDown.find(keycode) != kbd.keysDown.end();
	}
	inline bool IsKeyReleased(unsigned char keycode) const noexcept
	{
		return kbd.keysReleased.find(keycode) != kbd.keysReleased.end();
	}

	inline bool IsMouseButtonPressed(unsigned char buttoncode) const noexcept
	{
		return mouse.buttonsPressed.find(buttoncode) != mouse.buttonsPressed.end();
	}
	inline bool IsMouseButtonDown(unsigned char buttoncode) const noexcept
	{
		return mouse.buttonsDown.find(buttoncode) != mouse.buttonsDown.end();
	}
	inline bool IsMouseButtonReleased(unsigned char buttoncode) const noexcept
	{
		return mouse.buttonsReleased.find(buttoncode) != mouse.buttonsReleased.end();
	}

	inline int GetMousePosX() const noexcept
	{
		return mouse.xPos;
	}
	inline int GetMousePosY() const noexcept
	{
		return mouse.yPos;
	}

	inline int GetMouseDeltaX() const
	{
		return mouse.xDelta;
	}
	inline int GetMouseDeltaY() const
	{
		return mouse.yDelta;
	}

	inline int GetMouseWheelDeltaRaw() const noexcept
	{
		return mouse.wheelDeltaRaw;
	}

private:
	inline void RegisterRawInput(LPARAM lParam)
	{
		// aquire size of raw input in bytes
		UINT size = 0u;
		if (GetRawInputData(
			reinterpret_cast<HRAWINPUT>(lParam),
			RID_INPUT,
			nullptr,
			&size,
			sizeof(RAWINPUTHEADER)) == -1)
		{
			return;
		}

		// fill buffer
		rawInputBuffer.resize(size);
		if (GetRawInputData(
			reinterpret_cast<HRAWINPUT>(lParam),
			RID_INPUT,
			rawInputBuffer.data(),
			&size,
			sizeof(RAWINPUTHEADER)) != size)
		{
			return;
		}

		// read raw input from buffer
		auto& ri = reinterpret_cast<const RAWINPUT&>(*rawInputBuffer.data());
		switch (ri.header.dwType)
		{
			case RIM_TYPEMOUSE:
				mouse.RegisterMouseDelta(ri.data.mouse.lLastX, ri.data.mouse.lLastY);
				break;

			case RIM_TYPEKEYBOARD:
				// keyboard input
				break;
		}
	}

	// cursor-specific
public:
	inline void SetCursorVisibility(bool isVisible)
	{
		isCursorVisible = isVisible;
		UpdateCursorVisibility();
	}
	inline void SetCursorConfinement(bool isConfined)
	{
		isCursorConfined = isConfined;
		UpdateCursorConfinement();
	}
	inline bool GetCursorVisibility() const
	{
		return isCursorVisible;
	}
	inline bool GetCursorConfinement() const
	{
		return isCursorConfined;
	}

private:
	void UpdateCursorConfinement()
	{
		if (isWindowActive)
		{
			if (isCursorConfined)
			{
				RECT rect;
				GetClientRect(hWnd, &rect);
				MapWindowPoints(hWnd, nullptr, reinterpret_cast<POINT*>(&rect), 2);
				ClipCursor(&rect);
			}
			else
			{
				ClipCursor(nullptr);
			}
		}
		else
		{
			ClipCursor(nullptr);
		}
	}
	void UpdateCursorVisibility()
	{
		if (isWindowActive)
		{
			if (isCursorVisible)
			{
				while (ShowCursor(TRUE) < 0);
			}
			else
			{
				while (ShowCursor(FALSE) >= 0);
			}
		}
		else
		{
			// show cursor while window is not in focus
			while (ShowCursor(TRUE) < 0);
		}
	}

private:
	HWND hWnd;
	Keyboard kbd;
	Mouse mouse;

	std::vector<char> rawInputBuffer;
	bool isCursorConfined;
	bool isCursorVisible;
	bool isWindowActive;
};