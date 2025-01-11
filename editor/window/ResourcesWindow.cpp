#include "ResourcesWindow.h"

#include "external/IconsFontAwesome6.h"
#include "resources/icons/folder-icon_png.h"
#include "resources/icons/file-icon_png.h"

#include "Backend.h"

#include "imgui_internal.h"
#include "nfd.hpp"

using namespace Supernova;

namespace fs = std::filesystem;

Editor::ResourcesWindow::ResourcesWindow(Project* project) {
    this->project = project;
    this->firstOpen = true;
    this->requestSort = true;
    this->iconSize = 32.0f;
    this->iconPadding = 1.5 * this->iconSize;
    this->isExternalDragHovering = false;
    this->clipboardCut = false;
    this->isRenaming = false;
    this->isCreatingNewDirectory = false;
    memset(this->nameBuffer, 0, sizeof(this->nameBuffer));
}

void Editor::ResourcesWindow::handleExternalDragEnter() {
    isExternalDragHovering = true;
}

void Editor::ResourcesWindow::handleExternalDragLeave() {
    isExternalDragHovering = false;
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

    std::string projectPath = project->getProjectPath().string();
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

void Editor::ResourcesWindow::highlightDragAndDrop(){
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec2 maxPos = ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y);

    ImGui::GetWindowDrawList()->AddRect(
        windowPos,
        maxPos,
        ImGui::GetColorU32(ImGuiCol_DragDropTarget),
        0.0f, 0, 2.0f);
}

void Editor::ResourcesWindow::handleInternalDragAndDrop(const std::string& targetDirectory) {
    for (const auto& fileName : selectedFiles) {
        fs::path sourcePath = fs::path(currentPath) / fileName;
        fs::path destPath = fs::path(targetDirectory) / fileName;

        try {
            if (fs::exists(sourcePath)) {
                if (sourcePath.parent_path() != fs::path(targetDirectory)) {  // Prevent copying to same directory
                    if (fs::is_directory(sourcePath)) {
                        fs::copy(sourcePath, destPath, fs::copy_options::recursive);
                        fs::remove_all(sourcePath);
                    } else {
                        fs::copy(sourcePath, destPath, fs::copy_options::overwrite_existing);
                        fs::remove(sourcePath);
                    }
                }
            }
        } catch (const fs::filesystem_error& e) {
            // Handle error if needed
        }
    }

    selectedFiles.clear();
    files = scanDirectory(currentPath, (intptr_t)folderIcon.getRender()->getGLHandler(), (intptr_t)fileIcon.getRender()->getGLHandler());
}

