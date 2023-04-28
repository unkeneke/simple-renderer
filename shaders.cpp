#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include "shaders.h"


    
Vec4f GouraudShader::vertex(int iface, int nthvert) {
    // varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); // get diffuse lighting intensity
    // Vec4f gl_Vertex = std::embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
    // return this->viewport * this->projection * this->modelView * gl_Vertex; // transform it to screen coordinates

    return Vec4f();
}
    
bool GouraudShader::fragment(Vec3f bar, TGAColor &color) {
    float intensity = varying_intensity*bar;   // interpolate intensity for the current pixel
    color = TGAColor(255, 255, 255, 255) * intensity; // well duh
    return false;                              // no, we do not discard this pixel
}