#ifndef EDITORPLATFORM_H
#define EDITORPLATFORM_H

#include "System.h"

namespace Supernova::Editor{

    class Platform : public System{
    private:
        static int width;
        static int height;

    public:
        static bool setSizes(int width, int height);

        virtual int getScreenWidth();
        virtual int getScreenHeight();
    };

}

#endif /* EDITORPLATFORM_H */