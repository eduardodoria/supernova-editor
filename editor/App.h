#ifndef EDITORAPP_H
#define EDITORAPP_H

#include "imgui.h"

#include "layout/Properties.h"
#include "layout/Objects.h"
#include "layout/Console.h"

namespace Supernova::Editor{

    class App{
    private:
        ImGuiID dockspace_id;
        Objects objectsWindow;
        Properties propertiesWindow;
        Console consoleWindow;

        static bool isInitialized;

        void showMenu();
        void buildDockspace();

    public:
        static unsigned int texture;

        App();

        void show();

        static void engineRender(const ImDrawList* parent_list, const ImDrawCmd* cmd);
    };

}

#endif /* EDITORAPP_H */