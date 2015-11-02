#include "shader.h"
#include "model.h"

Vec4f Shader::vertex(int iface, int nthvert)
{
    varying_uv.set_col(nthvert, pModel->uv(iface, nthvert));
    varying_nrm.set_col(nthvert, proj<3>((Projection*ModelView).invert_transpose()*embed<4>(pModel->normal(iface, nthvert), 0.f)));
    Vec4f gl_Vertex = Projection*ModelView*embed<4>(pModel->vert(iface, nthvert));
    varying_tri.set_col(nthvert, gl_Vertex);
    return gl_Vertex;
}

bool Shader::fragment(Vec3f bar, TGAColor &color)
{
    Vec3f bn = (varying_nrm*bar).normalize();
    Vec2f uv = varying_uv*bar;
    mat<3,3,float> ndc_tri; // column-vectors
    for (int i=0; i<3; i++) ndc_tri.set_col(i, proj<3>(varying_tri.col(i)/varying_tri.col(i)[3]));
    mat<3,3,float> A;
    A[0] = ndc_tri.col(1)-ndc_tri.col(0);
    A[1] = ndc_tri.col(2)-ndc_tri.col(0);
    A[2] = bn;
    A = A.invert();
    Vec3f bu = A*Vec3f(varying_uv[0][1]-varying_uv[0][0], varying_uv[0][2]-varying_uv[0][0], 0);
    Vec3f bv = A*Vec3f(varying_uv[1][1]-varying_uv[1][0], varying_uv[1][2]-varying_uv[1][0], 0);
    mat<3,3,float> B;
    B.set_col(0, bu.normalize());
    B.set_col(1, bv.normalize());
    B.set_col(2, bn);

    Vec3f n = (B*pModel->normal(uv)).normalize();

    float intensity = n*light_dir;
    float diff = std::max<float>(0.f, intensity + 0.1 * pow(intensity, 10));
    color = TGAColor(255, 255, 255)*diff;
    return false;
}

void Shader::setLightDirection(Vec3f &direction)
{
    light_dir = proj<3>((Projection*ModelView*embed<4>(direction, 0.f))).normalize();
}
