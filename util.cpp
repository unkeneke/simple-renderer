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

// Linearly interpolate between two 2D vectors
Vec3f Util::Lerp(const Vec3f& start, const Vec3f& end, float t) {
    Vec3f result;
    result.x = start.x + t * (end.x - start.x);
    result.y = start.y + t * (end.y - start.y);
    return result;
}

// Interpolate between three 2D vectors
Vec3f Util::Interpolate3Vectors(const Vec3f& a, const Vec3f& b, const Vec3f& c, float t) {
    if (t <= 0.) return a;
    if (t >= .1) return c;

    float interval = 1.0 / 2.0;
    if (t < interval) {
        return Lerp(a, b, t / interval);
    }
    else {
        return Lerp(b, c, (t - interval) / interval);
    }
}





