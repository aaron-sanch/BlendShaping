#pragma once
#ifndef BLENDSHAPE_H
#define BLENDSHAPE_H

#include <memory>
#include <vector>
#include <string>

#define GLEW_STATIC
#include <GL/glew.h>

class MatrixStack;
class Program;

class BlendShape
{
public:
	BlendShape();
	virtual ~BlendShape();
	void loadObj(const std::string& filename, std::vector<float>& pos, std::vector<float>& nor, std::vector<float>& tex, bool loadNor = true, bool loadTex = true);
	void loadMesh(const std::string& meshName);

	GLuint* getpBufID() { return &posBufID; }
	GLuint* getnBufID() { return &norBufID; }
	GLuint* gettBufID() { return &texBufID; }

	std::vector<float>& getPBuf() { return posBuf; }
	std::vector<float>& getNBuf() { return norBuf; }
	std::vector<float>& getTBuf() { return texBuf; }

	float& getPos(const int& i) { return posBuf.at(i); }
	float& getNor(const int& i) { return norBuf.at(i); }

protected:
	std::string meshFilename;
	std::string textureFilename;
	std::shared_ptr<Program> prog;
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;
	GLuint posBufID;
	GLuint norBufID;
	GLuint texBufID;
};

#endif
