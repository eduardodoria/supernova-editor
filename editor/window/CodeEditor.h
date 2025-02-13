#pragma once
#include "imgui.h"
#include "TextEditor.h"
#include <string>
#include <unordered_map>
#include <memory>

namespace Supernova::Editor {
    struct EditorInstance {
        std::unique_ptr<TextEditor> editor;
        bool isOpen;
        std::string filepath;

        EditorInstance() : isOpen(true) {}
    };

    class CodeEditor {
    private:
        std::unordered_map<std::string, EditorInstance> editors;

    public:
        CodeEditor();
        ~CodeEditor();

        void openFile(const std::string& filepath, const std::string& content);
        void closeFile(const std::string& filepath);
        bool isFileOpen(const std::string& filepath) const;
        void setText(const std::string& filepath, const std::string& text);
        std::string getText(const std::string& filepath) const;

        void setup();
        void show();
    };
}