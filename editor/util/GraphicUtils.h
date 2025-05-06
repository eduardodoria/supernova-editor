#pragma once

#include "object/Camera.h"

namespace Supernova::Editor {
    class GraphicUtils {
    public:
        static void saveImage(int width, int height, FramebufferRender& framebuffer);

    };
}