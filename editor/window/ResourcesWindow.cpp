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

    // Calculate the size of the full path
    ImVec2 fullPathSize = ImGui::CalcTextSize(fullPath.c_str());
    if (fullPathSize.x <= maxWidth) {
        return fullPath;
    }

    // Get the filename and parent path
    std::string filename = path.filename().string();
    std::string parentPath = path.parent_path().string();

    // Calculate the size of the filename
    ImVec2 filenameSize = ImGui::CalcTextSize(filename.c_str());
    if (filenameSize.x > maxWidth) {
        // Truncate filename if it alone exceeds maxWidth
        std::string truncatedFilename = filename;
        while (!truncatedFilename.empty() && ImGui::CalcTextSize((truncatedFilename + "...").c_str()).x > maxWidth) {
            truncatedFilename.pop_back();
        }
        return truncatedFilename + "...";
    }

    // Determine how much space remains for the parent path
    float remainingWidth = maxWidth - filenameSize.x - ImGui::CalcTextSize("/").x; // Space for '/' separator

    // Truncate the parent path if necessary
    if (parentPath != "."){
        std::string truncatedParentPath = parentPath;
        while (!truncatedParentPath.empty() && ImGui::CalcTextSize((truncatedParentPath + ".../").c_str()).x > remainingWidth) {
            truncatedParentPath.pop_back();
        }

        return truncatedParentPath + ".../" + filename;
    }else{
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

        files = scanDirectory(".", (intptr_t)folderIcon.getRender()->getGLHandler(), (intptr_t)fileIcon.getRender()->getGLHandler());

        firstOpen = false;
    }

    // Check if CTRL or SHIFT is pressed
    ctrlPressed = ImGui::GetIO().KeyCtrl;
    shiftPressed = ImGui::GetIO().KeyShift;

    ImGui::Begin("Resources");

    // Calculate the number of columns based on the window width
    float windowWidth = ImGui::GetContentRegionAvail().x;
    float iconSize = 32.0f;       // Icon size
    float padding = 48.0f;       // Padding between columns
    float columnWidth = iconSize + padding;

    int columns = static_cast<int>(windowWidth / columnWidth);
    if (columns < 1) columns = 1; // Ensure at least one column

    ImGui::BeginDisabled(currentPath == ".");

    if (ImGui::Button(ICON_FA_HOUSE)){
        files = scanDirectory(".", (intptr_t)folderIcon.getRender()->getGLHandler(), (intptr_t)fileIcon.getRender()->getGLHandler());
    }
    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_ANGLE_LEFT)){
        if (!currentPath.empty() && currentPath != ".") {
            fs::path parentPath = fs::path(currentPath).parent_path();
            currentPath = parentPath.string();
            files = scanDirectory(currentPath, (intptr_t)folderIcon.getRender()->getGLHandler(), (intptr_t)fileIcon.getRender()->getGLHandler());
        }
    }

    ImGui::EndDisabled();

    // ------- path part --------
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 50, 50, 255)); // Dark gray background
    ImGui::BeginChild("PathFrame", ImVec2(-ImGui::CalcTextSize(ICON_FA_COPY).x - ImGui::GetStyle().ItemSpacing.x - ImGui::GetStyle().FramePadding.x * 2, ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y*2), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    std::string shortenedPath = shortenPath(currentPath, ImGui::GetContentRegionAvail().x);

    ImGui::SetCursorPosY(ImGui::GetStyle().FramePadding.y);
    ImGui::Text("%s", ((shortenedPath==".")?"":shortenedPath).c_str());

    ImGui::EndChild();
    ImGui::PopStyleColor();
    // ------- path part --------

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_GEAR)) {
        ImGui::SetClipboardText(currentPath.c_str());
    }

    ImGui::Separator();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGuiTableFlags table_flags_for_sort_specs = ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders;
    if (ImGui::BeginTable("for_sort_specs_only", 2, table_flags_for_sort_specs, ImVec2(0.0f, ImGui::GetFrameHeight())))
    {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Type");
        ImGui::TableHeadersRow();
        if (ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs()){
            if (sort_specs->SpecsDirty || requestSort){
                sortWithSortSpecs(sort_specs, files);
                sort_specs->SpecsDirty = requestSort = false;
            }
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleVar();

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(8.0f, 8.0f));

    // Begin table for dynamic columns
    if (ImGui::BeginTable("FileTable", columns, ImGuiTableFlags_SizingStretchSame)) {
        for (const auto& file : files) {
            // Begin a new table cell
            ImGui::TableNextColumn();

            // Push a unique ID for each item
            ImGui::PushID(file.name.c_str());

            float itemSpacingY = ImGui::GetStyle().ItemSpacing.y;
            float cellWidth = ImGui::GetContentRegionAvail().x;
            ImVec2 textSize = ImGui::CalcTextSize(file.name.c_str(), nullptr, true, cellWidth);
            float celHeight = iconSize + itemSpacingY + textSize.y; // Fixed height for the selectable area

            ImVec2 selectableSize(cellWidth, celHeight);

            ImGui::BeginGroup(); // Group the icon and text
            bool isSelected = selectedFiles.find(file.name) != selectedFiles.end();

            if (ImGui::Selectable("", isSelected, ImGuiSelectableFlags_AllowDoubleClick, selectableSize)) {
                if (ctrlPressed) {
                    // Toggle selection for this file
                    if (isSelected) {
                        selectedFiles.erase(file.name);
                    } else {
                        selectedFiles.insert(file.name);
                    }
                } else if (shiftPressed) {
                    // Handle range selection (not implemented in this example, but could be added)
                    // Example: Select all files between the last selected file and this one
                } else {
                    // Clear previous selections and select this file
                    selectedFiles.clear();
                    selectedFiles.insert(file.name);
                }

                if (ImGui::IsMouseDoubleClicked(0) && file.isDirectory) {
                    // Navigate into the directory
                    files = scanDirectory(currentPath + "/" + file.name, (intptr_t)folderIcon.getRender()->getGLHandler(), (intptr_t)fileIcon.getRender()->getGLHandler());
                    selectedFiles.clear();
                    ImGui::EndGroup();
                    ImGui::PopID();
                    break; // Exit loop to update the UI
                } else {
                    // Handle file selection
                    printf("Selected file: %s\n", file.name.c_str());
                }
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

            // Pop the unique ID for this item
            ImGui::PopID();
        }
        ImGui::EndTable();
        ImGui::PopStyleVar();
    }

    ImGui::End();
}