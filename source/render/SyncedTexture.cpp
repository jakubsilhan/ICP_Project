#include <GL/glew.h>

#include "render/SyncedTexture.hpp"

void SyncedTexture::fence_sync() {
    fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

void SyncedTexture::fence_wait() {
    if (fence) {
        glWaitSync(fence, 0, GL_TIMEOUT_IGNORED);
        glDeleteSync(fence);
        fence = nullptr;
    }
}

SyncedTexture::~SyncedTexture() {
    fence_wait();
    Texture::~Texture();
}
