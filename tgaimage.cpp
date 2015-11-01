#include <iostream>
#include <fstream>
#include <time.h>
#include <math.h>
#include "tgaimage.h"

TGAImage::TGAImage() : m_data(NULL), m_width(0), m_height(0), m_bytespp(0) {}

TGAImage::TGAImage(int w, int h, Format bpp) : m_data(NULL), m_width(w), m_height(h), m_bytespp(bpp) {
    unsigned long nbytes = m_width*m_height*m_bytespp;
    m_data = new unsigned char[nbytes];
    memset(m_data, 0, nbytes);
}

TGAImage::TGAImage(const TGAImage &img) : m_data(NULL), m_width(img.m_width), m_height(img.m_height), m_bytespp(img.m_bytespp) {
    unsigned long nbytes = m_width*m_height*m_bytespp;
    m_data = new unsigned char[nbytes];
    memcpy(m_data, img.m_data, nbytes);
}

TGAImage::~TGAImage() {
    if (m_data) delete [] m_data;
}

TGAImage & TGAImage::operator =(const TGAImage &img) {
    if (this != &img) {
        if (m_data) delete [] m_data;
        m_width  = img.m_width;
        m_height = img.m_height;
        m_bytespp = img.m_bytespp;
        unsigned long nbytes = m_width*m_height*m_bytespp;
        m_data = new unsigned char[nbytes];
        memcpy(m_data, img.m_data, nbytes);
    }
    return *this;
}

bool TGAImage::read_tga_file(const char *filename) {
    if (m_data) delete [] m_data;
    m_data = NULL;
    std::ifstream in;
    in.open (filename, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "can't open file " << filename << "\n";
        in.close();
        return false;
    }
    TGA_Header header;
    in.read((char *)&header, sizeof(header));
    if (!in.good()) {
        in.close();
        std::cerr << "an error occured while reading the header\n";
        return false;
    }
    m_width   = header.width;
    m_height  = header.height;
    m_bytespp = header.bitsperpixel>>3;
    if (m_width<=0 || m_height<=0 || (m_bytespp!=GRAYSCALE && m_bytespp!=RGB && m_bytespp!=RGBA)) {
        in.close();
        std::cerr << "bad bpp (or width/height) value\n";
        return false;
    }
    unsigned long nbytes = m_bytespp*m_width*m_height;
    m_data = new unsigned char[nbytes];
    if (3==header.datatypecode || 2==header.datatypecode) {
        in.read((char *)m_data, nbytes);
        if (!in.good()) {
            in.close();
            std::cerr << "an error occured while reading the data\n";
            return false;
        }
    } else if (10==header.datatypecode||11==header.datatypecode) {
        if (!load_rle_data(in)) {
            in.close();
            std::cerr << "an error occured while reading the data\n";
            return false;
        }
    } else {
        in.close();
        std::cerr << "unknown file format " << (int)header.datatypecode << "\n";
        return false;
    }
    if (!(header.imagedescriptor & 0x20)) {
        flip_vertically();
    }
    if (header.imagedescriptor & 0x10) {
        flip_horizontally();
    }
    std::cerr << m_width << "x" << m_height << "/" << m_bytespp*8 << "\n";
    in.close();
    return true;
}

