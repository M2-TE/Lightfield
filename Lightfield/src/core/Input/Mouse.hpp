#pragma once

#include <set>

namespace MouseButtons
{
	constexpr unsigned char LEFT = 0;
	constexpr unsigned char RIGHT = 1;
	constexpr unsigned char MIDDLE = 2;
	//inline constexpr unsigned char MOUSEBUTTON_EXTRA1	= 3;
	//inline constexpr unsigned char MOUSEBUTTON_EXTRA2	= 4;

	//inline constexpr unsigned char MOUSEBUTTON_LEFT = 0b0001;
	//inline constexpr unsigned char MOUSEBUTTON_RIGHT = 0b0010;
	//inline constexpr unsigned char MOUSEBUTTON_MIDDLE = 0b0100;
	////inline constexpr unsigned char MOUSEBUTTON_EXTRA1	= 0b1000;
	////inline constexpr unsigned char MOUSEBUTTON_EXTRA2	= 0x0010;
}

class Mouse
{
public:
	Mouse() : xPos(0), yPos(0),
		xDelta(0), yDelta(0),
		wheelDeltaRaw(0)
	{
		RAWINPUTDEVICE rid = {};
		rid.usUsagePage = 0x01;
		rid.usUsage = 0x02;
		rid.dwFlags = 0;
		rid.hwndTarget = nullptr;
		if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE)
		{
			std::runtime_error("Mouse registration failed");
		}
	}

	inline void RegisterMouseDelta(int x, int y)
	{
		xPos += x;
		yPos += y;
		xDelta = x;
		yDelta = y;
	}
	inline void RegisterMouseWheelDelta(int delta) noexcept
	{
		wheelDeltaRaw += delta;
	}

	inline void RegisterMouseButtonPress(unsigned char buttonCode) noexcept
	{
		buttonsPressed.insert(buttonCode);
		buttonsDown.insert(buttonCode);
	}
	inline void RegisterMouseButtonRelease(unsigned char buttonCode) noexcept
	{
		buttonsReleased.insert(buttonCode);
		buttonsDown.erase(buttonCode);
	}

	inline void FlushOldInputs()
	{
		buttonsPressed.clear();
		buttonsReleased.clear();
		wheelDeltaRaw = 0;
		xDelta = 0;
		yDelta = 0;
	}
	inline void FlushAll()
	{
		buttonsPressed.clear();
		buttonsDown.clear();
		buttonsReleased.clear();
		wheelDeltaRaw = 0;
	}

public:
	int	xPos, yPos;
	int xDelta, yDelta;
	int	wheelDeltaRaw;

	std::set<unsigned char>	buttonsPressed;
	std::set<unsigned char>	buttonsDown;
	std::set<unsigned char>	buttonsReleased;
};