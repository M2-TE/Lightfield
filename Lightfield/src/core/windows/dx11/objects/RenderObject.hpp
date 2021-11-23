#pragma once

#include "Transform.hpp"
#include "Mesh.hpp"

class RenderObject
{
public:
	RenderObject(ID3D11Device* const pDevice, Primitive primitive) : transform(pDevice), mesh(pDevice, primitive) {}
	~RenderObject() = default;
	ROF_DELETE(RenderObject);

public:
	inline Transform& GetTransform() { return transform; }
	inline void Draw(ID3D11DeviceContext* const pDeviceContext) { mesh.Draw(pDeviceContext); }

private:
	Transform transform;
	Mesh mesh;
};