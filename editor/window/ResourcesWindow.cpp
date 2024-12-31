#include "ResourcesWindow.h"

#include "external/IconsFontAwesome6.h"
#include "resources/icons/folder-icon_png.h"
#include "resources/icons/file-icon_png.h"

#include "imgui_internal.h"

using namespace Supernova;

namespace fs = std::filesystem;

Editor::ResourcesWindow::ResourcesWindow(Project* project){
    this->project = project;
    this->firstOpen = true;
    this->requestSort = true;
    this->iconSize = 32.0f;
    this->iconPadding = 1.5 * this->iconSize;
}

std::vector<Editor::FileEntry> Editor::ResourcesWindow::scanDirectory(const std::string& path, intptr_t folderIcon, intptr_t fileIcon) {
    currentPath = path;
    requestSort = true;

    std::vector<Editor::FileEntry> files;

    for (const auto& entry : fs::directory_iterator(path)) {
        FileEntry fileEntry;
        fileEntry.name = entry.path().filename().string();
        fileEntry.isDirectory = entry.is_directory();
        fileEntry.icon = entry.is_directory() ? folderIcon : fileIcon;
        if (!fileEntry.isDirectory) {
            fileEntry.type = entry.path().extension().string();
        } else {
            fileEntry.type = "";
        }
        files.push_back(fileEntry);
    }

    return files;
}

void Editor::ResourcesWindow::sortWithSortSpecs(ImGuiTableSortSpecs* sortSpecs, std::vector<FileEntry>& files) {
    if (!sortSpecs || sortSpecs->SpecsCount == 0) {
        // Default behavior: Sort directories first, then by name
        std::sort(files.begin(), files.end(), [](const FileEntry& a, const FileEntry& b) {
            if (a.isDirectory != b.isDirectory) {
                return a.isDirectory; // Directories come first
            }
            return a.name < b.name;
        });
        return;
    }

    auto comparator = [&](const FileEntry& a, const FileEntry& b) -> bool {
        // Always sort directories first
        if (a.isDirectory != b.isDirectory) {
            return a.isDirectory; // Directories come first
        }

        for (int i = 0; i < sortSpecs->SpecsCount; i++) {
            const ImGuiTableColumnSortSpecs& spec = sortSpecs->Specs[i];
            bool ascending = (spec.SortDirection == ImGuiSortDirection_Ascending);

            switch (spec.ColumnIndex) {
                case 0: // Column 0: "Name"
                    if (a.name != b.name) {
                        return ascending ? (a.name < b.name) : (a.name > b.name);
                    }
                    break;

                case 1: // Column 1: "Type"
                    if (a.type != b.type) {
                        return ascending ? (a.type < b.type) : (a.type > b.type);
                    }
                    break;

                default:
                    return false;
            }
        }

        return false;
    };

    std::sort(files.begin(), files.end(), comparator);
}

std::string Editor::ResourcesWindow::shortenPath(const std::filesystem::path& path, float maxWidth) {
    std::string fullPath = path.string();

    std::string projectPath = project->getProjectPath();
    if (fullPath.find(projectPath) == 0) {
        fullPath = fullPath.substr(projectPath.length());
        if (fullPath.empty()){
            fullPath = "/";
        }
    }

    ImVec2 fullPathSize = ImGui::CalcTextSize(fullPath.c_str());
    if (fullPathSize.x <= maxWidth) {
        return fullPath;
    }

    std::string filename = path.filename().string();
    std::string parentPath = path.parent_path().string();

    ImVec2 filenameSize = ImGui::CalcTextSize(filename.c_str());
    if (filenameSize.x > maxWidth) {
        std::string truncatedFilename = filename;
        while (!truncatedFilename.empty() && ImGui::CalcTextSize((truncatedFilename + "...").c_str()).x > maxWidth) {
            truncatedFilename.pop_back();
        }
        return truncatedFilename + "...";
    }

    float remainingWidth = maxWidth - filenameSize.x - ImGui::CalcTextSize("/").x; // Space for '/' separator

    if (parentPath != ".") {
        std::string truncatedParentPath = parentPath;
        while (!truncatedParentPath.empty() && ImGui::CalcTextSize((truncatedParentPath + ".../").c_str()).x > remainingWidth) {
            truncatedParentPath.pop_back();
        }

        return truncatedParentPath + ".../" + filename;
    } else {
        return filename;
    }
}

