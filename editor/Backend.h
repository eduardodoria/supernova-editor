#ifndef EDITORBACKEND_H
#define EDITORBACKEND_H

#include "App.h"

namespace Supernova::Editor{
    class Backend{
    private:
        static App app;
        static std::string title;

    public:
        static int init(int argc, char **argv);

        static App& getApp();

        static void disableMouseCursor();
        static void enableMouseCursor();
        static void closeWindow();

        static void updateWindowTitle(const std::string& projectName);

        static void* getNFDWindowHandle();
    };

}

#endif /* EDITORBACKEND_H */