bool TGAImage::load_rle_data(std::ifstream &in) {
    unsigned long pixelcount = m_width*m_height;
    unsigned long currentpixel = 0;
    unsigned long currentbyte  = 0;
    TGAColor colorbuffer;
    do {
        unsigned char chunkheader = 0;
        chunkheader = in.get();
        if (!in.good()) {
            std::cerr << "an error occured while reading the data\n";
            return false;
        }
        if (chunkheader<128) {
            chunkheader++;
            for (int i=0; i<chunkheader; i++) {
                in.read((char *)colorbuffer.bgra, m_bytespp);
                if (!in.good()) {
                    std::cerr << "an error occured while reading the header\n";
                    return false;
                }
                for (int t=0; t<m_bytespp; t++)
                    m_data[currentbyte++] = colorbuffer.bgra[t];
                currentpixel++;
                if (currentpixel>pixelcount) {
                    std::cerr << "Too many pixels read\n";
                    return false;
                }
            }
        } else {
            chunkheader -= 127;
            in.read((char *)colorbuffer.bgra, m_bytespp);
            if (!in.good()) {
                std::cerr << "an error occured while reading the header\n";
                return false;
            }
            for (int i=0; i<chunkheader; i++) {
                for (int t=0; t<m_bytespp; t++)
                    m_data[currentbyte++] = colorbuffer.bgra[t];
                currentpixel++;
                if (currentpixel>pixelcount) {
                    std::cerr << "Too many pixels read\n";
                    return false;
                }
            }
        }
    } while (currentpixel < pixelcount);
    return true;
}

bool TGAImage::write_tga_file(const char *filename, bool rle) {
    unsigned char developer_area_ref[4] = {0, 0, 0, 0};
    unsigned char extension_area_ref[4] = {0, 0, 0, 0};
    unsigned char footer[18] = {'T','R','U','E','V','I','S','I','O','N','-','X','F','I','L','E','.','\0'};
    std::ofstream out;
    out.open (filename, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "can't open file " << filename << "\n";
        out.close();
        return false;
    }
    TGA_Header header;
    memset((void *)&header, 0, sizeof(header));
    header.bitsperpixel = m_bytespp<<3;
    header.width  = m_width;
    header.height = m_height;
    header.datatypecode = (m_bytespp==GRAYSCALE?(rle?11:3):(rle?10:2));
    header.imagedescriptor = 0x20; // top-left origin
    out.write((char *)&header, sizeof(header));
    if (!out.good()) {
        out.close();
        std::cerr << "can't dump the tga file\n";
        return false;
    }
    if (!rle) {
        out.write((char *)m_data, m_width*m_height*m_bytespp);
        if (!out.good()) {
            std::cerr << "can't unload raw data\n";
            out.close();
            return false;
        }
    } else {
        if (!unload_rle_data(out)) {
            out.close();
            std::cerr << "can't unload rle data\n";
            return false;
        }
    }
    out.write((char *)developer_area_ref, sizeof(developer_area_ref));
    if (!out.good()) {
        std::cerr << "can't dump the tga file\n";
        out.close();
        return false;
    }
    out.write((char *)extension_area_ref, sizeof(extension_area_ref));
    if (!out.good()) {
        std::cerr << "can't dump the tga file\n";
        out.close();
        return false;
    }
    out.write((char *)footer, sizeof(footer));
    if (!out.good()) {
        std::cerr << "can't dump the tga file\n";
        out.close();
        return false;
    }
    out.close();
    return true;
}

// TODO: it is not necessary to break a raw chunk for two equal pixels (for the matter of the resulting size)
bool TGAImage::unload_rle_data(std::ofstream &out) {
    const unsigned char max_chunk_length = 128;
    unsigned long npixels = m_width*m_height;
    unsigned long curpix = 0;
    while (curpix<npixels) {
        unsigned long chunkstart = curpix*m_bytespp;
        unsigned long curbyte = curpix*m_bytespp;
        unsigned char run_length = 1;
        bool raw = true;
        while (curpix+run_length<npixels && run_length<max_chunk_length) {
            bool succ_eq = true;
            for (int t=0; succ_eq && t<m_bytespp; t++) {
                succ_eq = (m_data[curbyte+t]==m_data[curbyte+t+m_bytespp]);
            }
            curbyte += m_bytespp;
            if (1==run_length) {
                raw = !succ_eq;
            }
            if (raw && succ_eq) {
                run_length--;
                break;
            }
            if (!raw && !succ_eq) {
                break;
            }
            run_length++;
        }
        curpix += run_length;
        out.put(raw?run_length-1:run_length+127);
        if (!out.good()) {
            std::cerr << "can't dump the tga file\n";
            return false;
        }
        out.write((char *)(m_data+chunkstart), (raw?run_length*m_bytespp:m_bytespp));
        if (!out.good()) {
            std::cerr << "can't dump the tga file\n";
            return false;
        }
    }
    return true;
}

