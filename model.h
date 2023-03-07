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
	int nverts();
	int nfaces();
	int nvertTextures();
	Vec3f vert(int i);
	std::vector<std::vector<int>> face(int idx);
	Vec3f vertTexture(int i);
};

#endif //__MODEL_H__
