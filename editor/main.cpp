#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/treebase.h>
#include <wx/treectrl.h>

#include "EditorFrame.h"
#include "Engine.h"

 
class SupernovaEditor : public wxApp{
public:
    bool OnInit() override;
};

wxIMPLEMENT_APP(SupernovaEditor);

bool SupernovaEditor::OnInit(){
    Supernova::EditorFrame *frame = new Supernova::EditorFrame("Supernova Editor", wxDefaultPosition, wxSize(1280, 800));
    //Supernova::Engine::systemInit(argc, argv);
    frame->Show(true);
    return true;
}
 
