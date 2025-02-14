#pragma once
#include "imgui.h"
#include "TextEditor.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <filesystem>

namespace fs = std::filesystem;

namespace Supernova::Editor {

    struct EditorInstance {
        std::unique_ptr<TextEditor> editor;
        bool isOpen;
        fs::path filepath;
        fs::file_time_type lastWriteTime;
        bool isModified;
        double lastCheckTime;
        bool hasExternalChanges;
        int savedUndoIndex;

        EditorInstance() : isOpen(true), isModified(false), lastCheckTime(0.0), hasExternalChanges(false), savedUndoIndex(0) {}
    };

    class CodeEditor {
    private:
        std::unordered_map<std::string, EditorInstance> editors;
        struct PendingFileChange {
            fs::path filepath;
            fs::file_time_type newWriteTime;
        };
        std::vector<PendingFileChange> changedFilesQueue;
        bool isFileChangePopupOpen;
        bool windowFocused;

        void checkFileChanges(EditorInstance& instance);
        bool loadFileContent(EditorInstance& instance);
        void handleFileChangePopup();
        std::string getWindowTitle(const EditorInstance& instance) const;

    public:
        CodeEditor();
        ~CodeEditor();

        std::vector<fs::path> getOpenPaths() const;
        bool isFocused() const;

        bool save(const std::string& filepath);
        bool save(EditorInstance& instance);
        void saveAll();

        void openFile(const std::string& filepath);
        void closeFile(const std::string& filepath);
        bool isFileOpen(const std::string& filepath) const;
        void setText(const std::string& filepath, const std::string& text);
        std::string getText(const std::string& filepath) const;

        void show();
    };
}