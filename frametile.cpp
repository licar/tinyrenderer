#include "frametile.h"
#include "tgaimage.h"
#include <string.h>

FrameTile::FrameTile(Vec2i origin, Vec2i size)
    : m_origin(origin)
    , m_size(size)
{
}

void FrameTile::init(TGAImage &image, float *zbuffer)
{
    m_imageSize = image.get_size();
    m_data = image.buffer();
    m_bytespp = image.get_bytespp();
    m_zbuffer = zbuffer;
}

TGAColor FrameTile::get(int x, int y) const
{
    return TGAColor(m_data + m_bytespp * index(x, y), m_bytespp);
}

void FrameTile::set(int x, int y, const TGAColor &c)
{
    memcpy(m_data + m_bytespp * index(x, y), c.bgra, m_bytespp);
}

int FrameTile::get_top() const
{
    return m_origin.y;
}

int FrameTile::get_left() const
{
    return m_origin.x;
}

int FrameTile::get_right() const
{
    return m_origin.x + m_size.x;
}

int FrameTile::get_bottom() const
{
    return m_origin.y + m_size.y;
}

float FrameTile::get_z(int x, int y) const
{
    return m_zbuffer[index(x, y)];
}

void FrameTile::set_z(int x, int y, float z)
{
    m_zbuffer[index(x, y)] = z;
}

size_t FrameTile::index(int x, int y) const
{
    return x + y * m_imageSize.x;
}
