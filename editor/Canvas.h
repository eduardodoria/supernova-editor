#ifndef EDITORCANVAS_H
#define EDITORCANVAS_H

#include <wx/wx.h>
#include <wx/glcanvas.h>

namespace Supernova::Editor{
    class Canvas : public wxGLCanvas {
    private:
        bool isInitialized;
        wxGLContext* context;

        void ViewLoaded();
        void Render();

    public:
        Canvas(wxWindow* parent);

        void OnPaint(wxPaintEvent& event);
        void OnResize(wxSizeEvent& event);
    };
}

#endif /* EDITORCANVAS_H */