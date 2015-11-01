#include "sdlwindow.h"
#include <SDL2/SDL.h>
#include "tgaimage.h"

namespace {

const char WINDOW_TITLE[] = "Rendered Frame";

}

class SDLWindow::Impl
{
public:
    std::shared_ptr<TGAImage> m_pImage;
    SDL_Window *m_pWindow = nullptr;
    SDL_Surface *m_pWindowSurf = nullptr;

    Impl()
    {
    }

    ~Impl()
    {
        destroy_window();
    }

    void create_window()
    {
        destroy_window();
        uint32_t flags = 0;
        m_pWindow = SDL_CreateWindow(WINDOW_TITLE,
                                    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                    m_pImage->get_width(), m_pImage->get_height(), flags);
        m_pWindowSurf = SDL_GetWindowSurface(m_pWindow);
        SDL_ShowWindow(m_pWindow);
    }

    void wait_for_closed()
    {
        SDL_Event event;
        while (SDL_WaitEvent(&event)) {
            if (event.type == SDL_WINDOWEVENT) {
                switch (event.window.event) {
                case SDL_WINDOWEVENT_EXPOSED:
                case SDL_WINDOWEVENT_ENTER:
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    draw_image();
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    return;
                }
            }
        }
    }

    void destroy_window()
    {
        if (m_pWindow) {
            SDL_DestroyWindow(m_pWindow);
            m_pWindow = nullptr;
        }
    }

private:
    void draw_image()
    {
        SDL_Surface *pImageSurf = create_tga_surface(*m_pImage);
        auto getDrawRect = [](SDL_Surface *pSurface) {
            SDL_Rect rect {0, 0, pSurface->w, pSurface->h};
            return rect;
        };
        SDL_Rect srcRect = getDrawRect(pImageSurf);
        SDL_Rect dstRect = getDrawRect(m_pWindowSurf);
        SDL_BlitSurface(pImageSurf, &srcRect, m_pWindowSurf, &dstRect);
        SDL_UpdateWindowSurface(m_pWindow);
        SDL_FreeSurface(pImageSurf);
    }

    SDL_Surface *create_tga_surface(TGAImage &img)
    {
        int bytesPerPixel = img.get_bytespp();
        uint32_t redMask = 0;
        uint32_t greenMask = 0;
        uint32_t blueMask = 0;
        uint32_t alphaMask = 0;
        switch (bytesPerPixel) {
        case TGAImage::GRAYSCALE:
            redMask   = 0xFF;
            greenMask = 0xFF;
            blueMask  = 0xFF;
            break;
        case TGAImage::RGB:
            redMask   = 0xFF0000;
            greenMask = 0x00FF00;
            blueMask  = 0x0000FF;
            break;
        case TGAImage::RGBA:
            redMask   = 0xFF000000;
            greenMask = 0x00FF0000;
            blueMask  = 0x0000FF00;
            alphaMask = 0x000000FF;
            break;
        }

        return SDL_CreateRGBSurfaceFrom(
                    img.buffer(),
                    img.get_width(),
                    img.get_height(),
                    8 * img.get_bytespp(),
                    img.get_bytespp() * img.get_width(),
                    redMask, greenMask, blueMask, alphaMask);
    }
};

SDLWindow::SDLWindow(const std::shared_ptr<TGAImage> &pImage)
    : d(new Impl)
{
    d->m_pImage = pImage;
}

SDLWindow::~SDLWindow()
{
}

void SDLWindow::show()
{
    d->create_window();
    d->wait_for_closed();
    d->destroy_window();
}

