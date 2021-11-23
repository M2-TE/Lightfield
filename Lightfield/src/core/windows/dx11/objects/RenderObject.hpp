#pragma once

#include "Transform.hpp"

class RenderObject
{
public:
	RenderObject() = default;
	~RenderObject() = default;
	ROF_DELETE(RenderObject);

private:
	Transform transform;
};