void Editor::ResourcesWindow::show() {
    if (firstOpen) {
        int iconWidth, iconHeight;

        TextureData data;

        data.loadTextureFromMemory(folder_12003793_png, folder_12003793_png_len);
        folderIcon.setData("editor:resources:folder_icon", data);
        folderIcon.load();

        data.loadTextureFromMemory(file_2521594_png, file_2521594_png_len);
        fileIcon.setData("editor:resources:file_icon", data);
        fileIcon.load();

        files = scanDirectory(project->getProjectPath(), (intptr_t)folderIcon.getRender()->getGLHandler(), (intptr_t)fileIcon.getRender()->getGLHandler());

        firstOpen = false;
    }

    ctrlPressed = ImGui::GetIO().KeyCtrl;
    shiftPressed = ImGui::GetIO().KeyShift;

    ImGui::Begin("Resources");

    bool clickedOutside = false;

    float windowWidth = ImGui::GetContentRegionAvail().x;
    float columnWidth = iconSize + iconPadding;

    int columns = static_cast<int>(windowWidth / columnWidth);
    if (columns < 1) columns = 1;

    ImGui::BeginDisabled(currentPath == project->getProjectPath());

    if (ImGui::Button(ICON_FA_HOUSE)) {
        files = scanDirectory(project->getProjectPath(), (intptr_t)folderIcon.getRender()->getGLHandler(), (intptr_t)fileIcon.getRender()->getGLHandler());
        selectedFiles.clear();
    }
    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_ANGLE_LEFT)) {
        if (!currentPath.empty() && currentPath != project->getProjectPath()) {
            fs::path parentPath = fs::path(currentPath).parent_path();
            currentPath = parentPath.string();
            files = scanDirectory(currentPath, (intptr_t)folderIcon.getRender()->getGLHandler(), (intptr_t)fileIcon.getRender()->getGLHandler());
            selectedFiles.clear();
        }
    }

    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 50, 50, 255));
    ImGui::BeginChild("PathFrame", ImVec2(-ImGui::CalcTextSize(ICON_FA_COPY).x - ImGui::GetStyle().ItemSpacing.x - ImGui::GetStyle().FramePadding.x * 2, ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    std::string shortenedPath = shortenPath(currentPath, ImGui::GetContentRegionAvail().x);

    ImGui::SetCursorPosY(ImGui::GetStyle().FramePadding.y);
    ImGui::Text("%s", ((shortenedPath == ".") ? "" : shortenedPath).c_str());
    ImGui::SetItemTooltip("%s", currentPath.c_str());

    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_GEAR)) {
        ImGui::OpenPopup("SettingsPopup");
    }

    if (ImGui::BeginPopup("SettingsPopup")) {
        ImGui::Text("Settings");
        ImGui::Separator();

        if (ImGui::SliderInt("Icon Size", &iconSize, 16.0f, 128.0f)) {
            iconPadding = 1.5 * iconSize;
        }

        ImGui::EndPopup();
    }

    ImGui::Separator();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGuiTableFlags table_flags_for_sort_specs = ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders;
    if (ImGui::BeginTable("for_sort_specs_only", 2, table_flags_for_sort_specs, ImVec2(0.0f, ImGui::GetFrameHeight()))) {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Type");
        ImGui::TableHeadersRow();
        if (ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs()) {
            if (sort_specs->SpecsDirty || requestSort) {
                sortWithSortSpecs(sort_specs, files);
                sort_specs->SpecsDirty = requestSort = false;
            }
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleVar();

    ImGui::BeginChild("FileTableScrollRegion", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(8.0f, 8.0f));

    // Deferred deletion
    static bool showDeleteConfirmation = false;

    if (ImGui::BeginTable("FileTable", columns, ImGuiTableFlags_SizingStretchSame)) {
        for (auto& file : files) {
            ImGui::TableNextColumn();

            ImGui::PushID(file.name.c_str());

            float itemSpacingY = ImGui::GetStyle().ItemSpacing.y;
            float cellWidth = ImGui::GetContentRegionAvail().x;
            ImVec2 textSize = ImGui::CalcTextSize(file.name.c_str(), nullptr, true, cellWidth);
            float celHeight = iconSize + itemSpacingY + textSize.y;

            ImVec2 selectableSize(cellWidth, celHeight);

            ImGui::BeginGroup();
            bool isSelected = selectedFiles.find(file.name) != selectedFiles.end();

            // Handle left-click and right-click for selection
            if (ImGui::Selectable("", isSelected, ImGuiSelectableFlags_AllowDoubleClick, selectableSize)) {
                clickedOutside = true;

                if (ctrlPressed) {
                    if (isSelected) {
                        selectedFiles.erase(file.name);
                    } else {
                        selectedFiles.insert(file.name);
                    }
                    lastSelectedFile = file.name;
                } else if (shiftPressed) {
                    if (!lastSelectedFile.empty()) {
                        auto itStart = std::find_if(files.begin(), files.end(), [&](const FileEntry& entry) {
                            return entry.name == lastSelectedFile;
                        });

                        auto itEnd = std::find_if(files.begin(), files.end(), [&](const FileEntry& entry) {
                            return entry.name == file.name;
                        });

                        if (itStart != files.end() && itEnd != files.end()) {
                            if (itStart > itEnd) std::swap(itStart, itEnd);

                            for (auto it = itStart; it <= itEnd; ++it) {
                                selectedFiles.insert(it->name);
                            }
                        }
                    } else {
                        selectedFiles.insert(file.name);
                    }
                } else {
                    selectedFiles.clear();
                    selectedFiles.insert(file.name);
                    lastSelectedFile = file.name;
                }

                if (ImGui::IsMouseDoubleClicked(0) && file.isDirectory) {
                    files = scanDirectory(currentPath + "/" + file.name, (intptr_t)folderIcon.getRender()->getGLHandler(), (intptr_t)fileIcon.getRender()->getGLHandler());
                    selectedFiles.clear();
                    ImGui::EndGroup();
                    ImGui::PopID();
                    break;
                }
            }

            // Handle right-click for selection and open the context menu
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                // Right-click selects the file
                if (!ctrlPressed && !isSelected) {
                    selectedFiles.clear();
                }
                selectedFiles.insert(file.name);
                lastSelectedFile = file.name;

                // Open the context menu
                ImGui::OpenPopup("FileContextMenu");
            }

            if (ImGui::BeginPopup("FileContextMenu")) {
                if (ImGui::MenuItem("Delete")) {
                    // Trigger the confirmation dialog
                    showDeleteConfirmation = true;
                }

                if (ImGui::MenuItem("Rename")) {
                    std::string newName = "new_name.txt";  // Replace with user input logic
                    std::filesystem::rename(currentPath + "/" + file.name, currentPath + "/" + newName);
                    files = scanDirectory(currentPath, (intptr_t)folderIcon.getRender()->getGLHandler(), (intptr_t)fileIcon.getRender()->getGLHandler());
                }

                ImGui::EndPopup();
            }

            float iconOffsetX = (cellWidth - iconSize) / 2;
            float iconOffsetY = selectableSize.y + itemSpacingY;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + iconOffsetX);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - iconOffsetY);
            ImGui::Image((ImTextureID)file.icon, ImVec2(iconSize, iconSize));

            float textOffsetX = (cellWidth / 2) - (textSize.x / 2);
            if (textOffsetX < 0) textOffsetX = 0;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffsetX);
            ImGui::TextWrapped("%s", file.name.c_str());

            ImGui::EndGroup();

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleVar();

    if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered() && !clickedOutside) {
        selectedFiles.clear();
    }

    ImGui::EndChild();

    ImGui::End();

    // Show confirmation dialog
    if (showDeleteConfirmation) {
        ImGui::OpenPopup("Delete Confirmation");
    }

    if (ImGui::BeginPopupModal("Delete Confirmation", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to delete the selected files?");
        ImGui::Separator();

        if (ImGui::Button("Yes", ImVec2(120, 0))) {
            // Delete selected files
            for (const auto& fileName : selectedFiles) {
                std::filesystem::remove(currentPath + "/" + fileName);
            }

            // Clear the selection set
            selectedFiles.clear();

            // Refresh the files list
            files = scanDirectory(currentPath, (intptr_t)folderIcon.getRender()->getGLHandler(), (intptr_t)fileIcon.getRender()->getGLHandler());

            // Close the modal
            showDeleteConfirmation = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();

        if (ImGui::Button("No", ImVec2(120, 0))) {
            // Close the modal without deleting
            showDeleteConfirmation = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}