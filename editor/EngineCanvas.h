#ifndef ENGINECANVAS_H
#define ENGINECANVAS_H

#include <wx/wx.h>
#include <wx/glcanvas.h>

namespace Supernova{
    class EngineCanvas : public wxGLCanvas {
    private:
        bool isInitialized;
        wxGLContext* context;

        void ViewLoaded();
        void Render();

    public:
        EngineCanvas(wxWindow* parent);

        void OnPaint(wxPaintEvent& event);
        void OnResize(wxSizeEvent& event);
    };
}

#endif /* ENGINECANVAS_H */