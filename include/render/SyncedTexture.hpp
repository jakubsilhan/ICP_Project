#pragma once

#include <glm/glm.hpp>

#include "render/Texture.hpp"

class SyncedTexture : public Texture {
public:
    using Texture::Texture;
    void fence_sync();
    void fence_wait();
    ~SyncedTexture();
private:
    GLsync fence = nullptr;
};
