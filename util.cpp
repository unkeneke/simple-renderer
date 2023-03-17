#include <iostream>
#include <string>
#include "util.h"

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





