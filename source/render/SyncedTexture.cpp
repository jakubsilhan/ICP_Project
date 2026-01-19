#include <GL/glew.h>

#include "render/SyncedTexture.hpp"

void SyncedTexture::fence_sync() {
    fence_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

void SyncedTexture::fence_wait() {
    if (fence_) {
        glWaitSync(fence_, 0, GL_TIMEOUT_IGNORED);
        glDeleteSync(fence_);
        fence_ = nullptr;
    }
}

SyncedTexture::~SyncedTexture() {
    fence_wait();
    Texture::~Texture();
}
