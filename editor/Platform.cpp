#include "Platform.h"


using namespace Supernova;

int Editor::Platform::width = 0;
int Editor::Platform::height = 0;

int Editor::Platform::getScreenWidth(){
    return width;
}

int Editor::Platform::getScreenHeight(){
    return height;
}