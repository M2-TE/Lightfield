#pragma once

#include "Transform.hpp"
#include "Mesh.hpp"

class RenderObject
{
public:
	RenderObject(ID3D11Device* const pDevice, Primitive primitive) : transform(pDevice), mesh(pDevice, primitive) {}
	~RenderObject() = default;
	ROF_DELETE(RenderObject);

private:
	Transform transform;
	Mesh mesh;
};