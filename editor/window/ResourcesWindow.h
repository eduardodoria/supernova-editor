#ifndef RESOURCESWINDOW_H
#define RESOURCESWINDOW_H

#include "Project.h"
#include "command/CommandHistory.h"
#include "window/CodeEditor.h"

#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <unordered_map>
#include <condition_variable>

#include "imgui.h"

namespace fs = std::filesystem;

namespace Supernova::Editor {

    struct FileEntry {
        std::string name;
        std::string type;
        bool isDirectory;
        intptr_t icon;
        bool isImage;
        bool hasThumbnail;
        std::string thumbnailPath;
    };

    class ResourcesWindow {
    private:
        Project* project;
        CodeEditor* codeEditor;

        enum class LayoutType {
            AUTO,  // Automatically switch between GRID and SPLIT based on window size
            GRID,  // Current single-column list view
            SPLIT   // New two-column tree view
        };
        LayoutType currentLayout = LayoutType::AUTO;
        float layoutAutoThreshold = 600.0f;
        float leftPanelWidth = 200.0f;  // Width of the left panel in SPLIT layout

        CommandHistory cmdHistory;

        bool firstOpen;
        bool requestSort;
        fs::path currentPath;
        std::vector<Editor::FileEntry> files;

        Texture folderIcon;
        Texture fileIcon;
        std::unordered_map<std::string, Texture> thumbnailTextures;

        int iconSize;
        float iconPadding;

        std::string lastSelectedFile;
        std::unordered_set<std::string> selectedFiles;
        bool ctrlPressed;
        bool shiftPressed;

        bool isDragging;
        ImVec2 dragStart;
        ImVec2 dragEnd;
        ImVec2 windowPos;
        ImVec2 scrollOffset;

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

        bool windowFocused;

        bool showDeleteConfirmation;

        // Thumbnail generation
        std::thread thumbnailThread;
        std::mutex thumbnailMutex;
        std::queue<fs::path> thumbnailQueue;
        std::atomic<bool> stopThumbnailThread;
        std::condition_variable thumbnailCondition;

        // Queue for completed thumbnails
        std::mutex completedThumbnailMutex;
        std::queue<fs::path> completedThumbnailQueue;

        void renderHeader();
        void renderFileListing(bool showDirectories);
        void renderDirectoryTree(const fs::path& path);

        void scanDirectory(const fs::path& path);
        void sortWithSortSpecs(ImGuiTableSortSpecs* sortSpecs, std::vector<FileEntry>& files);
        void highlightDragAndDrop();
        void handleInternalDragAndDrop(const fs::path& targetDirectory);
        void handleNewDirectory();
        void handleRename();
        void copySelectedFiles(bool cut);
        void pasteFiles(const fs::path& targetDirectory);

        bool isImageFile(const std::string& extension) const;
        void queueThumbnailGeneration(const fs::path& filePath, const std::string& extension);
        void thumbnailWorker();
        fs::path getThumbnailPath(const fs::path& originalPath) const;
        void loadThumbnail(FileEntry& entry);

    public:
        ResourcesWindow(Project* project, CodeEditor* codeEditor);
        ~ResourcesWindow();

        bool isFocused() const;

        void notifyProjectPathChange();

        void handleExternalDragEnter();
        void handleExternalDragLeave();

        void cleanupThumbnails();

        void show();
    };

}

#endif /* RESOURCESWINDOW_H */