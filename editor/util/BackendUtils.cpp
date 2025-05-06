#include "BackendUtils.h"

#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "stb_image_write.h"

using namespace Supernova;

void Editor::BackendUtils::saveImage(int width, int height, FramebufferRender& framebuffer){
    uint8_t* pixels = new uint8_t[width * height * 4];

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.getGLHandler());
    glReadPixels(
        0, 0, width, height,
        GL_RGBA, GL_UNSIGNED_BYTE,
        pixels
    );
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    stbi_write_png("output.png", width, height, 4, pixels, width * 4);

    delete[] pixels;
}