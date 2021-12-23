#pragma once

// TODO: pack this into .cpp to hide from codebase
#define TINYOBJLOADER_IMPLEMENTATION
//#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include "tinyobjloader/tiny_obj_loader.h"
#include "Submesh.hpp"

enum class Primitive { Cube, Sphere, Quad };
class Mesh
{
public:
	Mesh(ID3D11Device* const pDevice, Primitive primitive) 
	{
		submeshPtrs.emplace_back(std::make_unique<Submesh>());

		switch (primitive)
		{
			case Primitive::Cube: submeshPtrs.front()->SetAsCube(pDevice); break;
			case Primitive::Sphere: submeshPtrs.front()->SetAsSphere(pDevice); break;
			case Primitive::Quad: submeshPtrs.front()->SetAsQuad(pDevice); break;
		}

		submeshPtrs.front()->CreateBuffer(pDevice);
	}
	Mesh(ID3D11Device* const pDevice, std::string fileName) {

		LoadObj(pDevice, fileName);

		for (auto cur = submeshPtrs.begin(), end = submeshPtrs.end(); cur < end; cur++) {
			(*cur)->CreateBuffer(pDevice);
		}
	}
	~Mesh() = default;
	ROF_DELETE(Mesh);

public:
	void Draw(ID3D11DeviceContext* const pDeviceContext)
	{
		for (auto cur = submeshPtrs.begin(), end = submeshPtrs.end(); cur < end; cur++) {
			(*cur)->Draw(pDeviceContext);
		}
	}
private:
	void LoadObj(ID3D11Device* const pDevice, std::string fileName) 
	{
		std::ostringstream oss;
		oss << "data/objs/" << fileName << "/";
		std::string filePath = oss.str();

		tinyobj::ObjReaderConfig readerConfig;
		readerConfig.mtl_search_path = filePath; // Path to material files
		readerConfig.triangulate = false;

		tinyobj::ObjReader reader;
		oss << fileName << ".obj";
		bool success = reader.ParseFromFile(oss.str(), readerConfig);
		const std::string& error = reader.Error();
		const std::string& warning = reader.Warning();
		if (!success) throw std::runtime_error(error);


		auto& attrib = reader.GetAttrib();
		auto& shapes = reader.GetShapes();
		auto& materials = reader.GetMaterials();

		// TODO: reserve space for vertices, indices

		submeshPtrs.reserve(shapes.size());

		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++) {
			submeshPtrs.emplace_back(std::make_unique<Submesh>());

			// this assumes that each submesh/shape has a unique material assigned to it
			if (!materials[s].diffuse_texname.empty()) {
				submeshPtrs.back()->texturePack.diffuse_tex = std::make_unique<Texture2D>();
				submeshPtrs.back()->texturePack.diffuse_tex->CreateTextureJPG(pDevice, s2ws(filePath + materials[s].diffuse_texname));
			}

			// Loop over faces(polygon)
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++) {

					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					Vertex vertex = {};

					vertex.pos.x = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
					vertex.pos.y = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
					vertex.pos.z = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
					vertex.pos.w = 1.0f;

					// Check if `normal_index` is zero or positive. negative = no normal data
					if (idx.normal_index >= 0) {
						vertex.norm.x = attrib.normals[3 * size_t(idx.normal_index) + 0];
						vertex.norm.y = attrib.normals[3 * size_t(idx.normal_index) + 1];
						vertex.norm.z = attrib.normals[3 * size_t(idx.normal_index) + 2];
						vertex.norm.w = 0.0f;
					}

					// Check if `texcoord_index` is zero or positive. negative = no texcoord data
					if (idx.texcoord_index >= 0) {
						// TODO
						tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
						tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
					}

					// Optional: vertex colors
					vertex.col = { 1.0f, 1.0f, 1.0f, 1.0f };
					// tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
					// tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
					// tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];

					submeshPtrs.back()->vertices.push_back(vertex);
					submeshPtrs.back()->indices.push_back(static_cast<Index>(submeshPtrs.back()->vertices.size()) - 1); // duplicate array access!
				}
				index_offset += fv;

				// per-face material
				shapes[s].mesh.material_ids[f];
			}
		}
	}

private:
	std::vector<std::unique_ptr<Submesh>> submeshPtrs;
};