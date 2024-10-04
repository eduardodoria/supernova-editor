#ifndef EDITORPLATFORM_H
#define EDITORPLATFORM_H

#include "System.h"
#include <GLFW/glfw3.h>

namespace Supernova::Editor{

    class Platform : public System{
    private:
        static GLFWwindow* window;

    public:
        static int init(int argc, char **argv);

        static int width;
        static int height;

        virtual int getScreenWidth();
        virtual int getScreenHeight();

        static void disableMouseCursor();
        static void enableMouseCursor();
    };

}

#endif /* EDITORPLATFORM_H */