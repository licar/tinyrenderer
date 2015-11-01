#pragma once

#include "geometry.h"
#include "tgaimage.h"

class FrameTile
{
public:
    explicit FrameTile(Vec2i origin, Vec2i size);
    void init(TGAImage &image, float *zbuffer);

    TGAColor get(int x, int y) const;
    void set(int x, int y, const TGAColor &c);

    int get_top() const;
    int get_left() const;
    int get_right() const;
    int get_bottom() const;

    float get_z(int x, int y) const;
    void set_z(int x, int y, float z);

private:
    inline size_t index(int x, int y) const;

    Vec2i m_origin;
    Vec2i m_size;
    Vec2i m_imageSize;
    unsigned char* m_data = nullptr;
    int m_bytespp = 0;
    float *m_zbuffer = nullptr;
};
