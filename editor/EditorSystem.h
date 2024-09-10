#ifndef EDITORSYSTEM_H
#define EDITORSYSTEM_H

#include <wx/wx.h>
#include "EditorFrame.h"
#include "System.h"

namespace Supernova{

    class EditorSystem : public System{

    public:

        virtual int getScreenWidth();
        virtual int getScreenHeight();
    };

}

#endif /* EDITORSYSTEM_H */