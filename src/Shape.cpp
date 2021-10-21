#include <iostream>
#include <fstream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "Shape.h"
#include "GLSL.h"
#include "Program.h"

using namespace std;
using namespace glm;

Shape::Shape() :
	prog(NULL),
	posBufID(0),
	norBufID(0),
	texBufID(0), 
	dPos1BufID(0),
	dPos2BufID(0),
	dNor1BufID(0),
	dNor2BufID(0),
	deltas()
{
}

Shape::~Shape()
{
}

void Shape::loadObj(const string &filename, vector<float> &pos, vector<float> &nor, vector<float> &tex, bool loadNor, bool loadTex)
{
	
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn;
	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str());
	if(!warn.empty()) {
		//std::cout << warn << std::endl;
	}
	if(!err.empty()) {
		std::cerr << err << std::endl;
	}
	if(!ret) {
		return;
	}
	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for(size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			int fv = shapes[s].mesh.num_face_vertices[f];
			// Loop over vertices in the face.
			for(size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				pos.push_back(attrib.vertices[3*idx.vertex_index+0]);
				pos.push_back(attrib.vertices[3*idx.vertex_index+1]);
				pos.push_back(attrib.vertices[3*idx.vertex_index+2]);
				if(!attrib.normals.empty() && loadNor) {
					nor.push_back(attrib.normals[3*idx.normal_index+0]);
					nor.push_back(attrib.normals[3*idx.normal_index+1]);
					nor.push_back(attrib.normals[3*idx.normal_index+2]);
				}
				if(!attrib.texcoords.empty() && loadTex) {
					tex.push_back(attrib.texcoords[2*idx.texcoord_index+0]);
					tex.push_back(attrib.texcoords[2*idx.texcoord_index+1]);
				}
			}
			index_offset += fv;
		}
	}
}

void Shape::loadMesh(const string &meshName)
{
	// Load geometry
	meshFilename = meshName;
	loadObj(meshFilename, posBuf, norBuf, texBuf);
}

void Shape::init()
{
	// Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	
	// Send the normal array to the GPU
	glGenBuffers(1, &norBufID);
	glBindBuffer(GL_ARRAY_BUFFER, norBufID);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
	
	// Send the texcoord array to the GPU
	glGenBuffers(1, &texBufID);
	glBindBuffer(GL_ARRAY_BUFFER, texBufID);
	glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	
	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	GLSL::checkError(GET_FILE_LINE);
}

void Shape::draw() const
{
	assert(prog);

	int h_pos = prog->getAttribute("aPos");
	glEnableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

	int h_nor = prog->getAttribute("aNor");
	glEnableVertexAttribArray(h_nor);
	glBindBuffer(GL_ARRAY_BUFFER, norBufID);
	glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

	int h_tex = prog->getAttribute("aTex");
	glEnableVertexAttribArray(h_tex);
	glBindBuffer(GL_ARRAY_BUFFER, texBufID);
	glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);

	// Draw
	int count = posBuf.size()/3; // number of indices to be rendered
	glDrawArrays(GL_TRIANGLES, 0, count);
	
	glDisableVertexAttribArray(h_tex);
	glDisableVertexAttribArray(h_nor);
	glDisableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLSL::checkError(GET_FILE_LINE);
}

// Code for CPU
void Shape::drawBlends(float t)
{
	if (blends.size() > 1) {
		std::vector<float> currPosBuf;
		std::vector<float> currNorBuf;

		float w1 = fabs(sin(t));
		float w2 = fabs(sin(-2 * t));
		for (int i = 0; i < posBuf.size() / 3; i++) {
			float p1 = posBuf.at(3 * i) + (w1 * (blends.at(0)->getPos(3 * i) - posBuf.at(3 * i))) + (w2 * (blends.at(1)->getPos(3 * i) - posBuf.at(3 * i)));
			currPosBuf.push_back(p1);
			float p2 = posBuf.at(3 * i + 1) + (w1 * (blends.at(0)->getPos(3 * i + 1) - posBuf.at(3 * i + 1))) + (w2 * (blends.at(1)->getPos(3 * i + 1) - posBuf.at(3 * i + 1)));
			currPosBuf.push_back(p2);
			float p3 = posBuf.at(3 * i + 2) + (w1 * (blends.at(0)->getPos(3 * i + 2) - posBuf.at(3 * i + 2))) + (w2 * (blends.at(1)->getPos(3 * i + 2) - posBuf.at(3 * i + 2)));
			currPosBuf.push_back(p3);

			float n1 = norBuf.at(3 * i) + (w1 * (blends.at(0)->getNor(3 * i) - norBuf.at(3 * i))) + (w2 * (blends.at(1)->getNor(3 * i) - norBuf.at(3 * i)));
			float n2 = norBuf.at(3 * i + 1) + (w1 * (blends.at(0)->getNor(3 * i + 1) - norBuf.at(3 * i + 1))) + (w2 * (blends.at(1)->getNor(3 * i + 1) - norBuf.at(3 * i + 1)));
			float n3 = norBuf.at(3 * i + 2) + (w1 * (blends.at(0)->getNor(3 * i + 2) - norBuf.at(3 * i + 2))) + (w2 * (blends.at(1)->getNor(3 * i + 2) - norBuf.at(3 * i + 2)));
			glm::vec3 n = glm::vec3(n1, n2, n3);
			n = glm::normalize(n);
			currNorBuf.push_back(n[0]);
			currNorBuf.push_back(n[1]);
			currNorBuf.push_back(n[2]);
		}
		// Check to see if we are reusing the same buffer, thats what it seems like
		glGenBuffers(1, &posBufID);
		glBindBuffer(GL_ARRAY_BUFFER, posBufID);
		glBufferData(GL_ARRAY_BUFFER, currPosBuf.size() * sizeof(float), &currPosBuf[0], GL_STATIC_DRAW);

		// Send the normal array to the GPU
		glGenBuffers(1, &norBufID);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glBufferData(GL_ARRAY_BUFFER, currNorBuf.size() * sizeof(float), &currNorBuf[0], GL_STATIC_DRAW);
	}
}

