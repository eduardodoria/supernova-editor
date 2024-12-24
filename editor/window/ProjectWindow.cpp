#include "ProjectWindow.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <filesystem> // C++17 filesystem library
#include "stb_image.h" // For loading icons

#include "imgui.h"
#include "imgui_internal.h"

using namespace Supernova;

namespace fs = std::filesystem;

Editor::ProjectWindow::ProjectWindow(Project* project){
    this->project = project;
    this->firstOpen = true;
}

void* Editor::ProjectWindow::LoadTexture(const char* filePath, int& outWidth, int& outHeight) {
    int channels;
    unsigned char* data = stbi_load(filePath, &outWidth, &outHeight, &channels, 4);
    if (!data) {
        return nullptr;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, outWidth, outHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return reinterpret_cast<void*>(static_cast<intptr_t>(texture));
}

// Function to release textures
void Editor::ProjectWindow::FreeTexture(void* textureID) {
    GLuint texture = static_cast<GLuint>(reinterpret_cast<intptr_t>(textureID));
    glDeleteTextures(1, &texture);
}

// Function to scan a directory and populate file entries
std::vector<Editor::FileEntry> Editor::ProjectWindow::ScanDirectory(const std::string& path, void* folderIcon, void* fileIcon) {
    currentPath = path;
    
    std::vector<Editor::FileEntry> entries;

    for (const auto& entry : fs::directory_iterator(path)) {
        FileEntry fileEntry;
        fileEntry.name = entry.path().filename().string();
        fileEntry.isDirectory = entry.is_directory();
        fileEntry.icon = entry.is_directory() ? folderIcon : fileIcon;
        entries.push_back(fileEntry);
    }

    return entries;
}

void Editor::ProjectWindow::show(){
    if (firstOpen){
        int iconWidth, iconHeight;
        void* folderIcon = LoadTexture("folder_icon.png", iconWidth, iconHeight); // Replace with your folder icon path
        void* fileIcon = LoadTexture("file_icon.png", iconWidth, iconHeight); 

        entries = ScanDirectory(".", folderIcon, fileIcon);

        firstOpen = false;
    }

    ImGui::Begin("Project");

    // Calculate the number of columns based on the window width
    float windowWidth = ImGui::GetContentRegionAvail().x;
    float iconSize = 32.0f;       // Icon size
    float padding = 48.0f;       // Padding between columns
    float columnWidth = iconSize + padding;

    int columns = static_cast<int>(windowWidth / columnWidth);
    if (columns < 1) columns = 1; // Ensure at least one column

    // Display current path
    ImGui::Text("Current Path: %s", currentPath.c_str());
    ImGui::Separator();

    // Begin table for dynamic columns
    if (ImGui::BeginTable("FileTable", columns, ImGuiTableFlags_SizingStretchSame)) {
        for (const auto& entry : entries) {
            // Begin a new table cell
            ImGui::TableNextColumn();

            // Push a unique ID for each item
            ImGui::PushID(entry.name.c_str());

            float cellWidth = ImGui::GetContentRegionAvail().x;
            ImVec2 textSize = ImGui::CalcTextSize(entry.name.c_str(), nullptr, true, cellWidth);
            float padding = ImGui::GetStyle().ItemSpacing.y;
            float selectableHeight = iconSize + textSize.y + padding;

            ImGui::BeginGroup(); // Group the icon and text
            if (ImGui::Selectable("", false, 0, ImVec2(cellWidth, selectableHeight))) {
                if (entry.isDirectory) {
                    // Navigate into the directory
                    //entries = ScanDirectory(path + "/" + entry.name, entries[0].icon, entries[1].icon);
                    //break; // Exit loop to update the UI
                } else {
                    // Handle file selection
                    printf("Selected file: %s\n", entry.name.c_str());
                }
            }

            float iconX = (cellWidth - iconSize) / 2;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + iconX);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - selectableHeight - padding);
            ImGui::Image((ImTextureID)(intptr_t)entry.icon, ImVec2(iconSize, iconSize));
            float textOffsetX = (cellWidth / 2) - (textSize.x / 2);
            if (textOffsetX < 0) textOffsetX = 0;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffsetX);
            ImGui::TextWrapped("%s", entry.name.c_str());
            ImGui::EndGroup();

            // Pop the unique ID for this item
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    ImGui::End();
}