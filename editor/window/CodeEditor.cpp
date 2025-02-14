#include "CodeEditor.h"

#include "Backend.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace Supernova::Editor;
namespace fs = std::filesystem;

CodeEditor::CodeEditor() : isFileChangePopupOpen(false) {
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
            if (instance.isModified) {
                // Check if this file is already in the queue
                auto it = std::find_if(changedFilesQueue.begin(), changedFilesQueue.end(),
                    [&](const PendingFileChange& change) {
                        return change.filepath == instance.filepath;
                    });

                // Only add to queue if not already present
                if (it == changedFilesQueue.end()) {
                    changedFilesQueue.push_back({instance.filepath, currentWriteTime});
                }
            } else {
                // If no unsaved changes, silently reload the file
                loadFileContent(instance);
            }
        }
    } catch (const std::exception& e) {
        // Handle file access errors
    }
}

void CodeEditor::handleFileChangePopup() {
    if (changedFilesQueue.empty()) {
        return;
    }

    if (!isFileChangePopupOpen) {
        ImGui::OpenPopup("Files Changed###FilesChanged");
        isFileChangePopupOpen = true;
    }

    // Center popup
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Files Changed###FilesChanged", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (changedFilesQueue.size() == 1) {
            ImGui::Text("The file '%s' has been modified externally.", 
                changedFilesQueue[0].filepath.filename().string().c_str());
        } else {
            ImGui::Text("%zu files have been modified externally:", changedFilesQueue.size());

            // Display up to 10 files
            int fileCount = 0;
            for (const auto& change : changedFilesQueue) {
                if (fileCount < 10) {
                    ImGui::BulletText("%s", change.filepath.filename().string().c_str());
                }
                fileCount++;
            }

            // If there are more files, show a message
            if (fileCount > 10) {
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "And %zu more files...", changedFilesQueue.size() - 10);
            }
        }

        ImGui::Text("Do you want to reload the modified files?");
        ImGui::Text("Warning: You have unsaved changes that will be lost.");
        ImGui::Separator();

        float buttonWidth = 120.0f;
        float windowWidth = ImGui::GetWindowSize().x;

        ImGui::SetCursorPosX((windowWidth - buttonWidth * 2 - ImGui::GetStyle().ItemSpacing.x) * 0.5f);

        if (ImGui::Button("Yes", ImVec2(buttonWidth, 0))) {
            // Reload all files in the queue
            for (const auto& change : changedFilesQueue) {
                auto it = editors.find(change.filepath);
                if (it != editors.end()) {
                    loadFileContent(it->second);
                }
            }
            changedFilesQueue.clear();
            isFileChangePopupOpen = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("No", ImVec2(buttonWidth, 0))) {
            // Update timestamps without reloading
            for (const auto& change : changedFilesQueue) {
                auto it = editors.find(change.filepath);
                if (it != editors.end()) {
                    it->second.lastWriteTime = change.newWriteTime;
                }
            }
            changedFilesQueue.clear();
            isFileChangePopupOpen = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    } else {
        isFileChangePopupOpen = false;
    }
}

std::string CodeEditor::getWindowTitle(const EditorInstance& instance) const{
    std::string filename = instance.filepath.filename().string();
    return filename + "###" + instance.filepath.string();
}

std::vector<fs::path> CodeEditor::getOpenPaths() const{
    std::vector<fs::path> openPaths;
    for (auto it = editors.begin(); it != editors.end();) {
        const auto& instance = it->second;

        openPaths.push_back(instance.filepath);
    }

    return openPaths;
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

    Backend::getApp().addNewCodeWindowToDock(instance.filepath);
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

void CodeEditor::show() {
    // Get current time
    double currentTime = ImGui::GetTime();

    // Iterate through all open editors
    for (auto it = editors.begin(); it != editors.end();) {
        auto& instance = it->second;

        // Check for external file changes every second
        if (currentTime - instance.lastCheckTime >= 1.0) {
            checkFileChanges(instance);
            instance.lastCheckTime = currentTime;
        }

        // Create window title using filename
        std::string windowTitle = getWindowTitle(instance);

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

    // Handle file change popup after all windows are rendered
    handleFileChangePopup();
}