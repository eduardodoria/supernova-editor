#include "Platform.h"

#include "App.h"

using namespace Supernova;

int Editor::Platform::getScreenWidth(){
    return Editor::App::getFrame()->getCanvas()->GetSize().GetWidth();
}

int Editor::Platform::getScreenHeight(){
    return Editor::App::getFrame()->getCanvas()->GetSize().GetHeight();
}