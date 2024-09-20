#ifndef EDITORPLATFORM_H
#define EDITORPLATFORM_H

#include "System.h"

namespace Supernova::Editor{

    class Platform : public System{

    public:

        static int width;
        static int height;

        virtual int getScreenWidth();
        virtual int getScreenHeight();
    };

}

#endif /* EDITORPLATFORM_H */