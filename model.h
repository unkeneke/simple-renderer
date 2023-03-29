#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec3f> vertTextures_;
	std::vector<std::vector<std::vector<int>> > faces_;
public:
	Model(const char *filename);
	~Model();
	int getTotalVertices();
	int getTotalFaces();
	int getTotalTextureVertices();
	Vec3f getVertexByIndex(int i);
	std::vector<std::vector<int>> getFaceByIndex(int idx);
	Vec3f getTextureVertexByIndex(int i);
};

#endif //__MODEL_H__
