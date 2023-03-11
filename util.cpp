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





