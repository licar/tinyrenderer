#include "sdlwindow.h"
#include <SDL.h>
#include <stdio.h>
#include <thread>
#include "tgaimage.h"

class SDLWindow::Impl
{
public:
    std::shared_ptr<TGAImage> m_pImage;
    Vec2i m_windowSize;
    SDL_Window *m_pWindow = nullptr;
    SDL_Surface *m_pWindowSurf = nullptr;
    int m_startTicks = 0;
    int m_framesCount = 0;
    std::function<void ()> m_onIdle;

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
        m_pWindow = SDL_CreateWindow("Waiting for 1st frame...",
                                    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                    m_windowSize.x, m_windowSize.y, flags);
        m_pWindowSurf = SDL_GetWindowSurface(m_pWindow);
        SDL_ShowWindow(m_pWindow);
    }

    void wait_for_closed()
    {
        SDL_Event event;
        for (;;) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_WINDOWEVENT) {
                    switch (event.window.event) {
                    case SDL_WINDOWEVENT_SHOWN:
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
            on_idle();
            draw_image();
        }
    }

    void destroy_window()
    {
        if (m_pWindow) {
            SDL_DestroyWindow(m_pWindow);
            m_pWindow = nullptr;
        }
    }

    void swap_buffers(std::shared_ptr<TGAImage> &pImage)
    {
        std::swap(pImage, m_pImage);
        if (!m_pImage)
        {
            init_framebuffer();
        }
        else
        {
            ++m_framesCount;
            int averageFrameTime = (SDL_GetTicks() - m_startTicks) / m_framesCount;
            char title[1024];
            sprintf(title, "Rendered %d frames, average time %d ms", m_framesCount, averageFrameTime);
            SDL_SetWindowTitle(m_pWindow, title);
            SDL_UpdateWindowSurface(m_pWindow);
        }
    }

    void init_framebuffer()
    {
        m_pImage = std::make_shared<TGAImage>(m_windowSize.x, m_windowSize.y, TGAImage::RGB);
    }

private:
    void on_idle()
    {
        if (m_onIdle)
        {
            m_onIdle();
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

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

SDLWindow::SDLWindow(int width, int height)
    : d(new Impl)
{
    d->m_startTicks = SDL_GetTicks();
    d->m_windowSize = Vec2i(width, height);
    d->init_framebuffer();
}

SDLWindow::~SDLWindow()
{
}

void SDLWindow::swapBuffers(std::shared_ptr<TGAImage> &pImage)
{
    return d->swap_buffers(pImage);
}

void SDLWindow::show()
{
    d->create_window();
}

void SDLWindow::wait_for_closed()
{
    d->wait_for_closed();
    d->destroy_window();
}

void SDLWindow::do_on_idle(std::function<void ()> action)
{
    d->m_onIdle = action;
}

