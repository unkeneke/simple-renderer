#ifndef __UTIL_H__
#define __UTIL_H__

#include "geometry.h"
#include "tgaimage.h"

class Util {

	

public:
	static const TGAColor COLOR_WHITE;
	static const TGAColor COLOR_RED;
	static const TGAColor COLOR_GREEN;
	static const TGAColor COLOR_BLUE;
	static const TGAColor COLOR_PURPLE;
	static const TGAColor COLOR_BACKGROUND_GRADIENT;
	static const TGAColor COLOR_RANDOM;
	static const TGAColor COLOR_TEXTURE;

	Vec2f calculateTriangleCentroid(Vec2i t0, Vec2i t1, Vec2i t2);
	void drawVectorToPoint(std::vector<Vec2f> linePoints, Vec2f point, TGAImage &image, TGAColor color);
	void rasterize2dDepthBuffer(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color, int yBuffer[]);
	static char* convertWStringToCharPtr(_In_ std::wstring input);
	static Vec2f linearInterpolate(Vec2f v0, Vec2f v1, float t);
	static Vec3f lerp(const Vec3f& start, const Vec3f& end, float t);
	static Vec3f interpolateVectors(const Vec3f& a, const Vec3f& b, const Vec3f& c, float t);
	static Vec3f interpolatePoint(Vec3f *trianglePoints, const Vec3f& p);
	static Vec3f alternativeBarycentric(Vec3f triangleVertexA, Vec3f triangleVertexB, Vec3f triangleVertexC, Vec3f point);
	static Vec3f normalizeVector(Vec3f* pixel, float maxWidth, float maxHeight, float maxDepth, float limit);
	static void drawTriangleByLineSweeping(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color);
	static void drawTriangleExamples(TGAImage &image);
};

struct IShader {
	virtual ~IShader();
	virtual Vec3i vertex(int iface, int nthvert) = 0;
	virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

#endif //__UTIL_H__