void Editor::ResourcesWindow::handleNewDirectory(){
    // Handle new directory creation popup
    if (isCreatingNewDirectory) {
        ImGui::OpenPopup("Create New Directory");
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    }

    if (ImGui::BeginPopupModal("Create New Directory", &isCreatingNewDirectory, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter directory name:");

        // Auto-focus the input field when the popup opens
        if (ImGui::IsWindowAppearing()) {
            ImGui::SetKeyboardFocusHere();
        }

        bool enterPressed = ImGui::InputText("##newdir", nameBuffer, sizeof(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue);

        // Calculate total width of the buttons
        float buttonWidth = 120.0f;
        float buttonSpacing = ImGui::GetStyle().ItemSpacing.x;
        float totalWidth = (buttonWidth * 2) + buttonSpacing;

        // Center the buttons
        float windowWidth = ImGui::GetWindowSize().x;
        ImGui::SetCursorPosX((windowWidth - totalWidth) * 0.5f);

        bool confirmed = ImGui::Button("Create", ImVec2(buttonWidth, 0)) || enterPressed;
        if (confirmed) {
            std::string dirName = nameBuffer;
            if (!dirName.empty()) {
                try {
                    fs::path newDirPath = fs::path(currentPath) / dirName;

                    // Check if directory already exists
                    if (fs::exists(newDirPath)) {
                        ImGui::OpenPopup("Directory Already Exists");
                    } else {
                        fs::create_directory(newDirPath);
                        files = scanDirectory(currentPath, (intptr_t)folderIcon.getRender()->getGLHandler(), (intptr_t)fileIcon.getRender()->getGLHandler());
                        isCreatingNewDirectory = false;
                        ImGui::CloseCurrentPopup();
                    }
                } catch (const fs::filesystem_error& e) {
                    ImGui::OpenPopup("Creation Error");
                }
            }
            if (dirName.empty()) {
                ImGui::OpenPopup("Invalid Name");
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
            isCreatingNewDirectory = false;
            ImGui::CloseCurrentPopup();
        }

        // Error popup for existing directory
        if (ImGui::BeginPopupModal("Directory Already Exists", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("A directory with this name already exists.");
            ImGui::Separator();

            float popupWidth = ImGui::GetWindowSize().x;
            float buttonWidth = 120.0f;
            ImGui::SetCursorPosX((popupWidth - buttonWidth) * 0.5f);

            if (ImGui::Button("OK", ImVec2(buttonWidth, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Error popup for creation failure
        if (ImGui::BeginPopupModal("Creation Error", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Failed to create the directory.");
            ImGui::Separator();

            float popupWidth = ImGui::GetWindowSize().x;
            float buttonWidth = 120.0f;
            ImGui::SetCursorPosX((popupWidth - buttonWidth) * 0.5f);

            if (ImGui::Button("OK", ImVec2(buttonWidth, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Error popup for invalid name
        if (ImGui::BeginPopupModal("Invalid Name", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Please enter a valid directory name.");
            ImGui::Separator();

            float popupWidth = ImGui::GetWindowSize().x;
            float buttonWidth = 120.0f;
            ImGui::SetCursorPosX((popupWidth - buttonWidth) * 0.5f);

            if (ImGui::Button("OK", ImVec2(buttonWidth, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::EndPopup();
    }
}

void Editor::ResourcesWindow::handleRename(){
    // Handle rename popup
    if (isRenaming) {
        ImGui::OpenPopup("Rename File");
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    }

    if (ImGui::BeginPopupModal("Rename File", &isRenaming, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Rename '%s' to:", fileBeingRenamed.c_str());

        // Auto-focus the input field when the popup opens
        if (ImGui::IsWindowAppearing()) {
            ImGui::SetKeyboardFocusHere();
        }

        ImGui::SetNextItemWidth(-1);

        bool enterPressed = ImGui::InputText("##rename", nameBuffer, sizeof(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue);

        // Calculate total width of the buttons
        float buttonWidth = 120.0f;
        float buttonSpacing = ImGui::GetStyle().ItemSpacing.x;
        float totalWidth = (buttonWidth * 2) + buttonSpacing;

        // Center the buttons
        float windowWidth = ImGui::GetWindowSize().x;
        ImGui::SetCursorPosX((windowWidth - totalWidth) * 0.5f);

        bool confirmed = ImGui::Button("OK", ImVec2(buttonWidth, 0)) || enterPressed;
        if (confirmed) {
            std::string newName = nameBuffer;
            if (!newName.empty() && newName != fileBeingRenamed) {
                try {
                    fs::path oldPath = fs::path(currentPath) / fileBeingRenamed;
                    fs::path newPath = fs::path(currentPath) / newName;

                    // Check if the target file already exists
                    if (fs::exists(newPath)) {
                        ImGui::OpenPopup("File Already Exists");
                    } else {
                        fs::rename(oldPath, newPath);
                        files = scanDirectory(currentPath, (intptr_t)folderIcon.getRender()->getGLHandler(), (intptr_t)fileIcon.getRender()->getGLHandler());
                        isRenaming = false;
                        ImGui::CloseCurrentPopup();
                    }
                } catch (const fs::filesystem_error& e) {
                    ImGui::OpenPopup("Rename Error");
                }
            }
            if (newName.empty()) {
                ImGui::OpenPopup("Invalid Name");
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
            isRenaming = false;
            ImGui::CloseCurrentPopup();
        }

        // Error popup for existing file
        if (ImGui::BeginPopupModal("File Already Exists", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("A file with this name already exists.");
            ImGui::Separator();

            float popupWidth = ImGui::GetWindowSize().x;
            float buttonWidth = 120.0f;
            ImGui::SetCursorPosX((popupWidth - buttonWidth) * 0.5f);

            if (ImGui::Button("OK", ImVec2(buttonWidth, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Error popup for rename failure
        if (ImGui::BeginPopupModal("Rename Error", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Failed to rename the file.");
            ImGui::Separator();

            float popupWidth = ImGui::GetWindowSize().x;
            float buttonWidth = 120.0f;
            ImGui::SetCursorPosX((popupWidth - buttonWidth) * 0.5f);

            if (ImGui::Button("OK", ImVec2(buttonWidth, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Error popup for invalid name
        if (ImGui::BeginPopupModal("Invalid Name", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Please enter a valid file name.");
            ImGui::Separator();

            float popupWidth = ImGui::GetWindowSize().x;
            float buttonWidth = 120.0f;
            ImGui::SetCursorPosX((popupWidth - buttonWidth) * 0.5f);

            if (ImGui::Button("OK", ImVec2(buttonWidth, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::EndPopup();
    }
}

void Editor::ResourcesWindow::openFileDialog(){
    const nfdpathset_t* pathSet;
    nfdopendialogu8args_t args = {0};

    args.parentWindow = *static_cast<nfdwindowhandle_t*>(Backend::getNFDWindowHandle());

    const nfdresult_t res = NFD_OpenDialogMultipleU8_With(&pathSet, &args);
    switch (res) {
        case NFD_OKAY:
            nfdpathsetsize_t num_paths;
            if (NFD_PathSet_GetCount(pathSet, &num_paths) != NFD_OKAY) {
                printf("Error: NFD_PathSet_GetCount failed: %s\n", NFD_GetError());
                break;
            }
            nfdpathsetsize_t i;
            for (i = 0; i != num_paths; ++i) {
                char* path;
                if (NFD_PathSet_GetPathU8(pathSet, i, &path) != NFD_OKAY) {
                    printf("Error: NFD_PathSet_GetPathU8 failed: %s\n", NFD_GetError());
                    break;
                }

                std::filesystem::path sourcePath = path;
                std::filesystem::path destPath = std::filesystem::path(currentPath) / sourcePath.filename();

                try {
                    if (std::filesystem::exists(sourcePath)) {
                        if (std::filesystem::is_directory(sourcePath)) {
                            std::filesystem::copy(sourcePath, destPath, 
                                std::filesystem::copy_options::recursive);
                        } else {
                            std::filesystem::copy(sourcePath, destPath, 
                                std::filesystem::copy_options::overwrite_existing);
                        }
                    }
                } catch (const std::filesystem::filesystem_error& e) {
                    // Handle error if needed
                }

                NFD_PathSet_FreePathU8(path);
            }

            NFD_PathSet_Free(pathSet);

            // Refresh directory after importing files
            files = scanDirectory(currentPath, (intptr_t)folderIcon.getRender()->getGLHandler(), 
                (intptr_t)fileIcon.getRender()->getGLHandler());

            break;
        case NFD_ERROR:
            printf("Error: %s", NFD_GetError());
            break;
        default:
            break;
    }
}

void Editor::ResourcesWindow::copySelectedFiles(bool cut) {
    clipboardFiles.clear();
    clipboardCut = cut;

    for (const auto& fileName : selectedFiles) {
        clipboardFiles.push_back(currentPath + "/" + fileName);
    }
}

void Editor::ResourcesWindow::pasteFiles(const std::string& targetDirectory) {
    for (const auto& sourcePath : clipboardFiles) {
        fs::path sourceFs = fs::path(sourcePath);
        fs::path destFs = fs::path(targetDirectory) / sourceFs.filename();

        try {
            if (fs::exists(sourceFs)) {
                if (fs::is_directory(sourceFs)) {
                    fs::copy(sourceFs, destFs, fs::copy_options::recursive);
                    if (clipboardCut) {
                        fs::remove_all(sourceFs);
                    }
                } else {
                    fs::copy(sourceFs, destFs, fs::copy_options::overwrite_existing);
                    if (clipboardCut) {
                        fs::remove(sourceFs);
                    }
                }
            }
        } catch (const fs::filesystem_error& e) {
            // Handle error if needed
        }
    }

    // Clear clipboard if it was a cut operation
    if (clipboardCut) {
        clipboardFiles.clear();
    }

    files = scanDirectory(currentPath, (intptr_t)folderIcon.getRender()->getGLHandler(), (intptr_t)fileIcon.getRender()->getGLHandler());
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

        files = scanDirectory(project->getProjectPath().string(), (intptr_t)folderIcon.getRender()->getGLHandler(), (intptr_t)fileIcon.getRender()->getGLHandler());

        firstOpen = false;
    }

    ctrlPressed = ImGui::GetIO().KeyCtrl;
    shiftPressed = ImGui::GetIO().KeyShift;

    windowPos = ImGui::GetWindowPos();
    scrollOffset = ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());

    ImGui::Begin("Resources");

    bool clickedInFile = false;

    float windowWidth = ImGui::GetContentRegionAvail().x;
    float columnWidth = iconSize + iconPadding;

    int columns = static_cast<int>(windowWidth / columnWidth);
    if (columns < 1) columns = 1;

    ImGui::BeginDisabled(currentPath == project->getProjectPath());

    if (ImGui::Button(ICON_FA_HOUSE)) {
        files = scanDirectory(project->getProjectPath().string(), (intptr_t)folderIcon.getRender()->getGLHandler(), (intptr_t)fileIcon.getRender()->getGLHandler());
        selectedFiles.clear();
    }
    if (currentPath != project->getProjectPath() && ImGui::BeginDragDropTarget()){
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource_files")) {
            std::string targetDirectory = project->getProjectPath().string();
            handleInternalDragAndDrop(targetDirectory);
        }
        ImGui::EndDragDropTarget();
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
    if (currentPath != project->getProjectPath() && ImGui::BeginDragDropTarget()){
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource_files")) {
            std::string targetDirectory = fs::path(currentPath).parent_path().string();
            handleInternalDragAndDrop(targetDirectory);
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 50, 50, 255));
    ImGui::BeginChild("PathFrame", ImVec2(-ImGui::CalcTextSize(ICON_FA_COPY).x - ImGui::GetStyle().ItemSpacing.x - ImGui::GetStyle().FramePadding.x * 2, ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    std::string shortenedPath = shortenPath(currentPath, ImGui::GetContentRegionAvail().x);

    ImGui::SetCursorPosY(ImGui::GetStyle().FramePadding.y);
    ImGui::Text("%s", ((shortenedPath == ".") ? "" : shortenedPath).c_str());

    ImGui::EndChild();
    ImGui::SetItemTooltip("%s", currentPath.c_str());
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

    ImVec2 scrollRegionMin = ImGui::GetWindowPos();
    ImVec2 scrollRegionMax = ImVec2(scrollRegionMin.x + ImGui::GetWindowSize().x,
                        scrollRegionMin.y + ImGui::GetWindowSize().y);

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(8.0f, 8.0f));

    // Deferred deletion
    static bool showDeleteConfirmation = false;

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) {
        isDragging = true;
        dragStart = ImGui::GetMousePos();
        // Convert to window-relative coordinates
        dragStart.x -= scrollRegionMin.x;
        dragStart.y -= scrollRegionMin.y;
        // Add current scroll offset
        dragStart.x += ImGui::GetScrollX();
        dragStart.y += ImGui::GetScrollY();
        selectedFiles.clear();
    }

    if (isDragging) {
        ImVec2 mousePos = ImGui::GetMousePos();

        // Handle auto-scrolling
        float scrollMargin = 20.0f;
        float currentScroll = ImGui::GetScrollY();

        if (mousePos.y > scrollRegionMax.y - scrollMargin) {
            float scrollDelta = (mousePos.y - (scrollRegionMax.y - scrollMargin)) * 0.5f;
            ImGui::SetScrollY(currentScroll + scrollDelta);
        }
        else if (mousePos.y < scrollRegionMin.y + scrollMargin) {
            float scrollDelta = (mousePos.y - (scrollRegionMin.y + scrollMargin)) * 0.5f;
            ImGui::SetScrollY(currentScroll + scrollDelta);
        }

        // Update drag end position
        dragEnd = mousePos;
        // Convert to window-relative coordinates
        dragEnd.x -= scrollRegionMin.x;
        dragEnd.y -= scrollRegionMin.y;
        // Add current scroll offset
        dragEnd.x += ImGui::GetScrollX();
        dragEnd.y += ImGui::GetScrollY();

        if (!ImGui::IsMouseDown(0)) {
            isDragging = false;
        }
    }

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

            if (isDragging) {
                ImVec2 itemPos = ImGui::GetCursorScreenPos();
                // Convert item position to the same coordinate space as drag coordinates
                itemPos.x -= scrollRegionMin.x;
                itemPos.y -= scrollRegionMin.y;
                itemPos.x += ImGui::GetScrollX();
                itemPos.y += ImGui::GetScrollY();

                ImVec2 itemSize = selectableSize;
                ImRect itemRect(itemPos, ImVec2(itemPos.x + itemSize.x, itemPos.y + itemSize.y));

                ImRect selectionRect(
                    ImVec2(std::min(dragStart.x, dragEnd.x), std::min(dragStart.y, dragEnd.y)),
                    ImVec2(std::max(dragStart.x, dragEnd.x), std::max(dragStart.y, dragEnd.y))
                );

                // Check if item overlaps with selection rectangle
                bool isOverlapping = itemRect.Overlaps(selectionRect);

                // If CTRL is not pressed, we need to handle unselection
                if (!ctrlPressed) {
                    if (isOverlapping) {
                        selectedFiles.insert(file.name);
                    } else {
                        selectedFiles.erase(file.name);
                    }
                } else {
                    // With CTRL pressed, we only add to selection, never remove
                    if (isOverlapping) {
                        selectedFiles.insert(file.name);
                    }
                }
            }

            bool isSelected = selectedFiles.find(file.name) != selectedFiles.end();

            // Handle left-click and right-click for selection
            if (ImGui::Selectable("", isSelected, ImGuiSelectableFlags_AllowDoubleClick, selectableSize)) {
                clickedInFile = true;

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
                clickedInFile = true;
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
                if (ImGui::MenuItem(ICON_FA_COPY"  Copy")) {
                    copySelectedFiles(false);
                }

                if (ImGui::MenuItem(ICON_FA_SCISSORS"  Cut")) {
                    copySelectedFiles(true);
                }

                if (ImGui::MenuItem(ICON_FA_PASTE"  Paste", nullptr, false, !clipboardFiles.empty())) {
                    std::string targetDirectory = currentPath + "/" + lastSelectedFile;
                    pasteFiles(targetDirectory);
                }

                ImGui::Separator();

                if (ImGui::MenuItem(ICON_FA_TRASH"  Delete")) {
                    showDeleteConfirmation = true;
                }

                if (ImGui::MenuItem(ICON_FA_I_CURSOR"  Rename")) {
                    isRenaming = true;
                    fileBeingRenamed = file.name;
                    strncpy(nameBuffer, file.name.c_str(), sizeof(nameBuffer) - 1);
                    nameBuffer[sizeof(nameBuffer) - 1] = '\0';
                    ImGui::CloseCurrentPopup();
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

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                // If the dragged file isn't already selected, clear selection and select only this file
                if (selectedFiles.find(file.name) == selectedFiles.end()) {
                    selectedFiles.clear();
                    selectedFiles.insert(file.name);
                    lastSelectedFile = file.name;
                }

                // Create a dummy payload (we just need a non-null payload)
                static int dummyPayload = 1;
                ImGui::SetDragDropPayload("resource_files", &dummyPayload, sizeof(int));

                // Preview of dragged items
                ImGui::Text("Moving %zu file(s)", selectedFiles.size());

                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginDragDropTarget()){
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource_files")) {
                    std::string targetDirectory = currentPath + "/" + file.name;
                    handleInternalDragAndDrop(targetDirectory);
                }
                ImGui::EndDragDropTarget();
            }
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleVar();

    if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered() && !clickedInFile) {
        selectedFiles.clear();
    }

    // Handle right-click on empty space
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered() && !clickedInFile) {
        ImGui::OpenPopup("ResourcesContextMenu");
    }

    if (ImGui::BeginPopup("ResourcesContextMenu")) {
        if (ImGui::MenuItem(ICON_FA_FILE_IMPORT"  Import Files...")) {
            openFileDialog();
        }

        ImGui::Separator();

        if (ImGui::MenuItem(ICON_FA_FOLDER"  New Folder")) {
            isCreatingNewDirectory = true;
            memset(nameBuffer, 0, sizeof(nameBuffer));
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem(ICON_FA_PASTE"  Paste", nullptr, false, !clipboardFiles.empty())) {
            pasteFiles(currentPath);
        }

        ImGui::EndPopup();
    }

    // drawing the selection rectangle
    if (isDragging) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // Convert drag coordinates back to screen space for drawing
        ImVec2 rectMin(
            scrollRegionMin.x + std::min(dragStart.x, dragEnd.x) - ImGui::GetScrollX(),
            scrollRegionMin.y + std::min(dragStart.y, dragEnd.y) - ImGui::GetScrollY()
        );

        ImVec2 rectMax(
            scrollRegionMin.x + std::max(dragStart.x, dragEnd.x) - ImGui::GetScrollX(),
            scrollRegionMin.y + std::max(dragStart.y, dragEnd.y) - ImGui::GetScrollY()
        );

        // Clip rectangle to scroll region
        rectMin.x = ImClamp(rectMin.x, scrollRegionMin.x, scrollRegionMax.x);
        rectMin.y = ImClamp(rectMin.y, scrollRegionMin.y, scrollRegionMax.y);
        rectMax.x = ImClamp(rectMax.x, scrollRegionMin.x, scrollRegionMax.x);
        rectMax.y = ImClamp(rectMax.y, scrollRegionMin.y, scrollRegionMax.y);

        drawList->AddRect(rectMin, rectMax, IM_COL32(100, 150, 255, 255));
        drawList->AddRectFilled(rectMin, rectMax, IM_COL32(100, 150, 255, 50));
    }

    ImGui::EndChild();

    // Handle drag and drop from system
    if (ImGui::BeginDragDropTarget())
    {
        isDragDropTarget = true;

        // Accept dropped files from external applications
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("external_files"))
        {
            droppedFiles = *(std::vector<std::string>*)payload->Data;

            for (const auto& sourcePath : droppedFiles)
            {
                fs::path sourceFs = fs::path(sourcePath);
                fs::path destFs = fs::path(currentPath) / sourceFs.filename();

                try {
                    if (fs::exists(sourceFs)) {
                        if (fs::is_directory(sourceFs)) {
                            fs::copy(sourceFs, destFs, fs::copy_options::recursive);
                        } else {
                            fs::copy(sourceFs, destFs, fs::copy_options::overwrite_existing);
                        }
                    }
                } catch (const fs::filesystem_error& e) {
                    // Handle error if needed
                }
            }

            // Refresh the directory after files are copied
            files = scanDirectory(currentPath, (intptr_t)folderIcon.getRender()->getGLHandler(), (intptr_t)fileIcon.getRender()->getGLHandler());
        }

        ImGui::EndDragDropTarget();
    }

    // Visual feedback for drag and drop
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)){
        ImGuiDragDropFlags target_flags = 0;
        target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;

        if (const ImGuiPayload* payload = ImGui::GetDragDropPayload()){
            if (payload->IsDataType("external_files")){
                highlightDragAndDrop();
            }
        }
    }

    if (isExternalDragHovering){
        highlightDragAndDrop();
    }

    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
        if (!selectedFiles.empty()){
            if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
                showDeleteConfirmation = true;
            }
        }

        // Add keyboard shortcuts for copy/cut/paste
        if (ctrlPressed) {
            if (ImGui::IsKeyPressed(ImGuiKey_C)) {
                copySelectedFiles(false);
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_X)) {
                copySelectedFiles(true);
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_V)) {
                if (!clipboardFiles.empty()) {
                    pasteFiles(currentPath);
                }
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_A)) {
                // Select all files in the current directory
                selectedFiles.clear();
                for (const auto& file : files) {
                    selectedFiles.insert(file.name);
                }
                if (!files.empty()) {
                    lastSelectedFile = files.back().name;
                }
            }
        }
    }

    ImGui::End();

    // Show confirmation dialog
    if (showDeleteConfirmation) {
        ImGui::OpenPopup("Delete Confirmation");
    }

    if (ImGui::BeginPopupModal("Delete Confirmation", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to delete the following items?");
        ImGui::Separator();

        // Display up to 10 files selected for deletion
        int fileCount = 0;
        for (const auto& fileName : selectedFiles) {
            if (fileCount < 10) {
                ImGui::BulletText("%s", fileName.c_str());
            }
            fileCount++;
        }

        // If there are more files, show a message
        if (fileCount > 10) {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "And %d more items...", fileCount - 10);
        }

        ImGui::Separator();

        // Calculate total width of the buttons
        const float buttonWidth = 120.0f;
        const float buttonSpacing = ImGui::GetStyle().ItemSpacing.x;
        const float totalWidth = (buttonWidth * 2) + buttonSpacing;

        // Center the buttons
        float windowWidth = ImGui::GetWindowSize().x;
        ImGui::SetCursorPosX((windowWidth - totalWidth) / 2.0f);

        if (ImGui::Button("Yes", ImVec2(buttonWidth, 0))) {
            // Delete selected files
            for (const auto& fileName : selectedFiles) {
                std::filesystem::path pathToDelete = std::filesystem::path(currentPath) / fileName;
                try {
                    if (std::filesystem::is_directory(pathToDelete)) {
                        std::filesystem::remove_all(pathToDelete);  // Use remove_all for directories
                    } else {
                        std::filesystem::remove(pathToDelete);      // Use remove for files
                    }
                } catch (const std::filesystem::filesystem_error& e) {
                    // Optionally handle or log the error
                    std::cerr << "Error deleting " << fileName << ": " << e.what() << std::endl;
                }
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

        if (ImGui::Button("No", ImVec2(buttonWidth, 0))) {
            // Close the modal without deleting
            showDeleteConfirmation = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    handleNewDirectory();
    handleRename();
}