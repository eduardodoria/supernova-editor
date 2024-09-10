#ifndef SUPERNOVAEDITOR_H
#define SUPERNOVAEDITOR_H

#include <wx/wx.h>
#include "EditorFrame.h"
#include "System.h"

class SupernovaEditor : public wxApp, public Supernova::System{
private:
    Supernova::EditorFrame *frame;
public:
    bool OnInit() override;

    int getScreenWidth();
    int getScreenHeight();
};

#endif /* SUPERNOVAEDITOR_H */