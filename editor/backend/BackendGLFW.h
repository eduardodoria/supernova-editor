#ifndef EDITORBACKEND_H
#define EDITORBACKEND_H

#include <GLFW/glfw3.h>

namespace Supernova::Editor{

    class Backend{
    private:
        static GLFWwindow* window;

    public:
        static int init(int argc, char **argv);

        static void disableMouseCursor();
        static void enableMouseCursor();
    };

}

#endif /* EDITORBACKEND_H */