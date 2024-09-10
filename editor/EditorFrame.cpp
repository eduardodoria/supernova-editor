#include "EditorFrame.h"

using namespace Supernova;

EditorFrame::EditorFrame(const wxString &title, const wxPoint &pos, const wxSize &size)
    : wxFrame(nullptr, wxID_ANY, title, pos, size){

    wxMenu* menuFile = new wxMenu;
    menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
                     "Help string shown in status bar for this menu item");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
 
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
 
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");
 
    SetMenuBar( menuBar );
 
    //CreateStatusBar();
    //SetStatusText("Supernova Engine editor");


    splitterMain = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_BORDER | wxSP_LIVE_UPDATE);

    wxPanel* panel_right = new wxPanel(splitterMain, wxID_ANY, wxDefaultPosition, wxSize(200, 100));


    splitter_left = new wxSplitterWindow(splitterMain, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_BORDER | wxSP_LIVE_UPDATE);

    splitter_top = new wxSplitterWindow(splitter_left, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_BORDER | wxSP_LIVE_UPDATE);


    wxPanel* panel_bottom = new wxPanel(splitter_left, wxID_ANY, wxDefaultPosition, wxSize(200, 100));
    //panel_bottom->SetBackgroundColour(wxColor(100, 100, 200));

    wxPanel* panel_top_left = new wxPanel(splitter_top, wxID_ANY, wxDefaultPosition, wxSize(200, 100));
    //panel_top_left->SetBackgroundColour(wxColor(100, 200, 100));

    wxPanel* panel_top_middle = new wxPanel(splitter_top, wxID_ANY, wxDefaultPosition, wxSize(200, 100));
    panel_top_middle->SetBackgroundColour(wxColor(100, 100, 200));

    splitterMain->SplitVertically(splitter_left, panel_right);
    splitterMain->SetMinimumPaneSize(100);
    splitterMain->SetSashGravity(1.0);
    splitterMain->Layout();

    splitter_top->SplitVertically(panel_top_left, panel_top_middle);
    splitter_top->SetMinimumPaneSize(100);
    splitter_top->SetSashGravity(0.0);
    splitter_top->Layout();

    splitter_left->SplitHorizontally(splitter_top, panel_bottom);
    splitter_left->SetMinimumPaneSize(100);
    splitter_left->SetSashGravity(1.0);
    splitter_left->Layout();
    
    
    canvas = new EngineCanvas(panel_top_middle);

    wxBoxSizer* canvasSizer = new wxBoxSizer(wxVERTICAL);
    canvasSizer->Add(canvas, 1, wxEXPAND | wxALL, 0);
    panel_top_middle->SetSizer(canvasSizer);


    // Assuming panel_top_left is already created and initialized
    wxTreeCtrl* treeCtrl = new wxTreeCtrl(panel_top_left, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE);

    // Add some example nodes to the tree
    wxTreeItemId rootId = treeCtrl->AddRoot("Root Node");
    treeCtrl->AppendItem(rootId, "Child Node 1");
    treeCtrl->AppendItem(rootId, "Child Node 2");

    // Set the sizer for panel_top_left if not already set
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(treeCtrl, 1, wxEXPAND | wxALL, 0);
    panel_top_left->SetSizer(topSizer);



    // Add wxTextCtrl to panel_bottom
    textConsole = new wxTextCtrl(panel_bottom, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    
    wxBoxSizer* bottomSizer = new wxBoxSizer(wxVERTICAL);
    bottomSizer->Add(textConsole, 1, wxEXPAND | wxALL, 0);
    panel_bottom->SetSizer(bottomSizer);


    // Create a wxPropertyGrid object
    wxPropertyGrid* propertyGrid = new wxPropertyGrid(panel_right, wxID_ANY, wxDefaultPosition, wxSize(300, 400));

    // Add properties to the property grid
    propertyGrid->Append(new wxPropertyCategory("General Settings"));
    propertyGrid->Append(new wxStringProperty("Name", wxPG_LABEL, "Default Name"));
    propertyGrid->Append(new wxIntProperty("Age", wxPG_LABEL, 30));
    propertyGrid->Append(new wxBoolProperty("Enable Feature", wxPG_LABEL, true));

    propertyGrid->Append(new wxPropertyCategory("Advanced Settings"));
    propertyGrid->Append(new wxFloatProperty("Threshold", wxPG_LABEL, 0.5));
    propertyGrid->Append(new wxColourProperty("Color", wxPG_LABEL, *wxWHITE));



    wxBoxSizer* sizerDetails = new wxBoxSizer(wxVERTICAL);
    sizerDetails->Add(propertyGrid, 1, wxEXPAND | wxALL, 0);
    panel_right->SetSizer(sizerDetails); 




    wxBoxSizer* sizerLeft = new wxBoxSizer(wxVERTICAL);
    sizerLeft->Add(splitter_left, 1, wxEXPAND, 0);
    splitterMain->SetSizer(sizerLeft);

    wxBoxSizer* sizerMain = new wxBoxSizer(wxVERTICAL);
    sizerMain->Add(splitterMain, 1, wxEXPAND, 0);
    this->SetSizer(sizerMain);
 
    Bind(wxEVT_SHOW, &EditorFrame::OnShow, this);
    Bind(wxEVT_MENU, &EditorFrame::OnHello, this, ID_Hello);
    Bind(wxEVT_MENU, &EditorFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &EditorFrame::OnExit, this, wxID_EXIT);
}

void EditorFrame::OnShow(wxShowEvent& event){
    if (event.IsShown()) {
        splitter_top->SetSashPosition(200);
        splitter_left->SetSashPosition(GetSize().GetHeight() - 200);
        splitterMain->SetSashPosition(GetSize().GetWidth() - 300);

        textConsole->AppendText("Supernova Engine console.\n");
        textConsole->AppendText("Welcome to the Text Console!\n");
        textConsole->AppendText("This is an example of multi-line text.\n");
        textConsole->AppendText("You can add more lines as needed.\n");

        canvas->ViewLoaded();
    }
    event.Skip();
}
 
void EditorFrame::OnExit(wxCommandEvent& event){
    Close(true);
}
 
void EditorFrame::OnAbout(wxCommandEvent& event){
    wxMessageBox("This is a wxWidgets Hello World example",
                 "About Hello World", wxOK | wxICON_INFORMATION);
}
 
void EditorFrame::OnHello(wxCommandEvent& event){
    wxLogMessage("Hello world from wxWidgets!");
}

EngineCanvas* EditorFrame::getCanvas(){
    return canvas;
}