#include "Platform.h"

using namespace Supernova;

int Editor::Platform::width = 0;
int Editor::Platform::height = 0;

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