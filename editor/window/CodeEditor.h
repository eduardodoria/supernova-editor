#pragma once
#include "imgui.h"
#include "TextEditor.h"
#include "Project.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <filesystem>

namespace fs = std::filesystem;

namespace Supernova::Editor {

    enum class LanguageType {
        CPP,
        LUA,
        UNKNOWN
    };

    struct EditorInstance {
        std::unique_ptr<TextEditor> editor;
        bool isOpen;
        fs::path filepath;
        LanguageType languageType;
        fs::file_time_type lastWriteTime;
        bool isModified;
        double lastCheckTime;
        bool hasExternalChanges;
        int savedUndoIndex;

        EditorInstance() : isOpen(true), languageType(LanguageType::UNKNOWN), isModified(false), lastCheckTime(0.0), hasExternalChanges(false), savedUndoIndex(0) {}
    };

    class CodeEditor {
    private:
        Project* project;

        std::unordered_map<std::string, EditorInstance> editors;
        struct PendingFileChange {
            fs::path filepath;
            fs::file_time_type newWriteTime;
        };
        std::vector<PendingFileChange> changedFilesQueue;
        bool isFileChangePopupOpen;
        bool windowFocused;
        EditorInstance* lastFocused;

        void checkFileChanges(EditorInstance& instance);
        bool loadFileContent(EditorInstance& instance);
        void handleFileChangePopup();
        std::string getWindowTitle(const EditorInstance& instance) const;
        void updateScriptProperties(const EditorInstance& instance);

    public:
        CodeEditor(Project* project);
        ~CodeEditor();

        std::vector<fs::path> getOpenPaths() const;
        bool isFocused() const;

        bool save(const std::string& filepath);
        bool save(EditorInstance& instance);
        void saveLastFocused();
        void saveAll();

        bool hasUnsavedChanges() const;
        bool hasLastFocusedUnsavedChanges() const;

        void openFile(const std::string& filepath);
        void closeFile(const std::string& filepath);
        bool isFileOpen(const std::string& filepath) const;
        void setText(const std::string& filepath, const std::string& text);
        std::string getText(const std::string& filepath) const;
        bool handleFileRename(const fs::path& oldPath, const fs::path& newPath);

        void show();
    };
}