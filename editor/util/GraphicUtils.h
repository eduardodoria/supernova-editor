#pragma once

#include "object/Camera.h"
#include <filesystem>

namespace fs = std::filesystem;

namespace Supernova::Editor {
    class GraphicUtils {
    public:
        static void saveFramebufferImage(Framebuffer* framebuffer, fs::path path);

    };
}