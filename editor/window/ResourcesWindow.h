#ifndef RESOURCESWINDOW_H
#define RESOURCESWINDOW_H

#include "Project.h"
#include "command/CommandHistory.h"
#include "window/CodeEditor.h"

#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

#include "imgui.h"

namespace Supernova::Editor{

    struct FileEntry {
        std::string name;
        std::string type;
        bool isDirectory;
        intptr_t icon;
    };

    class ResourcesWindow{
    private:
        Project* project;
        CodeEditor* codeEditor;

        CommandHistory cmdHistory;

        bool firstOpen;
        bool requestSort;
        std::string currentPath;
        std::vector<Editor::FileEntry> files;

        Texture folderIcon;
        Texture fileIcon;

        int iconSize;
        float iconPadding;

        std::string lastSelectedFile;
        std::unordered_set<std::string> selectedFiles; // To track selected files
        bool ctrlPressed; // To handle multi-selection using CTRL key
        bool shiftPressed; // To handle range selection using SHIFT key

        bool isDragging;
        ImVec2 dragStart;
        ImVec2 dragEnd;
        ImVec2 windowPos;        // Store window position for coordinate conversion
        ImVec2 scrollOffset;     // Store scroll offset

        bool isDragDropTarget;
        bool isExternalDragHovering;

        std::vector<std::string> clipboardFiles;
        bool clipboardCut;

        char nameBuffer[256];

        bool isRenaming;
        std::string fileBeingRenamed;

        bool isCreatingNewDirectory;

        std::filesystem::file_time_type lastWriteTime;
        float timeSinceLastCheck;

        std::vector<FileEntry> scanDirectory(const std::string& path);
        void sortWithSortSpecs(ImGuiTableSortSpecs* sortSpecs, std::vector<FileEntry>& files);
        std::string shortenPath(const std::filesystem::path& path, float maxWidth);
        void highlightDragAndDrop();
        void handleInternalDragAndDrop(const std::string& targetDirectory);
        void handleNewDirectory();
        void handleRename();
        void copySelectedFiles(bool cut);
        void pasteFiles(const std::string& targetDirectory);

    public:
        ResourcesWindow(Project* project, CodeEditor* codeEditor);

        void handleExternalDragEnter();
        void handleExternalDragLeave();

        void show();
    };

}

#endif /* RESOURCESWINDOW_H */