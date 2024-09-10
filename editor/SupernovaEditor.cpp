#include "SupernovaEditor.h"

bool SupernovaEditor::OnInit(){
    frame = new Supernova::EditorFrame("Supernova Editor", wxDefaultPosition, wxSize(1280, 800));
    //Supernova::Engine::systemInit(argc, argv);
    frame->Show(true);
    return true;
}

int SupernovaEditor::getScreenWidth(){
    return frame->getCanvas()->GetSize().GetWidth();
}

int SupernovaEditor::getScreenHeight(){
    return frame->getCanvas()->GetSize().GetHeight();
}