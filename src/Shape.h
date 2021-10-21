#pragma once
#ifndef SHAPE_H
#define SHAPE_H

#include <memory>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>
#include "BlendShape.h"

class MatrixStack;
class Program;

class Shape
{
public:
	Shape();
	virtual ~Shape();
	void loadObj(const std::string &filename, std::vector<float> &pos, std::vector<float> &nor, std::vector<float> &tex, bool loadNor = true, bool loadTex = true);
	void loadMesh(const std::string &meshName);
	void setProgram(std::shared_ptr<Program> p) { prog = p; }
	virtual void init();
	virtual void draw() const;
	void setTextureFilename(const std::string &f) { textureFilename = f; }
	std::string getTextureFilename() const { return textureFilename; }
	
	void drawBlends(float t);
	void setBlends(std::vector< std::shared_ptr<BlendShape>> blendshapes) { blends = blendshapes; }
	void calculateDeltas();
	void sendBlends(float t);
	void initDeltas();

	std::string getFilename() { return meshFilename; }
protected:
	std::vector<std::shared_ptr<BlendShape>> blends;
	std::vector<std::vector<float>> deltas;
	std::string meshFilename;
	std::string textureFilename;
	std::shared_ptr<Program> prog;
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;
	GLuint posBufID;
	GLuint norBufID;
	GLuint texBufID;
	GLuint dPos1BufID;
	GLuint dPos2BufID;
	GLuint dNor1BufID;
	GLuint dNor2BufID;
};

#endif
