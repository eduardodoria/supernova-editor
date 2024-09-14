#include "Frame.h"

#include "Engine.h"

using namespace Supernova;

Editor::Frame::Frame(const wxString &title, const wxPoint &pos, const wxSize &size)
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

    splitterLeft = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_BORDER | wxSP_LIVE_UPDATE);
    splitterRight = new wxSplitterWindow(splitterLeft, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_BORDER | wxSP_LIVE_UPDATE);
    splitterMiddle = new wxSplitterWindow(splitterRight, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_BORDER | wxSP_LIVE_UPDATE);

    wxPanel* panelLeft = new wxPanel(splitterLeft, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    wxPanel* panelRight = new wxPanel(splitterRight, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    wxPanel* panelMiddleTop = new wxPanel(splitterMiddle, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    wxPanel* panelMiddleBottom = new wxPanel(splitterMiddle, wxID_ANY, wxDefaultPosition, wxDefaultSize);


    splitterLeft->SplitVertically(panelLeft, splitterRight);
    splitterLeft->SetMinimumPaneSize(100);
    splitterLeft->SetSashGravity(0.0);
    //splitterLeft->Layout();

    splitterRight->SplitVertically(splitterMiddle, panelRight);
    splitterRight->SetMinimumPaneSize(100);
    splitterRight->SetSashGravity(1.0);
    //splitterRight->Layout();

    
    splitterMiddle->SplitHorizontally(panelMiddleTop, panelMiddleBottom);
    splitterMiddle->SetMinimumPaneSize(100);
    splitterMiddle->SetSashGravity(1.0);
    //splitterMiddle->Layout();
    
    
    canvas = new Editor::Canvas(panelMiddleTop);
    textConsole = new wxTextCtrl(panelMiddleBottom, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxBORDER_NONE);
    sceneTree = new wxDataViewTreeCtrl( panelLeft, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE | wxDV_NO_HEADER );

    wxBoxSizer* canvasSizer = new wxBoxSizer(wxVERTICAL);
    canvasSizer->Add(canvas, 1, wxEXPAND | wxALL, 0);
    panelMiddleTop->SetSizer(canvasSizer);
    
    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
    leftSizer->Add(sceneTree, 1, wxEXPAND | wxALL, 0);
    panelLeft->SetSizer(leftSizer);
    
    wxBoxSizer* middleBottomSizer = new wxBoxSizer(wxVERTICAL);
    middleBottomSizer->Add(textConsole, 1, wxEXPAND | wxALL, 0);
    panelMiddleBottom->SetSizer(middleBottomSizer);

    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);

    // Create a notebook and add a tab for the property grid
    wxNotebook* notebook = new wxNotebook(panelRight, wxID_ANY);
    wxPanel* inspectorPanel = new wxPanel(notebook, wxID_ANY);
    notebook->AddPage(inspectorPanel, "Inspector");

    // Create the property grid with inspectorPanel as the parent
    propertyGrid = new wxPropertyGrid(inspectorPanel, wxID_ANY, wxDefaultPosition, wxSize(300, 400), wxBORDER_NONE);

    // Add the property grid to the inspector panel
    wxBoxSizer* inspectorSizer = new wxBoxSizer(wxVERTICAL);
    inspectorSizer->Add(propertyGrid, 1, wxEXPAND | wxALL, 0);
    inspectorPanel->SetSizer(inspectorSizer);

    // Add the notebook to the right sizer
    rightSizer->Add(notebook, 1, wxEXPAND | wxALL, 0);
    panelRight->SetSizer(rightSizer);


    //======== start tree ==================
    wxDataViewTreeCtrl::Images images;
    images.push_back(wxArtProvider::GetBitmapBundle(wxART_WX_LOGO, wxART_LIST));
    sceneTree->SetImages(images);

    const wxDataViewItem root = sceneTree->AppendContainer( wxDataViewItem(0), "The Root", 0 );
    sceneTree->AppendItem( root, "Child 1", 0 );
    sceneTree->AppendItem( root, "Child 2", 0 );
    sceneTree->AppendItem( root, "Child 3, very long, long, long, long", 0 );

    wxDataViewItem cont = sceneTree->AppendContainer( root, "Container child", 0 );
    sceneTree->AppendItem( cont, "Child 4", 0 );
    sceneTree->AppendItem( cont, "Child 5", 0 );

    sceneTree->Expand(cont);
    //======== end tree ==================


    // Add properties to the property grid
    propertyGrid->Append(new wxPropertyCategory("General Settings"));
    propertyGrid->Append(new wxStringProperty("Name", wxPG_LABEL, "Default Name"));
    propertyGrid->Append(new wxIntProperty("Age", wxPG_LABEL, 30));
    propertyGrid->Append(new wxBoolProperty("Enable Feature", wxPG_LABEL, true));

    propertyGrid->Append(new wxPropertyCategory("Advanced Settings"));
    propertyGrid->Append(new wxFloatProperty("Threshold", wxPG_LABEL, 0.5));
    propertyGrid->Append(new wxColourProperty("Color", wxPG_LABEL, *wxWHITE));

 
    Bind(wxEVT_SHOW, &Editor::Frame::OnShow, this);
    Bind(wxEVT_SIZE, &Editor::Frame::OnSize, this);
    Bind(wxEVT_CLOSE_WINDOW, &Editor::Frame::OnExit, this);

    Bind(wxEVT_MENU, &Editor::Frame::OnHelloMenu, this, ID_Hello);
    Bind(wxEVT_MENU, &Editor::Frame::OnAboutMenu, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &Editor::Frame::OnExitMenu, this, wxID_EXIT);
}



void Editor::Frame::OnShow(wxShowEvent& event){
    if (event.IsShown()) {
        Layout();

        splitterLeft->SetSashPosition(200);
        splitterMiddle->SetSashPosition(GetSize().GetHeight() - 200);
        splitterRight->SetSashPosition(GetSize().GetWidth() - splitterLeft->GetSashPosition() - 300);

        textConsole->AppendText("Supernova Engine console.\n");
        textConsole->AppendText("Welcome to the Text Console!\n");
        textConsole->AppendText("This is an example of multi-line text.\n");
        textConsole->AppendText("You can add more lines as needed.\n");
    }
    event.Skip();
}

void Editor::Frame::OnSize(wxSizeEvent& event) {
    event.Skip();
}

void Editor::Frame::OnExit(wxCloseEvent& event){
    Engine::systemShutdown();

    event.Skip();
}
 
void Editor::Frame::OnExitMenu(wxCommandEvent& event){
    Close(true);
}
 
void Editor::Frame::OnAboutMenu(wxCommandEvent& event){
    wxMessageBox("This is a wxWidgets Hello World example",
                 "About Hello World", wxOK | wxICON_INFORMATION);
}
 
void Editor::Frame::OnHelloMenu(wxCommandEvent& event){
    wxLogMessage("Hello world from wxWidgets!");
}

Editor::Canvas* Editor::Frame::getCanvas(){
    return canvas;
}