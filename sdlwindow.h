#pragma once
#include <memory>

class TGAImage;

class SDLWindow
{
public:
    explicit SDLWindow(std::shared_ptr<TGAImage> const& pImage);
    ~SDLWindow();

    void show();

private:
    class Impl;

    std::unique_ptr<Impl> d;
};
