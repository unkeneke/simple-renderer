#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include "gl_util.h"


class GouraudShader : public IShader {
    // Vec3f varying_intensity; // written by vertex shader, read by fragment shader
    //
    // GouraudShader(Matrix* viewport, Matrix* projection, Matrix* modelView) : IShader(viewport, projection, modelView)  {
    //
    // }
    //
    // virtual Vec4f vertex(int iface, int nthvert) {
    //     varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); // get diffuse lighting intensity
    //     Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
    //     return this->viewport * this->projection * this->modelView * gl_Vertex; // transform it to screen coordinates
    // }
    //
    // virtual bool fragment(Vec3f bar, TGAColor &color) {
    //     float intensity = varying_intensity*bar;   // interpolate intensity for the current pixel
    //     color = TGAColor(255, 255, 255, 255) * intensity; // well duh
    //     return false;                              // no, we do not discard this pixel
    // }
};