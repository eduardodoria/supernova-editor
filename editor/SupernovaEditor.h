#ifndef SUPERNOVAEDITOR_H
#define SUPERNOVAEDITOR_H

#include <wx/wx.h>
#include "EditorFrame.h"
#include "System.h"

namespace Supernova{

    class SupernovaEditor : public wxApp{
    private:
        static EditorFrame *frame;

    public:
        bool OnInit() override;

        static EditorFrame* getFrame();
    };

}

#endif /* SUPERNOVAEDITOR_H */