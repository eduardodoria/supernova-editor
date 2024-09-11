#ifndef EDITORPLATFORM_H
#define EDITORPLATFORM_H

#include <wx/wx.h>
#include "Frame.h"
#include "System.h"

namespace Supernova::Editor{

    class Platform : public System{

    public:

        virtual int getScreenWidth();
        virtual int getScreenHeight();
    };

}

#endif /* EDITORPLATFORM_H */