#include "Canvas.h"

#include "Engine.h"

using namespace Supernova;


Editor::Canvas::Canvas(wxWindow* parent): wxGLCanvas(parent, wxID_ANY, nullptr) {
    context = new wxGLContext(this);
    isInitialized = false;

    Bind(wxEVT_SIZE, &Editor::Canvas::OnResize, this);
    Bind(wxEVT_PAINT, &Editor::Canvas::OnPaint, this);
}

void Editor::Canvas::ViewLoaded(){
    SetCurrent(*context);
    isInitialized = true;

    wxSize size = this->GetSize();
    printf("ViewLoaded to: %d x %d\n", size.GetWidth(), size.GetHeight());
    Engine::systemViewLoaded();
    Engine::systemViewChanged();
}

void Editor::Canvas::Render() {
    SetCurrent(*context);

    Engine::systemDraw();

    SwapBuffers();
    printf("render\n");
}

void Editor::Canvas::OnPaint(wxPaintEvent& event){
    wxPaintDC(this);

    if (!isInitialized){
        ViewLoaded();
    }

    Render();

    event.Skip();
}

void Editor::Canvas::OnResize(wxSizeEvent& event){
    if (!isInitialized) {
        event.Skip(); // Skip the event if not visible
        return;
    }
    
    // Log the new size of the left panel
    wxSize size = this->GetSize();
    printf("OnResize to: %d x %d\n", size.GetWidth(), size.GetHeight());
    Engine::systemViewChanged();

    Render();
    
    // Call the base class handler
    event.Skip();
}

//void EngineCanvas::OnExit(wxCommandEvent& event){
//    printf("oiii");
//}