#ifndef SUPERNOVAEDITOR_H
#define SUPERNOVAEDITOR_H

#include <wx/wx.h>
#include "EditorFrame.h"
#include "System.h"

class SupernovaEditor : public wxApp{
private:
    static Supernova::EditorFrame *frame;

public:
    bool OnInit() override;

    static Supernova::EditorFrame* getFrame();
};

#endif /* SUPERNOVAEDITOR_H */