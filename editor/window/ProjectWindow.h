#ifndef PROJECTWINDOW_H
#define PROJECTWINDOW_H


#include "Project.h"


namespace Supernova::Editor{

    struct FileEntry {
        std::string name;
        bool isDirectory;
        void* icon; // Texture ID for the icon
    };

    class ProjectWindow{
    private:
        Project* project;

        bool firstOpen;
        std::string currentPath;
        std::vector<Editor::FileEntry> entries;
        void* folderIcon;
        void* fileIcon;

        std::unordered_set<std::string> selectedFiles; // To track selected files
        bool ctrlPressed; // To handle multi-selection using CTRL key
        bool shiftPressed; // To handle range selection using SHIFT key

        void* LoadTexture(const char* filePath, int& outWidth, int& outHeight);
        void FreeTexture(void* textureID);
        std::vector<FileEntry> ScanDirectory(const std::string& path, void* folderIcon, void* fileIcon);

    public:
        ProjectWindow(Project* project);

        void show();
    };

}

#endif /* PROJECTWINDOW_H */