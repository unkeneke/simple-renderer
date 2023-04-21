#include <iostream>
#include <string>
#include <vector>
#include "gl_util.h"
#include "tgaimage.h"

const TGAColor Util::COLOR_WHITE               = TGAColor(255, 255, 255, 255);
const TGAColor Util::COLOR_RED                 = TGAColor(255, 0,   0,   255);
const TGAColor Util::COLOR_GREEN               = TGAColor(0, 255,   0,   255);
const TGAColor Util::COLOR_BLUE                = TGAColor(0, 0,   255,   255);
const TGAColor Util::COLOR_PURPLE              = TGAColor(255, 0,   255,   255);
const TGAColor Util::COLOR_BACKGROUND_GRADIENT = TGAColor(-1, 0,   0,   255);
const TGAColor Util::COLOR_RANDOM              = TGAColor(-2, 0,   0,   255);
const TGAColor Util::COLOR_TEXTURE             = TGAColor(-3, 0,   0,   255);

Matrix Util::createViewportMatrix(int x, int y, int w, int h, int d) {
    // | w/2  0    0    x+w/2  |
    // | 0    h/2  0    y+h/2  |
    // | 0    0    d/2  d/2    |
    // | 0    0    0    1      |
    Matrix m = Matrix::identity(4);
    m[0][3] = x + w/2.;
    m[1][3] = y + h/2.;
    m[2][3] = d/2.;

    m[0][0] = w/2.;
    m[1][1] = h/2.;
    m[2][2] = d/2.;
    return m;
}


Matrix Util::getViewport(int width, int height, int depth) {
    // With this matrix instead of scaling "by hand" the 3D vector to the screen's resolution
    // we use the matrix to do the same calculation, scale by half the screen
    // then move it to the center of the resulting 2D plane
    float x = width / 8.;
    float y = height / 8.;
    float w = width * 3/4;
    float h = height * 3/4;
    float d = depth;
    Matrix viewport = createViewportMatrix(x, y, w, h, d);

    return viewport;
}

Matrix Util::getProjection(Vec3f& camera) {
    // Then the 4D projection matrix just makes sure that when we go back to 3D
    // the viewport/camera matrix will have the vector's Z axis scale back and forth
    // as we please
    // | 1  0    0    0 |
    // | 0  1    0    0 |
    // | 0  0    1    0 |
    // | 0  0  -1./c  1 |
    Matrix projection = Matrix::identity(4);
    projection[3][2] = -1.f/camera.z;
    return projection;
}

Matrix Util::generateModelView(Vec3f& eye, Vec3f& center, Vec3f& up) {
    Vec3f z = (eye - center).normalize();
    Vec3f x = (up ^ z).normalize();
    Vec3f y = (z ^ x).normalize();
    Matrix Minv = Matrix::identity(4);
    Matrix Tr   = Matrix::identity(4);
    for (int i=0; i < 3; i++) {
        Minv[0][i] = x[i];
        Minv[1][i] = y[i];
        Minv[2][i] = z[i];
        Tr[i][3] = -eye[i];
    }
    return Minv * Tr;
}

Vec2f Util::calculateTriangleCentroid(Vec2i t0, Vec2i t1, Vec2i t2) {
    float xc = (t0.x + t1.x + t2.x) / 3;//* 0.33333333333; // this could be faster than division
    float yc = (t0.y + t1.y + t2.y) / 3;//* 0.33333333333;
    return Vec2f(xc, yc);
}

void Util::drawVectorToPoint(std::vector<Vec2f> linePoints, Vec2f point, TGAImage &image, TGAColor color) {
    for (int i = 0; i < linePoints.size(); i++) {
        // TODO move drawLine to util
        // drawLine(linePoints.at(i).x, linePoints.at(i).y, point.x, point.y, image, color);
    }
}

void Util::rasterize2dDepthBuffer(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color, int yBuffer[]) {
    if (p0.x>p1.x) {
        std::swap(p0, p1);
    }
    for (int x=p0.x; x<=p1.x; x++) {
        float t = (x-p0.x)/(float)(p1.x-p0.x);
        int y = p0.y*(1.-t) + p1.y*t;
        if (yBuffer[x]<y) {
            yBuffer[x] = y;
            image.set(x, 0, color);
        }
    }
}

char* Util::convertWStringToCharPtr(_In_ std::wstring input)
{
    size_t outputSize = input.length() + 1; // +1 for null terminator
    char * outputString = new char[outputSize];
    size_t charsConverted = 0;
    const wchar_t * inputW = input.c_str();
    wcstombs_s(&charsConverted, outputString, outputSize, inputW, input.length());
    return outputString;
}

Vec2f Util::linearInterpolate(Vec2f v0, Vec2f v1, float t) {
    Vec2f v;
    v.x = (1. - t) * v0.x + t * v1.x;
    v.y = (1. - t) * v0.y + t * v1.y;
    return v;
}

Vec3f Util::lerp(const Vec3f& start, const Vec3f& end, float t) {
    Vec3f result;
    result.x = start.x * (1. - t) + end.x * t;// start.x + t * (end.x - start.x);
    result.y = start.y * (1. - t) + end.y * t;// start.y + t * (end.y - start.y);
    return result;
}

