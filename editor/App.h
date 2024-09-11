#ifndef EDITORAPP_H
#define EDITORAPP_H

#include <wx/wx.h>
#include "Frame.h"
#include "System.h"

namespace Supernova::Editor{

    class App : public wxApp{
    private:
        static Editor::Frame *frame;

    public:
        bool OnInit() override;

        static Editor::Frame* getFrame();
    };

}

#endif /* EDITORAPP_H */