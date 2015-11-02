#pragma once
#include <memory>
#include <functional>

class TGAImage;

class SDLWindow
{
public:
    explicit SDLWindow(int width, int height);
    ~SDLWindow();

    void swapBuffers(std::shared_ptr<TGAImage> & pImage);
    void show();
    void wait_for_closed();
    void do_on_idle(std::function<void()> action);

private:
    class Impl;

    std::unique_ptr<Impl> d;
};
