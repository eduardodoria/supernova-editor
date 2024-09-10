#ifndef EDITORSYSTEM_H
#define EDITORSYSTEM_H

#include <wx/wx.h>
#include "EditorFrame.h"
#include "System.h"

class EditorSystem : public Supernova::System{

public:

    int getScreenWidth();
    int getScreenHeight();
};

#endif /* EDITORSYSTEM_H */