#pragma once

#include "render/Texture.hpp"

class SyncedTexture : public Texture
{
public:
    using Texture::Texture;
    void fence_sync();
    void fence_wait();
    ~SyncedTexture();
private:
    GLsync fence_ = nullptr;
};