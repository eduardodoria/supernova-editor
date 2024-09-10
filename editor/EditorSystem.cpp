#include "EditorSystem.h"

#include "SupernovaEditor.h"

int EditorSystem::getScreenWidth(){
    return SupernovaEditor::getFrame()->getCanvas()->GetSize().GetWidth();
}

int EditorSystem::getScreenHeight(){
    return SupernovaEditor::getFrame()->getCanvas()->GetSize().GetHeight();
}