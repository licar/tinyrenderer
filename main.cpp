#include <vector>
#include <limits>
#include <memory>
#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"
#include "shader.h"
#include "sdlwindow.h"
#include <SDL2/SDL.h>

const int WIDTH  = 800;
const int HEIGHT = 800;

Vec3f LIGHT_DIR(1,1,1);
Vec3f       EYE(1,1,3);
Vec3f    CENTER(0,0,0);
Vec3f        UP(0,1,0);

void draw_3d_model(Model &model, TGAImage &frame, float *zbuffer)
{
    Shader shader;
    shader.setLightDirection(LIGHT_DIR);
    shader.pModel = &model;
    for (int i=0; i<model.nfaces(); i++) {
        for (int j=0; j<3; j++) {
            shader.vertex(i, j);
        }
        triangle(shader.varying_tri, shader, frame, zbuffer);
    }
}

int main(int argc, char** argv) {
    if (2>argc) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    uint32_t startTime = SDL_GetTicks();

    std::unique_ptr<float[]> zbuffer(new float[WIDTH*HEIGHT]);
    for (int i=WIDTH*HEIGHT; i--; zbuffer[i] = -std::numeric_limits<float>::max());

    auto pFrame = std::make_shared<TGAImage>(WIDTH, HEIGHT, TGAImage::RGB);
    lookat(EYE, CENTER, UP);
    viewport(WIDTH/8, HEIGHT/8, WIDTH*3/4, HEIGHT*3/4);
    projection(-1.f/(EYE-CENTER).norm());

    for (int m=1; m<argc; m++) {
        std::unique_ptr<Model> pModel(new Model(argv[m]));
        draw_3d_model(*pModel, *pFrame, zbuffer.get());
    }
    pFrame->flip_vertically(); // to place the origin in the bottom left corner of the image
    pFrame->write_tga_file("framebuffer.tga");

    char title[1024];
    sprintf(title, "Frame generated in %d milliseconds", SDL_GetTicks() - startTime);
    SDLWindow window(pFrame, title);
    window.show();

    return 0;
}

