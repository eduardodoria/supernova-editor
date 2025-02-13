#pragma once
#include "imgui.h"
#include "TextEditor.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <filesystem>

namespace Supernova::Editor {
    struct EditorInstance {
        std::unique_ptr<TextEditor> editor;
        bool isOpen;
        std::string filepath;
        std::filesystem::file_time_type lastWriteTime;
        bool isModified;

        EditorInstance() : isOpen(true), isModified(false) {}
    };

    class CodeEditor {
    private:
        std::unordered_map<std::string, EditorInstance> editors;

        void checkFileChanges(EditorInstance& instance);
        bool loadFileContent(EditorInstance& instance);

    public:
        CodeEditor();
        ~CodeEditor();

        void openFile(const std::string& filepath);
        void closeFile(const std::string& filepath);
        bool isFileOpen(const std::string& filepath) const;
        void setText(const std::string& filepath, const std::string& text);
        std::string getText(const std::string& filepath) const;

        void setup();
        void show();
    };
}