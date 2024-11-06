#ifndef EDITORBACKEND_H
#define EDITORBACKEND_H

#include "App.h"

namespace Supernova::Editor{

    class Backend{
    private:
        static App app;

    public:
        static int init(int argc, char **argv);

        static App& getApp();

        static void disableMouseCursor();
        static void enableMouseCursor();
    };

}

#endif /* EDITORBACKEND_H */