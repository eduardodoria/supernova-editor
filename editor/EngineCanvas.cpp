#include "EngineCanvas.h"

#include "Engine.h"

using namespace Supernova;


EngineCanvas::EngineCanvas(wxWindow* parent): wxGLCanvas(parent, wxID_ANY, nullptr) {
    context = new wxGLContext(this);
    isInitialized = false;

    Bind(wxEVT_SIZE, &EngineCanvas::OnResize, this);
    Bind(wxEVT_PAINT, &EngineCanvas::OnPaint, this);
}

void EngineCanvas::ViewLoaded(){
    SetCurrent(*context);
    isInitialized = true;

    wxSize size = this->GetSize();
    printf("ViewLoaded to: %d x %d\n", size.GetWidth(), size.GetHeight());
    Engine::systemViewLoaded();
    Engine::systemViewChanged();
}

void EngineCanvas::Render() {
    SetCurrent(*context);

    Engine::systemDraw();

    SwapBuffers();
    printf("render\n");
}

void EngineCanvas::OnPaint(wxPaintEvent& event){
    wxPaintDC(this);

    if (!isInitialized){
        ViewLoaded();
    }

    Render();

    event.Skip();
}

void EngineCanvas::OnResize(wxSizeEvent& event){
    if (!isInitialized) {
        event.Skip(); // Skip the event if not visible
        return;
    }
    
    // Log the new size of the left panel
    wxSize size = this->GetSize();
    printf("OnResize to: %d x %d\n", size.GetWidth(), size.GetHeight());
    Engine::systemViewChanged();
    
    // Call the base class handler
    event.Skip();
}

//void EngineCanvas::OnExit(wxCommandEvent& event){
//    printf("oiii");
//}