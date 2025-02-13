#include "CodeEditor.h"
#include <filesystem>

using namespace Supernova::Editor;

CodeEditor::CodeEditor() {
}

CodeEditor::~CodeEditor() {
}

void CodeEditor::openFile(const std::string& filepath, const std::string& content) {
    if (editors.find(filepath) != editors.end()) {
        // File already open
        return;
    }

    auto& instance = editors[filepath];
    instance.filepath = filepath;
    instance.editor = std::make_unique<TextEditor>();

    // Setup the editor instance
    instance.editor->SetLanguageDefinition(TextEditor::LanguageDefinitionId::Cpp);
    instance.editor->SetPalette(TextEditor::PaletteId::Dark);
    instance.editor->SetTabSize(4);
    instance.editor->SetText(content);
}

void CodeEditor::closeFile(const std::string& filepath) {
    editors.erase(filepath);
}

bool CodeEditor::isFileOpen(const std::string& filepath) const {
    return editors.find(filepath) != editors.end();
}

void CodeEditor::setText(const std::string& filepath, const std::string& text) {
    if (auto it = editors.find(filepath); it != editors.end()) {
        it->second.editor->SetText(text);
    }
}

std::string CodeEditor::getText(const std::string& filepath) const {
    if (auto it = editors.find(filepath); it != editors.end()) {
        return it->second.editor->GetText();
    }
    return "";
}

void CodeEditor::setup() {
    // Global setup if needed
}

void CodeEditor::show() {
    // Iterate through all open editors
    for (auto it = editors.begin(); it != editors.end();) {
        auto& instance = it->second;

        // Create window title using filename
        std::string filename = std::filesystem::path(instance.filepath).filename().string();
        std::string windowTitle = filename + "###" + instance.filepath;

        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(windowTitle.c_str(), &instance.isOpen)) {
            int line, column;
            instance.editor->GetCursorPosition(line, column);

            ImGui::Text("%6d/%-6d %6d lines  | %s | %s", 
                line + 1, 
                column + 1,
                (int)instance.editor->GetTextLines().size(),
                instance.editor->GetLanguageDefinitionName(),
                instance.editor->CanUndo() ? "*" : " ");

            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // font needs to be monospace
            instance.editor->Render("TextEditor");
            ImGui::PopFont();
        }
        ImGui::End();

        // If the window was closed, remove it from our editors map
        if (!instance.isOpen) {
            it = editors.erase(it);
        } else {
            ++it;
        }
    }
}