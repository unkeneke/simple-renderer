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

	static Matrix createViewportMatrix(int x, int y, int w, int h, int d);
	static Matrix getViewport(int width, int height, int depth);
	static Matrix getProjection(Vec3f& camera);
	static Matrix generateModelView(Vec3f& eye, Vec3f& center, Vec3f& up);
	static Vec2f calculateTriangleCentroid(Vec2i t0, Vec2i t1, Vec2i t2);
	static void drawVectorToPoint(std::vector<Vec2f> linePoints, Vec2f point, TGAImage &image, TGAColor color);
	static void rasterize2dDepthBuffer(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color, int yBuffer[]);
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

#endif //__UTIL_H__
