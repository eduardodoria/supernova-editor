#ifndef EDITORPLATFORM_H
#define EDITORPLATFORM_H

#include "System.h"
#include "Project.h"

namespace Supernova::Editor{

    class Platform : public System{
    private:
        Project* project;

        static int width;
        static int height;

    public:
        Platform(Project* project);

        static bool setSizes(int width, int height);

        int getScreenWidth() override;
        int getScreenHeight() override;

        std::string getAssetPath() override;
    };

}

#endif /* EDITORPLATFORM_H */