Vec3f Util::interpolateVectors(const Vec3f& a, const Vec3f& b, const Vec3f& c, float t) {
    Vec3f interpolatedPoint = lerp(lerp(a, b, t), lerp(b, c, t), t);
    return interpolatedPoint;
}

Vec3f Util::interpolatePoint(Vec3f *trianglePoints, const Vec3f& p) {
    // calculate barycentric coordinates
    double alpha = ((trianglePoints[1].y - trianglePoints[2].y)*(p.x - trianglePoints[2].x) + (trianglePoints[2].x - trianglePoints[1].x)*(p.y - trianglePoints[2].y)) / ((trianglePoints[1].y - trianglePoints[2].y)*(trianglePoints[0].x - trianglePoints[2].x) + (trianglePoints[2].x - trianglePoints[1].x)*(trianglePoints[0].y - trianglePoints[2].y));
    double beta = ((trianglePoints[2].y - trianglePoints[0].y)*(p.x - trianglePoints[2].x) + (trianglePoints[0].x - trianglePoints[2].x)*(p.y - trianglePoints[2].y)) / ((trianglePoints[1].y - trianglePoints[2].y)*(trianglePoints[0].x - trianglePoints[2].x) + (trianglePoints[2].x - trianglePoints[1].x)*(trianglePoints[0].y - trianglePoints[2].y));
    double gamma = 1.0 - alpha - beta;
    
    // interpolate point using barycentric coordinates
    Vec3f result;
    result.x = alpha * trianglePoints[0].x + beta * trianglePoints[1].x + gamma * trianglePoints[2].x;
    result.y = alpha * trianglePoints[0].y + beta * trianglePoints[1].y + gamma * trianglePoints[2].y;
    return result;
}

Vec3f Util::alternativeBarycentric(Vec3f triangleVertexA, Vec3f triangleVertexB, Vec3f triangleVertexC, Vec3f point) {
    Vec3f v0 = triangleVertexB - triangleVertexA;
    Vec3f v1 = triangleVertexC - triangleVertexA;
    Vec3f v2 = point - triangleVertexA;

    // Calculating the Dot products
    float d00 = v0 * v0; 
    float d01 = v0 * v1;
    float d11 = v1 * v1;
    float d20 = v2 * v0;
    float d21 = v2 * v1;
    
    float denom = d00 * d11 - d01 * d01;
    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;
    return Vec3f(u, v, w);
}

Vec3f Util::normalizeVector(Vec3f* pixel, float maxWidth, float maxHeight, float maxDepth, float limit) {
    // zi = (xi – min(x)) / (max(x) – min(x)) * M, normalize between 0 and M
    float x = ((pixel->x - 1.) / (maxWidth - 1.)) * limit;
    float y = ((pixel->y - 1.) / (maxHeight - 1.)) * limit;
    float z = ((pixel->z - 1.) / (maxDepth - 1.)) * limit;
    return Vec3f(x, y, z);
}

Vec2i scaleVector(Vec2i vector) {
    return vector * 4;
}

void Util::drawTriangleByLineSweeping(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) { 
    // sort the vertices, t0, t1, t2 lower−to−upper
    if (t0.y>t1.y) std::swap(t0, t1); 
    if (t0.y>t2.y) std::swap(t0, t2); 
    if (t1.y>t2.y) std::swap(t1, t2);

    int totalHeight = t2.y - t0.y;
	
    for (int i = 0; i < totalHeight; i++) { 
        bool secondHalf = i>t1.y-t0.y || t1.y==t0.y; 
        int segmentHeight = secondHalf ? t2.y-t1.y : t1.y-t0.y; 
        float alpha = (float)i/totalHeight; 
        float beta = (float)(i-(secondHalf ? t1.y-t0.y : 0)) / segmentHeight;
        Vec2i A = t0 + (t2-t0) * alpha; 
        Vec2i B = secondHalf ? t1 + (t2-t1)*beta : t0 + (t1-t0)*beta; 
        if (A.x>B.x) {
            std::swap(A, B);
        }
        for (int j=A.x; j<=B.x; j++) { 
            image.set(j, t0.y+i, color);
        } 
    } 
}

void Util::drawTriangleExamples(TGAImage &image) {
    Vec2i t0[3] = { scaleVector(Vec2i(10, 70)),   scaleVector(Vec2i(50, 160)),  scaleVector(Vec2i(70, 80)) }; 
    Vec2i t1[3] = { scaleVector(Vec2i(180, 50)),  scaleVector(Vec2i(150, 1)),   scaleVector(Vec2i(70, 180)) }; 
    Vec2i t2[3] = { scaleVector(Vec2i(180, 150)), scaleVector(Vec2i(120, 160)), scaleVector(Vec2i(130, 180)) };
    Util::drawTriangleByLineSweeping(t0[0], t0[1], t0[2], image, COLOR_RED);
    Util::drawTriangleByLineSweeping(t1[0], t1[1], t1[2], image, COLOR_GREEN);
    Util::drawTriangleByLineSweeping(t2[0], t2[1], t2[2], image, COLOR_WHITE);
}



