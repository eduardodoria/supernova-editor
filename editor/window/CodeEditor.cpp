#include "CodeEditor.h"
#include <filesystem>
#include <fstream>
#include <sstream>

using namespace Supernova::Editor;
namespace fs = std::filesystem;

CodeEditor::CodeEditor() {
}

CodeEditor::~CodeEditor() {
}

bool CodeEditor::loadFileContent(EditorInstance& instance) {
    try {
        std::ifstream file(instance.filepath);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();

            instance.editor->SetText(buffer.str());
            instance.lastWriteTime = fs::last_write_time(instance.filepath);
            instance.isModified = false;
            return true;
        }
    } catch (const std::exception& e) {
        // Handle file access errors
    }
    return false;
}

void CodeEditor::checkFileChanges(EditorInstance& instance) {
    try {
        auto currentWriteTime = fs::last_write_time(instance.filepath);

        if (currentWriteTime != instance.lastWriteTime) {
            // File has been modified externally
            ImGui::OpenPopup("File Changed");

            // Center popup
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

            if (ImGui::BeginPopupModal("File Changed", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("The file '%s' has been modified externally.", 
                    fs::path(instance.filepath).filename().string().c_str());
                ImGui::Text("Do you want to reload it?");
                ImGui::Separator();

                float buttonWidth = 120.0f;
                float windowWidth = ImGui::GetWindowSize().x;

                ImGui::SetCursorPosX((windowWidth - buttonWidth * 2 - ImGui::GetStyle().ItemSpacing.x) * 0.5f);

                if (ImGui::Button("Yes", ImVec2(buttonWidth, 0))) {
                    loadFileContent(instance);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("No", ImVec2(buttonWidth, 0))) {
                    // Update the timestamp without reloading to prevent further popups
                    instance.lastWriteTime = currentWriteTime;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
    } catch (const std::exception& e) {
        // Handle file access errors
    }
}

void CodeEditor::openFile(const std::string& filepath) {
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

    // Load the file content
    if (!loadFileContent(instance)) {
        // If loading fails, remove the instance
        editors.erase(filepath);
    }
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
        it->second.isModified = false;
        try {
            it->second.lastWriteTime = fs::last_write_time(filepath);
        } catch (const std::exception& e) {
            // Handle file access errors
        }
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

        // Check for external file changes
        checkFileChanges(instance);

        // Create window title using filename
        std::string filename = std::filesystem::path(instance.filepath).filename().string();
        std::string windowTitle = filename + "###" + instance.filepath;

        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(windowTitle.c_str(), &instance.isOpen)) {
            // Track modifications by checking if undo is available
            if (instance.editor->CanUndo() && !instance.isModified) {
                instance.isModified = true;
            }

            int line, column;
            instance.editor->GetCursorPosition(line, column);

            ImGui::Text("%6d/%-6d %6d lines  | %s | %s", 
                line + 1, 
                column + 1,
                (int)instance.editor->GetTextLines().size(),
                instance.editor->GetLanguageDefinitionName(),
                instance.isModified ? "*" : " ");

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