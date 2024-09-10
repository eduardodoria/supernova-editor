#ifndef EDITORFRAME_H
#define EDITORFRAME_H

#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/treebase.h>
#include <wx/treectrl.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "EngineCanvas.h"

namespace Supernova{
    enum{
        ID_Hello = 1
    };

    class EditorFrame : public wxFrame{
    private:
        wxSplitterWindow* splitterMain;
        wxSplitterWindow* splitter_left;
        wxSplitterWindow* splitter_top;
        wxTextCtrl* textConsole;
        EngineCanvas* canvas;

        void OnShow(wxShowEvent& event);
        void OnExit(wxCloseEvent& event);

        void OnHelloMenu(wxCommandEvent& event);
        void OnExitMenu(wxCommandEvent& event);
        void OnAboutMenu(wxCommandEvent& event);
         
    public:
        EditorFrame(const wxString &title, const wxPoint &pos, const wxSize &size);

        EngineCanvas* getCanvas();
    };
}

#endif /* EDITORFRAME_H */