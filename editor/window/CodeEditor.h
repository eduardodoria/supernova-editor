#pragma once
#include "imgui.h"
#include "TextEditor.h"
#include <string>

namespace Supernova::Editor {
    class CodeEditor {
    private:
        TextEditor* editor;
        bool showEditor;

    public:
        CodeEditor();
        ~CodeEditor();
        
        void setText(const std::string& text);
        std::string getText() const;

        void setup();
        void show();
    };
}