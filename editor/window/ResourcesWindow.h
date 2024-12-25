#ifndef RESOURCESWINDOW_H
#define RESOURCESWINDOW_H


#include "Project.h"


namespace Supernova::Editor{

    struct FileEntry {
        std::string name;
        bool isDirectory;
        intptr_t icon;
    };

    class ResourcesWindow{
    private:
        Project* project;

        bool firstOpen;
        std::string currentPath;
        std::vector<Editor::FileEntry> entries;

        Texture folderIcon;
        Texture fileIcon;

        std::unordered_set<std::string> selectedFiles; // To track selected files
        bool ctrlPressed; // To handle multi-selection using CTRL key
        bool shiftPressed; // To handle range selection using SHIFT key

        std::vector<FileEntry> ScanDirectory(const std::string& path, intptr_t folderIcon, intptr_t fileIcon);

    public:
        ResourcesWindow(Project* project);

        void show();
    };

}

#endif /* RESOURCESWINDOW_H */