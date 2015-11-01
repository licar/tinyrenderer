#pragma once
#include "our_gl.h"

class Model;

struct Shader : public IShader {
    mat<2,3,float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
    mat<4,3,float> varying_tri; // triangle coordinates (clip coordinates), written by VS, read by FS
    mat<3,3,float> varying_nrm; // normal per vertex to be interpolated by FS
    Vec3f light_dir;
    Model *pModel = nullptr;

    Vec4f vertex(int iface, int nthvert) override;
    bool fragment(Vec3f bar, TGAColor &color) override;

    void setLightDirection(Vec3f &direction);
};
