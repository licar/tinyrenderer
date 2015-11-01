#pragma once

#include <fstream>
#include <string.h>
#include "geometry.h"

#pragma pack(push,1)
struct TGA_Header {
    char idlength;
    char colormaptype;
    char datatypecode;
    short colormaporigin;
    short colormaplength;
    char colormapdepth;
    short x_origin;
    short y_origin;
    short width;
    short height;
    char  bitsperpixel;
    char  imagedescriptor;
};
#pragma pack(pop)

struct TGAColor {
    unsigned char bgra[4];
    unsigned char bytespp;

    TGAColor() : bgra(), bytespp(1) {
        for (int i=0; i<4; i++) bgra[i] = 0;
    }

    TGAColor(unsigned char R, unsigned char G, unsigned char B, unsigned char A=255) : bgra(), bytespp(4) {
        bgra[0] = B;
        bgra[1] = G;
        bgra[2] = R;
        bgra[3] = A;
    }

    TGAColor(unsigned char v) : bgra(), bytespp(1) {
        for (int i=0; i<4; i++) bgra[i] = 0;
        bgra[0] = v;
    }


    TGAColor(const unsigned char *p, unsigned char bpp) : bgra(), bytespp(bpp) {
        for (int i=0; i<(int)bpp; i++) {
            bgra[i] = p[i];
        }
        for (int i=(int)bpp; i<4; i++) {
            bgra[i] = 0;
        }
    }

    unsigned char& operator[](const int i) { return bgra[i]; }

    TGAColor operator *(float intensity) const {
        TGAColor res = *this;
        intensity = (intensity>1.f?1.f:(intensity<0.f?0.f:intensity));
        for (int i=0; i<4; i++) res.bgra[i] = bgra[i]*intensity;
        return res;
    }
};

class TGAImage
{
public:
    enum Format {
        GRAYSCALE = 1,
        RGB = 3,
        RGBA = 4
    };

    TGAImage();
    TGAImage(int w, int h, Format bpp);
    TGAImage(const TGAImage &img);
    bool read_tga_file(const char *filename);
    bool write_tga_file(const char *filename, bool rle=true);
    bool flip_horizontally();
    bool flip_vertically();
    bool scale(int w, int h);
    ~TGAImage();
    TGAImage & operator =(const TGAImage &img);
    int get_width() const;
    int get_height() const;
    Vec2i get_size() const;
    int get_bytespp();
    unsigned char *buffer();
    void clear();
    TGAColor get(int x, int y);
    bool set(int x, int y, const TGAColor &c);

private:
    bool   load_rle_data(std::ifstream &in);
    bool unload_rle_data(std::ofstream &out);

    inline size_t index(int x, int y) const;

    unsigned char* m_data;
    int m_width;
    int m_height;
    int m_bytespp;
};
