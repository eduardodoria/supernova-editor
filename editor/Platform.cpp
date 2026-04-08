#include "Platform.h"
#include "Out.h"

using namespace Supernova;

int Editor::Platform::width = 0;
int Editor::Platform::height = 0;

Editor::Platform::Platform(Project* project) : System() {
    this->project = project;
}

bool Editor::Platform::setSizes(int width, int height){
    if (Editor::Platform::width != width || Editor::Platform::height != height){
        Editor::Platform::width = width;
        Editor::Platform::height = height;

        return true;
    }

    return false;
}

int Editor::Platform::getScreenWidth(){
    return width;
}

int Editor::Platform::getScreenHeight(){
    return height;
}

std::string Editor::Platform::getAssetPath(){
    return project->getProjectPath().string();
}

void Editor::Platform::platformLog(const int type, const char *fmt, va_list args){
    char buf[4096];
    vsnprintf(buf, sizeof(buf), fmt, args);

    switch (type) {
        case S_LOG_WARN:
            Editor::Out::warning(buf);
            break;
        case S_LOG_ERROR:
            Editor::Out::error(buf);
            break;
        default:
            Editor::Out::info(buf);
            break;
    }
}