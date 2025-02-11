#include "CodeEditor.h"

using namespace Supernova::Editor;

CodeEditor::CodeEditor() : showEditor(true) {
}

CodeEditor::~CodeEditor() {
    delete editor;
}

void CodeEditor::setText(const std::string& text) {
    editor->SetText(text);
}

std::string CodeEditor::getText() const {
    return editor->GetText();
}

void CodeEditor::setup() {
    editor = new TextEditor();

    editor->SetLanguageDefinition(TextEditor::LanguageDefinitionId::Cpp);
    editor->SetPalette(TextEditor::PaletteId::Dark);
    
    // Set basic editor properties
    editor->SetTabSize(4);
}

void CodeEditor::show() {
    if (!showEditor) return;

    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Code Editor", &showEditor)) {
        //int line, column;
        //editor->GetCursorPosition(line, column);
        
        //ImGui::Text("%6d/%-6d %6d lines  | %s | %s", 
        //    line + 1, 
        //    column + 1,
        //    (int)editor->GetTextLines().size(),
        //    editor->IsOverwriteEnabled() ? "Ovr" : "Ins",
        //    editor->CanUndo() ? "*" : " ");

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // font need to be monospace
        editor->Render("TextEditor");
        ImGui::PopFont();
    }
    ImGui::End();

}