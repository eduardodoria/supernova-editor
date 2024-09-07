#ifndef ENGINECANVAS_H
#define ENGINECANVAS_H

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <GLFW/glfw3.h>

namespace Supernova{
    class EngineCanvas : public wxGLCanvas {
    private:
        bool isInitialized;
        wxGLContext* context;

    public:
        EngineCanvas(wxWindow* parent);

        void ViewLoaded();
        void Render();

        void OnPaint(wxPaintEvent& event);
        void OnResize(wxSizeEvent& event);
    };
}

#endif /* ENGINECANVAS_H */