size_t TGAImage::index(int x, int y) const
{
    return (x + y * m_width) * m_bytespp;
}

TGAColor TGAImage::get(int x, int y) {
    if (!m_data || x<0 || y<0 || x>=m_width || y>=m_height) {
        return TGAColor();
    }
    return TGAColor(m_data + index(x, y), m_bytespp);
}

bool TGAImage::set(int x, int y, const TGAColor &c) {
    if (!m_data || x<0 || y<0 || x>=m_width || y>=m_height) {
        return false;
    }
    memcpy(m_data + index(x, y), c.bgra, m_bytespp);
    return true;
}

int TGAImage::get_bytespp() {
    return m_bytespp;
}

int TGAImage::get_width() const {
    return m_width;
}

int TGAImage::get_height() const {
    return m_height;
}

Vec2i TGAImage::get_size() const
{
    return Vec2i(m_width, m_height);
}

bool TGAImage::flip_horizontally() {
    if (!m_data) return false;
    int half = m_width>>1;
    for (int i=0; i<half; i++) {
        for (int j=0; j<m_height; j++) {
            TGAColor c1 = get(i, j);
            TGAColor c2 = get(m_width-1-i, j);
            set(i, j, c2);
            set(m_width-1-i, j, c1);
        }
    }
    return true;
}

bool TGAImage::flip_vertically() {
    if (!m_data) return false;
    unsigned long bytes_per_line = m_width*m_bytespp;
    unsigned char *line = new unsigned char[bytes_per_line];
    int half = m_height>>1;
    for (int j=0; j<half; j++) {
        unsigned long l1 = j*bytes_per_line;
        unsigned long l2 = (m_height-1-j)*bytes_per_line;
        memmove((void *)line,      (void *)(m_data+l1), bytes_per_line);
        memmove((void *)(m_data+l1), (void *)(m_data+l2), bytes_per_line);
        memmove((void *)(m_data+l2), (void *)line,      bytes_per_line);
    }
    delete [] line;
    return true;
}

unsigned char *TGAImage::buffer() {
    return m_data;
}

void TGAImage::clear() {
    memset((void *)m_data, 0, m_width*m_height*m_bytespp);
}

bool TGAImage::scale(int w, int h) {
    if (w<=0 || h<=0 || !m_data) return false;
    unsigned char *tdata = new unsigned char[w*h*m_bytespp];
    int nscanline = 0;
    int oscanline = 0;
    int erry = 0;
    unsigned long nlinebytes = w*m_bytespp;
    unsigned long olinebytes = m_width*m_bytespp;
    for (int j=0; j<m_height; j++) {
        int errx = m_width-w;
        int nx   = -m_bytespp;
        int ox   = -m_bytespp;
        for (int i=0; i<m_width; i++) {
            ox += m_bytespp;
            errx += w;
            while (errx>=(int)m_width) {
                errx -= m_width;
                nx += m_bytespp;
                memcpy(tdata+nscanline+nx, m_data+oscanline+ox, m_bytespp);
            }
        }
        erry += h;
        oscanline += olinebytes;
        while (erry>=(int)m_height) {
            if (erry>=(int)m_height<<1) // it means we jump over a scanline
                memcpy(tdata+nscanline+nlinebytes, tdata+nscanline, nlinebytes);
            erry -= m_height;
            nscanline += nlinebytes;
        }
    }
    delete [] m_data;
    m_data = tdata;
    m_width = w;
    m_height = h;
    return true;
}

