#include <iostream>
#include <fstream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "tiny_obj_loader.h"

#include "BlendShape.h"
#include "GLSL.h"
#include "Program.h"

using namespace std;
using namespace glm;

BlendShape::BlendShape():
	prog(NULL),
	posBufID(0),
	norBufID(0),
	texBufID(0)
{
}

BlendShape::~BlendShape()
{
}

void BlendShape::loadObj(const string& filename, vector<float>& pos, vector<float>& nor, vector<float>& tex, bool loadNor, bool loadTex)
{

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn;
	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str());
	if (!warn.empty()) {
		//std::cout << warn << std::endl;
	}
	if (!err.empty()) {
		std::cerr << err << std::endl;
	}
	if (!ret) {
		return;
	}
	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			int fv = shapes[s].mesh.num_face_vertices[f];
			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				pos.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
				pos.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
				pos.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
				if (!attrib.normals.empty() && loadNor) {
					nor.push_back(attrib.normals[3 * idx.normal_index + 0]);
					nor.push_back(attrib.normals[3 * idx.normal_index + 1]);
					nor.push_back(attrib.normals[3 * idx.normal_index + 2]);
				}
				if (!attrib.texcoords.empty() && loadTex) {
					tex.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
					tex.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
				}
			}
			index_offset += fv;
		}
	}
}

void BlendShape::loadMesh(const string& meshName)
{
	// Load geometry
	meshFilename = meshName;
	loadObj(meshFilename, posBuf, norBuf, texBuf);
}