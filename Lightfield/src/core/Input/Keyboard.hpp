#pragma once

#include <queue>
#include <set>

class Keyboard
{
public:
	Keyboard() = default;

	inline void RegisterKeyPress(unsigned char keycode) noexcept
	{
		keysPressed.insert(keycode);
		keysDown.insert(keycode);
	}
	inline void RegisterKeyRelease(unsigned char keycode) noexcept
	{
		keysReleased.insert(keycode);
		keysDown.erase(keycode);
	}
	inline void RegisterChar(char character) noexcept
	{
		charBuffer.push(character);

		if (charBuffer.size() > bufferSize)
		{
			charBuffer.pop();
		}
	}

	inline void FlushOldInputs()
	{
		keysPressed.clear();
		keysReleased.clear();

		// clear queue
		std::queue<char>().swap(charBuffer);
	}
	inline void FlushAll()
	{
		keysPressed.clear();
		keysDown.clear();
		keysReleased.clear();

		// clear queue
		std::queue<char>().swap(charBuffer);
	}

public:
	std::set<char> keysPressed;
	std::set<char> keysDown;
	std::set<char> keysReleased;

	std::queue<char> charBuffer;

private:
	static constexpr unsigned int bufferSize = 16u;
};