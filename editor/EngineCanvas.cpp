#include "EngineCanvas.h"

#include <GLFW/glfw3.h>

using namespace Supernova;


EngineCanvas::EngineCanvas(wxWindow* parent): wxGLCanvas(parent, wxID_ANY, nullptr) {
    context = new wxGLContext(this);
    isInitialized = false;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    Bind(wxEVT_SIZE, &EngineCanvas::OnResize, this);
    Bind(wxEVT_PAINT, &EngineCanvas::OnPaint, this);
}

void EngineCanvas::ViewLoaded(){
    isInitialized = true;
}

void EngineCanvas::Render() {
    SetCurrent(*context);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

    SwapBuffers();
}

void EngineCanvas::OnPaint(wxPaintEvent& event){
    Render();
}

void EngineCanvas::OnResize(wxSizeEvent& event){
    if (!isInitialized) {
        event.Skip(); // Skip the event if not visible
        return;
    }

    wxSize size = this->GetSize();
    
    // Log the new size of the left panel
    printf("resized to: %d x %d\n", size.GetWidth(), size.GetHeight());
    
    // Call the base class handler
    event.Skip();
}