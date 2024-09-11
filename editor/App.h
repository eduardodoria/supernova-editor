#ifndef EDITORAPP_H
#define EDITORAPP_H

#include <wx/wx.h>
#include "EditorFrame.h"
#include "System.h"

namespace Supernova::Editor{

    class App : public wxApp{
    private:
        static EditorFrame *frame;

    public:
        bool OnInit() override;

        static EditorFrame* getFrame();
    };

}

#endif /* EDITORAPP_H */