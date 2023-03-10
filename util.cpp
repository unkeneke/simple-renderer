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


