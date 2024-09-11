#ifndef EDITORFRAME_H
#define EDITORFRAME_H

#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/treebase.h>
#include <wx/treectrl.h>
#include <wx/notebook.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "Canvas.h"

namespace Supernova::Editor{
    enum{
        ID_Hello = 1
    };

    class Frame : public wxFrame{
    private:
        wxSplitterWindow* splitterRight;
        wxSplitterWindow* splitterMiddle;
        wxSplitterWindow* splitterLeft;

        wxTextCtrl* textConsole;
        Canvas* canvas;
        wxTreeCtrl* sceneTree;
        wxPropertyGrid* propertyGrid;

        void OnShow(wxShowEvent& event);
        void OnExit(wxCloseEvent& event);

        void OnHelloMenu(wxCommandEvent& event);
        void OnExitMenu(wxCommandEvent& event);
        void OnAboutMenu(wxCommandEvent& event);
         
    public:
        Frame(const wxString &title, const wxPoint &pos, const wxSize &size);

        Canvas* getCanvas();
    };
}

#endif /* EDITORFRAME_H */