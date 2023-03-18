#ifndef __UTIL_H__
#define __UTIL_H__

#include "geometry.h"

class Util {

public:
	static char* convertWStringToCharPtr(_In_ std::wstring input);
	static Vec2f linearInterpolate(Vec2f v0, Vec2f v1, float t);
	static Vec3f lerp(const Vec3f& start, const Vec3f& end, float t);
	static Vec3f interpolateVectors(const Vec3f& a, const Vec3f& b, const Vec3f& c, float t);
	static Vec3f interpolatePoint(Vec3f *trianglePoints, const Vec3f& p);
	static Vec3f alternativeBarycentric(Vec3f triangleVertexA, Vec3f triangleVertexB, Vec3f triangleVertexC, Vec3f point);
	static Vec3f normalizeVector(Vec3f* pixel, float maxWidth, float maxHeight, float maxDepth, float limit);
};

#endif //__UTIL_H__
