#include "EditorSystem.h"

#include "App.h"

using namespace Supernova;

int EditorSystem::getScreenWidth(){
    return Editor::App::getFrame()->getCanvas()->GetSize().GetWidth();
}

int EditorSystem::getScreenHeight(){
    return Editor::App::getFrame()->getCanvas()->GetSize().GetHeight();
}