void Shape::calculateDeltas()
{
	if (blends.size() > 0) {
		for (int i = 0; i < blends.size(); i++) {
			vector<float> p;
			vector<float> n;
			for (int j = 0; j < posBuf.size(); j++) {
				p.push_back(blends.at(i)->getPos(j) - posBuf.at(j));
				n.push_back(blends.at(i)->getNor(j) - norBuf.at(j));
			}
			// Push back pos and nor deltas for each of the blend shapes we have
			deltas.push_back(p);
			deltas.push_back(n);
		}
	}
}

void Shape::sendBlends(float t)
{
	assert(prog);

	int h_pos = prog->getAttribute("aPos");
	glEnableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

	int h_nor = prog->getAttribute("aNor");
	glEnableVertexAttribArray(h_nor);
	glBindBuffer(GL_ARRAY_BUFFER, norBufID);
	glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

	GLSL::checkError(GET_FILE_LINE);
	int d_pos_1 = prog->getAttribute("dPos1");
	glEnableVertexAttribArray(d_pos_1);
	glBindBuffer(GL_ARRAY_BUFFER, dPos1BufID);
	glVertexAttribPointer(d_pos_1, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

	GLSL::checkError(GET_FILE_LINE);
	int d_pos_2 = prog->getAttribute("dPos2");
	glEnableVertexAttribArray(d_pos_2);
	glBindBuffer(GL_ARRAY_BUFFER, dPos2BufID);
	glVertexAttribPointer(d_pos_2, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

	GLSL::checkError(GET_FILE_LINE);
	int d_nor_1 = prog->getAttribute("dNor1");
	GLSL::checkError(GET_FILE_LINE);
	glEnableVertexAttribArray(d_nor_1);
	GLSL::checkError(GET_FILE_LINE);
	glBindBuffer(GL_ARRAY_BUFFER, dNor1BufID);
	GLSL::checkError(GET_FILE_LINE);
	glVertexAttribPointer(d_nor_1, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

	GLSL::checkError(GET_FILE_LINE);
	int d_nor_2 = prog->getAttribute("dNor2");
	glEnableVertexAttribArray(d_nor_2);
	glBindBuffer(GL_ARRAY_BUFFER, dNor2BufID);
	glVertexAttribPointer(d_nor_2, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

	GLSL::checkError(GET_FILE_LINE);
	int h_tex = prog->getAttribute("aTex");
	glEnableVertexAttribArray(h_tex);
	glBindBuffer(GL_ARRAY_BUFFER, texBufID);
	glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void*)0);

	// Draw
	int count = posBuf.size() / 3; // number of indices to be rendered
	glDrawArrays(GL_TRIANGLES, 0, count);

	glDisableVertexAttribArray(h_tex);
	glDisableVertexAttribArray(h_nor);
	glDisableVertexAttribArray(h_pos);
	glDisableVertexAttribArray(d_pos_1);
	glDisableVertexAttribArray(d_pos_2);
	glDisableVertexAttribArray(d_nor_1);
	glDisableVertexAttribArray(d_nor_2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLSL::checkError(GET_FILE_LINE);
}

// Code for GPU
void Shape::initDeltas()
{
	// Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size() * sizeof(float), &posBuf[0], GL_STATIC_DRAW);

	// Send the delta position 1 array to the GPU
	glGenBuffers(1, &dPos1BufID);
	glBindBuffer(GL_ARRAY_BUFFER, dPos1BufID);
	glBufferData(GL_ARRAY_BUFFER, deltas.at(0).size() * sizeof(float), &deltas.at(0)[0], GL_STATIC_DRAW);

	// Send the delta position 2 array to the GPU
	glGenBuffers(1, &dPos2BufID);
	glBindBuffer(GL_ARRAY_BUFFER, dPos2BufID);
	glBufferData(GL_ARRAY_BUFFER, deltas.at(2).size() * sizeof(float), &deltas.at(2)[0], GL_STATIC_DRAW);

	// Send the normal array to the GPU
	glGenBuffers(1, &norBufID);
	glBindBuffer(GL_ARRAY_BUFFER, norBufID);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size() * sizeof(float), &norBuf[0], GL_STATIC_DRAW);

	// Send the delta normal 1 array to the GPU
	glGenBuffers(1, &dNor1BufID);
	glBindBuffer(GL_ARRAY_BUFFER, dNor1BufID);
	glBufferData(GL_ARRAY_BUFFER, deltas.at(1).size() * sizeof(float), &deltas.at(1)[0], GL_STATIC_DRAW);

	// Send the delta normal 2 array to the GPU
	glGenBuffers(1, &dNor2BufID);
	glBindBuffer(GL_ARRAY_BUFFER, dNor2BufID);
	glBufferData(GL_ARRAY_BUFFER, deltas.at(3).size() * sizeof(float), &deltas.at(3)[0], GL_STATIC_DRAW);

	// Send the texcoord array to the GPU
	glGenBuffers(1, &texBufID);
	glBindBuffer(GL_ARRAY_BUFFER, texBufID);
	glBufferData(GL_ARRAY_BUFFER, texBuf.size() * sizeof(float), &texBuf[0], GL_STATIC_DRAW);

	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLSL::checkError(GET_FILE_LINE);
}
