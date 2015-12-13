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
#include <SDL.h>

const int WIDTH  = 800;
const int HEIGHT = 800;

Vec3f LIGHT_DIR(1,1,1);
Vec3f       EYE(1,1,3);
Vec3f    CENTER(0,0,0);
Vec3f        UP(0,1,0);

typedef std::shared_ptr<Model> ModelPtr;
typedef std::vector<ModelPtr> ModelPtrArray;

Vec3f get_rotated_eye()
{
    float secondsSinceStart = 0.001f * float(SDL_GetTicks());
    float angle = 0.5f * secondsSinceStart;
    Matrix rotation = Matrix::identity();
    rotation[0][0] = cos(angle);
    rotation[1][0] = -sin(angle);
    rotation[1][1] = cos(angle);
    rotation[0][1] = sin(angle);
    Vec4f rotated = rotation * embed<4>(EYE, 1.f);
    return proj<3>(rotated);
}

void draw_3d_model_tile(Model &model, FrameTile &frame)
{
    Shader shader;
    shader.setLightDirection(LIGHT_DIR);
    shader.pModel = &model;
    for (int i=0; i<model.nfaces(); i++) {
        for (int j=0; j<3; j++) {
            shader.vertex(i, j);
        }
        triangle(shader.varying_tri, shader, frame);
    }
}

void draw_3d_model_simple(ModelPtrArray const& models, TGAImage &frame, float *zbuffer)
{
    const int width1 = frame.get_width() / 2;
    const int width2 = frame.get_width() - width1;
    const int height1 = frame.get_height() / 2;
    const int height2 = frame.get_height() - height1;

    FrameTile tile1(Vec2i(0, 0), Vec2i(width1, height1));
    FrameTile tile2(Vec2i(width1, 0), Vec2i(width2, height1));
    FrameTile tile3(Vec2i(0, height1), Vec2i(width2, height2));
    FrameTile tile4(Vec2i(width1, height1), Vec2i(width2, height2));
    tile1.init(frame, zbuffer);
    tile2.init(frame, zbuffer);
    tile3.init(frame, zbuffer);
    tile4.init(frame, zbuffer);
    for (auto const& pModel : models) {
        draw_3d_model_tile(*pModel, tile1);
        draw_3d_model_tile(*pModel, tile2);
        draw_3d_model_tile(*pModel, tile3);
        draw_3d_model_tile(*pModel, tile4);
    }
}

int main(int argc, char** argv) {
    if (2>argc) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }
    ModelPtrArray models;
    for (int m=1; m<argc; m++) {
        const char *filePath = argv[m];
        models.push_back(std::make_shared<Model>(filePath));
    }

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    std::unique_ptr<float[]> zbuffer(new float[WIDTH*HEIGHT]);

    SDLWindow window(WIDTH, HEIGHT);
    std::shared_ptr<TGAImage> pFrame;
    window.swapBuffers(pFrame);
    window.do_on_idle([&]() {
        pFrame->clear();
        for (int i=WIDTH*HEIGHT; i--; zbuffer[i] = -std::numeric_limits<float>::max());
        Vec3f eye = get_rotated_eye();
        lookat(eye, CENTER, UP);
        viewport(WIDTH/8, HEIGHT/8, WIDTH*3/4, HEIGHT*3/4);
        projection(-1.f/(eye-CENTER).norm());
        draw_3d_model_simple(models, *pFrame, zbuffer.get());
        pFrame->flip_vertically(); // to place the origin in the bottom left corner of the image
        window.swapBuffers(pFrame);
    });
    window.show();
    window.wait_for_closed();

    return 0;
}

