#pragma once

#define TINYOBJLOADER_IMPLEMENTATION
//#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include "tinyobjloader/tiny_obj_loader.h"

class Model
{
public:
	Model(std::string fileName)
	{
		tinyobj::ObjReaderConfig readerConfig;
		readerConfig.mtl_search_path = "./data/objs"; // Path to material files

		tinyobj::ObjReader reader;

		if (!reader.ParseFromFile(fileName, readerConfig)) {
			if (!reader.Error().empty()) {
				DebugBreak();
				//std::cerr << "TinyObjReader: " << reader.Error();
			}
		}

		if (!reader.Warning().empty()) {
			std::cout << "TinyObjReader: " << reader.Warning();
		}

		//auto& attrib = reader.GetAttrib();
		//auto& shapes = reader.GetShapes();
		//auto& materials = reader.GetMaterials();


	}
};