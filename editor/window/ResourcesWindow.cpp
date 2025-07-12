#include "ResourcesWindow.h"

#include "external/IconsFontAwesome6.h"
#include "resources/icons/folder-icon_png.h"
#include "resources/icons/file-icon_png.h"

#include "command/type/CopyFileCmd.h"
#include "command/type/RenameFileCmd.h"
#include "command/type/CreateDirCmd.h"
#include "command/type/DeleteFileCmd.h"

#include "window/Widgets.h"

#include "Backend.h"
#include "App.h"
#include "Stream.h"
#include "util/FileDialogs.h"
#include "util/SHA1.h"
#include "util/GraphicUtils.h"

#include "imgui_internal.h"

#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include <atomic>
#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize2.h"

using namespace Supernova;

Editor::ResourcesWindow::ResourcesWindow(Project* project, CodeEditor* codeEditor) {
    this->project = project;
    this->codeEditor = codeEditor;
    this->firstOpen = true;
    this->requestSort = true;
    this->iconSize = 32.0f;
    this->iconPadding = 1.5 * this->iconSize;
    this->isExternalDragHovering = false;
    this->clipboardCut = false;
    this->isRenaming = false;
    this->isCreatingNewDirectory = false;
    this->timeSinceLastCheck = 0.0f;
    this->windowFocused = false;
    this->showDeleteConfirmation = false;
    this->stopThumbnailThread = false;
    memset(this->nameBuffer, 0, sizeof(this->nameBuffer));

    thumbnailThread = std::thread(&ResourcesWindow::thumbnailWorker, this);
}

Editor::ResourcesWindow::~ResourcesWindow() {
    stopThumbnailThread = true;
    thumbnailCondition.notify_one(); // Wake the worker thread to check stop condition
    if (thumbnailThread.joinable()) {
        thumbnailThread.join();
    }
}

bool Editor::ResourcesWindow::isFocused() const{
    return windowFocused;
}

void Editor::ResourcesWindow::notifyProjectPathChange(){
    // Clear thumbnail textures when changing projects
    thumbnailTextures.clear();

    // Clear the thumbnail queue
    {
        std::lock_guard<std::mutex> lock(thumbnailMutex);
        std::queue<ThumbnailRequest> empty;
        std::swap(thumbnailQueue, empty);
    }
    scanDirectory(project->getProjectPath());
}

void Editor::ResourcesWindow::handleExternalDragEnter() {
    isExternalDragHovering = true;
}

void Editor::ResourcesWindow::handleExternalDragLeave() {
    isExternalDragHovering = false;
}

void Editor::ResourcesWindow::processMaterialThumbnails() {
    // Check if we have a pending material render that needs post-processing
    if (hasPendingMaterialRender && !Engine::isSceneRunning(materialRender.getScene())) {
        std::lock_guard<std::mutex> lock(materialRenderMutex);

        fs::path thumbnailPath = project->getThumbnailPath(pendingMaterialPath);
        fs::create_directories(thumbnailPath.parent_path());

        // Store captured variables for the callback
        fs::path capturedPath = pendingMaterialPath;
        FileType capturedType = FileType::MATERIAL;

        // Use the callback to notify when the image save is complete
        GraphicUtils::saveFramebufferImage(materialRender.getFramebuffer(), thumbnailPath, true, 
            [this, capturedPath, capturedType]() {
                // This code runs after the image has been saved
                std::lock_guard<std::mutex> completedLock(completedThumbnailMutex);
                completedThumbnailQueue.push({capturedPath, capturedType});
            });

        // Mark as processed
        hasPendingMaterialRender = false;

        // Notify the thumbnail thread that we've processed the material
        thumbnailCondition.notify_one();
    }
}

void Editor::ResourcesWindow::renderHeader() {
    ImGui::BeginDisabled(currentPath == project->getProjectPath());
    if (ImGui::Button(ICON_FA_HOUSE)) {
        scanDirectory(project->getProjectPath().string());
        selectedFiles.clear();
    }
    if (currentPath != project->getProjectPath() && ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource_files")) {
            handleInternalDragAndDrop(project->getProjectPath());
        }
        ImGui::EndDragDropTarget();
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ANGLE_LEFT)) {
        if (!currentPath.empty() && currentPath != project->getProjectPath()) {
            fs::path parentPath = currentPath.parent_path();
            currentPath = parentPath;
            scanDirectory(currentPath);
            selectedFiles.clear();
        }
    }
    if (currentPath != project->getProjectPath() && ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource_files")) {
            handleInternalDragAndDrop(currentPath.parent_path());
        }
        ImGui::EndDragDropTarget();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    Vector2 pathDisplaySize = Vector2(-ImGui::CalcTextSize(ICON_FA_GEAR).x - ImGui::GetStyle().ItemSpacing.x - ImGui::GetStyle().FramePadding.x * 2, ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2);
    Widgets::pathDisplay(currentPath, pathDisplaySize, project->getProjectPath());
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_GEAR)) {
        ImGui::OpenPopup("SettingsPopup");
    }

    if (ImGui::BeginPopup("SettingsPopup")) {
        ImGui::Text("Settings");
        ImGui::Separator();
        if (ImGui::SliderInt("Icon Size", &iconSize, 16.0f, THUMBNAIL_SIZE)) {
            iconPadding = 1.5 * iconSize;
        }
        ImGui::Separator();
        ImGui::Text("Layout");
        if (ImGui::RadioButton("Auto", currentLayout == LayoutType::AUTO)) {
            currentLayout = LayoutType::AUTO;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Grid", currentLayout == LayoutType::GRID)) {
            currentLayout = LayoutType::GRID;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Split view", currentLayout == LayoutType::SPLIT)) {
            currentLayout = LayoutType::SPLIT;
        }
        ImGui::EndPopup();
    }
}

void Editor::ResourcesWindow::renderFileListing(bool showDirectories) {
    float columnWidth = iconSize + iconPadding;
    float availableWidth = ImGui::GetContentRegionAvail().x;

    int columns = static_cast<int>(availableWidth / columnWidth);
    if (columns < 1) columns = 1;  // Ensure at least one column
    if (files.size() > 0 && files.size() < columns) columns = files.size();  // Limit columns to number of files if fewer

    ImVec2 cellPadding = ImVec2(8.0f, 8.0f);  // Example padding, adjust as needed
    float totalTableWidth = std::min(availableWidth, 
        (columnWidth * files.size()) + 
        (columnWidth * files.size() / 2.0f) + 
        (cellPadding.x * 2 * (files.size() - 1))
    );

    ImVec2 scrollRegionMin = ImGui::GetWindowPos();
    ImVec2 scrollRegionMax = ImVec2(scrollRegionMin.x + ImGui::GetWindowSize().x,
                                   scrollRegionMin.y + ImGui::GetWindowSize().y);
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cellPadding);

    bool clickedInFile = false;  // Declare this before the table for scope

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) {
        isDragging = true;
        dragStart = ImGui::GetMousePos();
        dragStart.x -= scrollRegionMin.x;
        dragStart.y -= scrollRegionMin.y;
        dragStart.x += ImGui::GetScrollX();
        dragStart.y += ImGui::GetScrollY();
        selectedFiles.clear();
    }

    if (isDragging) {
        ImVec2 mousePos = ImGui::GetMousePos();
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
        dragEnd = mousePos;
        dragEnd.x -= scrollRegionMin.x;
        dragEnd.y -= scrollRegionMin.y;
        dragEnd.x += ImGui::GetScrollX();
        dragEnd.y += ImGui::GetScrollY();
        if (!ImGui::IsMouseDown(0)) {
            isDragging = false;
        }
    }

    if (ImGui::BeginTable("FileTable", columns, ImGuiTableFlags_SizingStretchSame, ImVec2(totalTableWidth, 0))) {
        for (auto& file : files) {
            if (!showDirectories && file.isDirectory) continue;  // Skip directories if not in list mode
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
                itemPos.x -= scrollRegionMin.x;
                itemPos.y -= scrollRegionMin.y;
                itemPos.x += ImGui::GetScrollX();
                itemPos.y += ImGui::GetScrollY();
                ImRect itemRect(itemPos, ImVec2(itemPos.x + selectableSize.x, itemPos.y + selectableSize.y));
                ImRect selectionRect(
                    ImVec2(std::min(dragStart.x, dragEnd.x), std::min(dragStart.y, dragEnd.y)),
                    ImVec2(std::max(dragStart.x, dragEnd.x), std::max(dragStart.y, dragEnd.y))
                );
                bool isOverlapping = itemRect.Overlaps(selectionRect);
                if (!ctrlPressed) {
                    if (isOverlapping) selectedFiles.insert(file.name);
                    else selectedFiles.erase(file.name);
                } else {
                    if (isOverlapping) selectedFiles.insert(file.name);
                }
            }

            bool isSelected = selectedFiles.find(file.name) != selectedFiles.end();
            if (ImGui::Selectable("", isSelected, ImGuiSelectableFlags_AllowDoubleClick, selectableSize)) {
                clickedInFile = true;
                if (ctrlPressed) {
                    if (isSelected) selectedFiles.erase(file.name);
                    else selectedFiles.insert(file.name);
                    lastSelectedFile = file.name;
                } else if (shiftPressed) {
                    if (!lastSelectedFile.empty()) {
                        auto itStart = std::find_if(files.begin(), files.end(), [&](const FileEntry& entry) { return entry.name == lastSelectedFile; });
                        auto itEnd = std::find_if(files.begin(), files.end(), [&](const FileEntry& entry) { return entry.name == file.name; });
                        if (itStart != files.end() && itEnd != files.end()) {
                            if (itStart > itEnd) std::swap(itStart, itEnd);
                            for (auto it = itStart; it <= itEnd; ++it) {
                                if (showDirectories || !it->isDirectory) selectedFiles.insert(it->name);
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
                if (ImGui::IsMouseDoubleClicked(0)) {
                    if (file.isDirectory) {
                        scanDirectory(currentPath / file.name);
                        selectedFiles.clear();
                    } else {
                        // Existing file handling
                        std::string extension = file.extension;
                        if (extension == ".scene") {
                            project->openScene(currentPath / file.name);
                        } else if (extension == ".c" || extension == ".cpp" || extension == ".h" || extension == ".hpp") {
                            codeEditor->openFile((currentPath / file.name).string());
                        }
                    }
                }
            }

            if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                clickedInFile = true;
                if (!ctrlPressed && !isSelected) selectedFiles.clear();
                selectedFiles.insert(file.name);
                lastSelectedFile = file.name;
                ImGui::OpenPopup("FileContextMenu");
            }

            if (ImGui::BeginPopup("FileContextMenu")) {
                if (ImGui::MenuItem(ICON_FA_COPY"  Copy")) copySelectedFiles(false);
                if (ImGui::MenuItem(ICON_FA_SCISSORS"  Cut")) copySelectedFiles(true);
                if (ImGui::MenuItem(ICON_FA_PASTE"  Paste", nullptr, false, !clipboardFiles.empty())) pasteFiles(currentPath / lastSelectedFile);
                ImGui::Separator();
                if (ImGui::MenuItem(ICON_FA_TRASH"  Delete")) showDeleteConfirmation = true;
                if (ImGui::MenuItem(ICON_FA_I_CURSOR"  Rename")) {
                    isRenaming = true;
                    fileBeingRenamed = file.name;
                    strncpy(nameBuffer, file.name.c_str(), sizeof(nameBuffer) - 1);
                    nameBuffer[sizeof(nameBuffer) - 1] = '\0';
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            ImTextureID fileIconImage = (ImTextureID)file.icon;
            ImVec2 fileIconSize = ImVec2(iconSize, iconSize);

            float iconOffsetX = (cellWidth - iconSize) / 2;
            float iconOffsetY = selectableSize.y + itemSpacingY;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + iconOffsetX);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - iconOffsetY);
            if (file.hasThumbnail && thumbnailTextures.find(file.thumbnailPath) != thumbnailTextures.end()) {
                Texture& thumbTexture = thumbnailTextures[file.thumbnailPath];
                if (!thumbTexture.empty()) {
                    // Get actual thumbnail dimensions
                    int thumbWidth = thumbTexture.getWidth();
                    int thumbHeight = thumbTexture.getHeight();

                    // Calculate scaling to fit within iconSize while preserving aspect ratio
                    float scaleX = (float)iconSize / thumbWidth;
                    float scaleY = (float)iconSize / thumbHeight;
                    float scale = std::min(scaleX, scaleY);

                    // Calculate display dimensions
                    float displayWidth = thumbWidth * scale;
                    float displayHeight = thumbHeight * scale;

                    float offsetX = (iconSize - displayWidth) / 2;
                    float offsetY = (iconSize - displayHeight) / 2;

                    // Adjust cursor position for centering
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);

                    fileIconImage = (ImTextureID)(intptr_t)thumbTexture.getRender()->getGLHandler();
                    fileIconSize = ImVec2(displayWidth, displayHeight);
                }
            }

            ImGui::Image(fileIconImage, fileIconSize);

            float textOffsetX = (cellWidth / 2) - (textSize.x / 2);
            if (textOffsetX < 0) textOffsetX = 0;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffsetX);
            ImGui::TextWrapped("%s", file.name.c_str());

            ImGui::EndGroup();
            ImGui::PopID();

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                if (selectedFiles.find(file.name) == selectedFiles.end()) {
                    selectedFiles.clear();
                    selectedFiles.insert(file.name);
                    lastSelectedFile = file.name;
                }
                std::vector<char> buffer;
                for (const auto& selectedFile : selectedFiles) {
                    std::string fullPath = (currentPath / selectedFile).string();
                    buffer.insert(buffer.end(), fullPath.begin(), fullPath.end());
                    buffer.push_back('\0');
                }
                ImGui::SetDragDropPayload("resource_files", buffer.data(), buffer.size());
                ImGui::Text("Moving %zu file(s)", selectedFiles.size());
                if (selectedFiles.size() == 1){
                    float imageDragSize = 32;

                    // Calculate scaling to fit within iconSize while preserving aspect ratio
                    float scaleX = imageDragSize / fileIconSize.x;
                    float scaleY = imageDragSize / fileIconSize.y;
                    float scale = std::min(scaleX, scaleY);

                    // Calculate display dimensions
                    float displayWidth = fileIconSize.x * scale;
                    float displayHeight = fileIconSize.y * scale;

                    float availWidth = ImGui::GetCurrentWindow()->Size.x;
                    float xPos = (availWidth - displayWidth) * 0.5f;
                    ImGui::SetCursorPosX(xPos);

                    ImGui::Image(fileIconImage, ImVec2(displayWidth, displayHeight));
                }
                ImGui::EndDragDropSource();
            }

            if (file.isDirectory && ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource_files")) {
                    handleInternalDragAndDrop(currentPath / file.name);
                }
                ImGui::EndDragDropTarget();
            }
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleVar();

    // Handle right-click on empty space
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered() && !clickedInFile) {
        ImGui::OpenPopup("ResourcesContextMenu");
    }

    if (ImGui::BeginPopup("ResourcesContextMenu")) {
        if (ImGui::MenuItem(ICON_FA_FILE_IMPORT"  Import Files")) {
            std::vector<std::string> filePaths = Editor::FileDialogs::openFileDialogMultiple();
            if (!filePaths.empty()) {
                cmdHistory.addCommand(new CopyFileCmd(filePaths, currentPath.string(), true));
                scanDirectory(currentPath);
            }
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

    if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered() && !clickedInFile) {
        selectedFiles.clear();
    }

    if (isDragging) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 rectMin(
            scrollRegionMin.x + std::min(dragStart.x, dragEnd.x) - ImGui::GetScrollX(),
            scrollRegionMin.y + std::min(dragStart.y, dragEnd.y) - ImGui::GetScrollY()
        );
        ImVec2 rectMax(
            scrollRegionMin.x + std::max(dragStart.x, dragEnd.x) - ImGui::GetScrollX(),
            scrollRegionMin.y + std::max(dragStart.y, dragEnd.y) - ImGui::GetScrollY()
        );
        rectMin.x = ImClamp(rectMin.x, scrollRegionMin.x, scrollRegionMax.x);
        rectMin.y = ImClamp(rectMin.y, scrollRegionMin.y, scrollRegionMax.y);
        rectMax.x = ImClamp(rectMax.x, scrollRegionMin.x, scrollRegionMax.x);
        rectMax.y = ImClamp(rectMax.y, scrollRegionMin.y, scrollRegionMax.y);
        drawList->AddRect(rectMin, rectMax, IM_COL32(100, 150, 255, 255));
        drawList->AddRectFilled(rectMin, rectMax, IM_COL32(100, 150, 255, 50));
    }

    // Delete confirmation popup
    if (showDeleteConfirmation) {
        ImGui::OpenPopup("Delete Confirmation");
    }
    if (ImGui::BeginPopupModal("Delete Confirmation", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to delete the following items?");
        ImGui::Separator();
        int fileCount = 0;
        for (const auto& fileName : selectedFiles) {
            if (fileCount < 10) ImGui::BulletText("%s", fileName.c_str());
            fileCount++;
        }
        if (fileCount > 10) ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "And %d more items...", fileCount - 10);
        ImGui::Separator();
        const float buttonWidth = 120.0f;
        const float buttonSpacing = ImGui::GetStyle().ItemSpacing.x;
        const float totalWidth = (buttonWidth * 2) + buttonSpacing;
        float windowWidth = ImGui::GetWindowSize().x;
        ImGui::SetCursorPosX((windowWidth - totalWidth) / 2.0f);
        if (ImGui::Button("Yes", ImVec2(buttonWidth, 0))) {
            std::vector<fs::path> pathsToDelete;
            for (const auto& fileName : selectedFiles) {
                fs::path filePath = currentPath / fileName;
                pathsToDelete.push_back(filePath);
                codeEditor->closeFile(filePath.string());
            }
            cmdHistory.addCommand(new DeleteFileCmd(pathsToDelete, project->getProjectPath()));
            selectedFiles.clear();
            scanDirectory(currentPath);
            showDeleteConfirmation = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("No", ImVec2(buttonWidth, 0))) {
            showDeleteConfirmation = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void Editor::ResourcesWindow::renderDirectoryTree(const fs::path& rootPath) {
    std::string rootFullPath = rootPath.string();
    std::string rootDisplayName = rootPath.filename().string();
    if (rootDisplayName.empty()) rootDisplayName = rootPath.string(); // Handle root paths like C:/ or /

    bool isRootSelected = (currentPath == rootPath);
    ImGuiTreeNodeFlags rootFlags = ImGuiTreeNodeFlags_OpenOnArrow;
    if (isRootSelected) rootFlags |= ImGuiTreeNodeFlags_Selected;

    // Check if this directory is in the path to current directory
    bool isInCurrentPath = false;
    if (currentPath.string().find(rootPath.string()) == 0 && rootPath != currentPath) {
        // This directory is a parent of the current directory
        isInCurrentPath = true;
    }

    // For root directory of the project or directories in path to current directory, expand initially
    if (rootPath == project->getProjectPath() || isInCurrentPath) {
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    }

    // Auto-expand the parent of the current directory
    if (rootPath == currentPath.parent_path()) {
        ImGui::SetNextItemOpen(true, ImGuiCond_Always);
    }

    // Use unique ID to prevent ImGui tree node confusion
    ImGui::PushID(rootFullPath.c_str());

    // Get the unique ID for this tree node
    ImGuiID node_id = ImGui::GetID("##node");

    // Query the current open state from ImGui's state storage
    bool is_open = ImGui::GetStateStorage()->GetInt(node_id, 0) != 0;

    // Select the appropriate icon: open folder if node is open OR selected, closed folder otherwise
    const char* icon = (is_open || isRootSelected) ? ICON_FA_FOLDER_OPEN : ICON_FA_FOLDER;

    // Check if there are subdirectories to determine if the node is expandable
    bool hasSubdirectories = false;
    for (const auto& entry : fs::directory_iterator(rootPath)) {
        if (entry.is_directory()) {
            hasSubdirectories = true;
            break;
        }
    }

    // If no subdirectories, mark as leaf node (no arrow)
    if (!hasSubdirectories) {
        rootFlags |= ImGuiTreeNodeFlags_Leaf;
    }

    // Render the current directory node with the selected icon
    bool nodeOpen = ImGui::TreeNodeEx("##node", rootFlags, "%s %s", icon, rootDisplayName.c_str());

    if (ImGui::IsItemClicked()) {
        currentPath = rootPath;
        scanDirectory(currentPath);
    }

    // Handle drag and drop target
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource_files")) {
            handleInternalDragAndDrop(rootPath);
        }
        ImGui::EndDragDropTarget();
    }

    if (nodeOpen) {
        // Render subdirectories
        for (const auto& entry : fs::directory_iterator(rootPath)) {
            if (entry.is_directory()) {
                // Skip hidden directories (starting with ".")
                std::string fileName = entry.path().filename().string();
                if (fileName[0] == '.') continue;

                // Recursively render the subdirectory
                renderDirectoryTree(entry.path());
            }
        }
        ImGui::TreePop();
    }

    ImGui::PopID();
}

void Editor::ResourcesWindow::scanDirectory(const fs::path& path) {
    currentPath = path;

    if (!std::filesystem::is_directory(path)) {
        currentPath = project->getProjectPath();
    }

    requestSort = true;

    // Update last write time
    lastWriteTime = fs::last_write_time(currentPath);

    intptr_t folderIconH = (intptr_t)folderIcon.getRender()->getGLHandler();
    intptr_t fileIconH = (intptr_t)fileIcon.getRender()->getGLHandler();

    files.clear();

    for (const auto& entry : fs::directory_iterator(path)) {
        // Skip hidden files and directories (starting with '.')
        if (entry.path().filename().string()[0] == '.') {
            continue;
        }

        // Skip project.yaml file
        if (entry.path().filename() == "project.yaml") {
            continue;
        }

        FileEntry fileEntry;
        fileEntry.name = entry.path().filename().string();
        fileEntry.isDirectory = entry.is_directory();
        fileEntry.icon = entry.is_directory() ? folderIconH : fileIconH;
        fileEntry.hasThumbnail = false;

        if (!fileEntry.isDirectory) {
            fileEntry.extension = entry.path().extension().string();
            if (isImageFile(fileEntry.extension)){
                fileEntry.type = FileType::IMAGE;
            }else if (isSceneFile(fileEntry.extension)){
                fileEntry.type = FileType::SCENE;
            }else if (isMaterialFile(fileEntry.extension)){
                fileEntry.type = FileType::MATERIAL;
            }else if (isEntityFile(fileEntry.extension)){
                fileEntry.type = FileType::ENTITY;
            }

            if (fileEntry.type == FileType::IMAGE || fileEntry.type == FileType::MATERIAL) {
                queueThumbnailGeneration(entry.path(), fileEntry.type);
            }
        } else {
            fileEntry.extension = "";
        }

        files.push_back(fileEntry);
    }
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

                case 1: // Column 1: "Extension"
                    if (a.extension != b.extension) {
                        return ascending ? (a.extension < b.extension) : (a.extension > b.extension);
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

void Editor::ResourcesWindow::handleInternalDragAndDrop(const fs::path& targetDirectory) {
    std::vector<std::string> filesVector(selectedFiles.begin(), selectedFiles.end());
    cmdHistory.addCommand(new CopyFileCmd(filesVector, currentPath.string(), targetDirectory.string(), false));

    selectedFiles.clear();
    scanDirectory(currentPath);
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

        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
            isCreatingNewDirectory = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        bool confirmed = ImGui::Button("Create", ImVec2(buttonWidth, 0)) || enterPressed;
        if (confirmed) {
            std::string dirName = nameBuffer;
            if (!dirName.empty()) {
                try {
                    fs::path newDirPath = currentPath / dirName;

                    // Check if directory already exists
                    if (fs::exists(newDirPath)) {
                        ImGui::OpenPopup("Directory Already Exists");
                    } else {
                        cmdHistory.addCommand(new CreateDirCmd(dirName, currentPath.string()));
                        scanDirectory(currentPath);
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

        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
            isRenaming = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        bool confirmed = ImGui::Button("OK", ImVec2(buttonWidth, 0)) || enterPressed;
        if (confirmed) {
            std::string newName = nameBuffer;
            if (!newName.empty() && newName != fileBeingRenamed) {
                try {
                    fs::path oldPath = currentPath / fileBeingRenamed;
                    fs::path newPath = currentPath / newName;

                    // Check if the target file already exists
                    if (fs::exists(newPath)) {
                        ImGui::OpenPopup("File Already Exists");
                    } else {
                        cmdHistory.addCommand(new RenameFileCmd(fileBeingRenamed, newName, currentPath.string()));
                        codeEditor->handleFileRename(oldPath, newPath);
                        scanDirectory(currentPath);
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

void Editor::ResourcesWindow::copySelectedFiles(bool cut) {
    clipboardFiles.clear();
    clipboardCut = cut;

    for (const auto& fileName : selectedFiles) {
        clipboardFiles.push_back((currentPath / fileName).string());
    }
}

void Editor::ResourcesWindow::pasteFiles(const fs::path& targetDirectory) {
    cmdHistory.addCommand(new CopyFileCmd(clipboardFiles, targetDirectory.string(), !clipboardCut));

    // Clear clipboard if it was a cut operation
    if (clipboardCut) {
        clipboardFiles.clear();
    }

    scanDirectory(currentPath);
}

bool Editor::ResourcesWindow::isImageFile(const std::string& extension) const {
    static const std::unordered_set<std::string> imageExtensions = {
        ".png", ".jpg", ".jpeg", ".bmp", ".tga", ".gif", ".hdr", ".psd", ".pic", ".pnm"
    };

    std::string lowerExt = extension;
    std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);
    return imageExtensions.find(lowerExt) != imageExtensions.end();
}

bool Editor::ResourcesWindow::isSceneFile(const std::string& extension) const{
    static const std::unordered_set<std::string> imageExtensions = {
        ".scene"
    };

    std::string lowerExt = extension;
    std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);
    return imageExtensions.find(lowerExt) != imageExtensions.end();
}

bool Editor::ResourcesWindow::isMaterialFile(const std::string& extension) const{
    static const std::unordered_set<std::string> imageExtensions = {
        ".material"
    };

    std::string lowerExt = extension;
    std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);
    return imageExtensions.find(lowerExt) != imageExtensions.end();
}

bool Editor::ResourcesWindow::isEntityFile(const std::string& extension) const{
    static const std::unordered_set<std::string> entityExtensions = {
        ".entity"
    };

    std::string lowerExt = extension;
    std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);
    return entityExtensions.find(lowerExt) != entityExtensions.end();
}

void Editor::ResourcesWindow::queueThumbnailGeneration(const fs::path& filePath, FileType type) {
    fs::path thumbnailPath = project->getThumbnailPath(filePath);

    ThumbnailRequest thumbFile = {filePath, type};
    if (fs::exists(thumbnailPath)) {
        auto imageTime = fs::last_write_time(filePath);
        auto thumbTime = fs::last_write_time(thumbnailPath);
        if (thumbTime >= imageTime) {
            // Thumbnail is up-to-date, queue it for loading
            std::lock_guard<std::mutex> lock(completedThumbnailMutex);
            completedThumbnailQueue.push(thumbFile);
            return;
        }
    }
    {
        std::lock_guard<std::mutex> lock(thumbnailMutex);
        thumbnailQueue.push(thumbFile);
    }
    thumbnailCondition.notify_one();
}

void Editor::ResourcesWindow::thumbnailWorker() {
    while (!stopThumbnailThread) {
        ThumbnailRequest thumbFile;
        {
            std::unique_lock<std::mutex> lock(thumbnailMutex);
            // Wait until there is work or the thread is stopped
            thumbnailCondition.wait(lock, [this]() {
                return (!thumbnailQueue.empty() && !hasPendingMaterialRender) || stopThumbnailThread;
            });

            // If we're stopping, exit
            if (stopThumbnailThread) {
                return;
            }

            // Get the next file path
            thumbFile = thumbnailQueue.front();
            thumbnailQueue.pop();
        }

        // Generate thumbnail
        if (thumbFile.type == FileType::IMAGE) {
            int width, height, channels;
            unsigned char* data = stbi_load(thumbFile.path.string().c_str(), &width, &height, &channels, 4);
            if (data) {
                // Define maximum thumbnail dimension
                int maxThumbSize = THUMBNAIL_SIZE;

                // Calculate scaling factor to fit within maxThumbSize while preserving aspect ratio
                float scale = (width > height) ? 
                    (float)maxThumbSize / width : 
                    (float)maxThumbSize / height;

                // Calculate the new dimensions
                int newWidth = static_cast<int>(width * scale);
                int newHeight = static_cast<int>(height * scale);

                // Ensure dimensions are at least 1 pixel
                newWidth = std::max(1, newWidth);
                newHeight = std::max(1, newHeight);

                // Create a buffer for the resized image
                unsigned char* thumbData = new unsigned char[newWidth * newHeight * 4];

                // Resize the image while preserving aspect ratio
                stbir_resize_uint8_linear(
                    data,                   // input
                    width,                  // input width
                    height,                 // input height
                    0,                      // input stride in bytes (0 = width * channels)
                    thumbData,              // output
                    newWidth,               // output width
                    newHeight,              // output height
                    0,                      // output stride in bytes (0 = width * channels)
                    STBIR_RGBA              // number of channels (RGBA = 4)
                );

                // Save the thumbnail
                fs::path thumbnailPath = project->getThumbnailPath(thumbFile.path);
                fs::create_directories(thumbnailPath.parent_path());
                stbi_write_png(thumbnailPath.string().c_str(), newWidth, newHeight, 4, thumbData, newWidth * 4);

                // Clean up
                delete[] thumbData;
                stbi_image_free(data);

                // Notify main thread that thumbnail is ready
                {
                    std::lock_guard<std::mutex> lock(completedThumbnailMutex);
                    completedThumbnailQueue.push(thumbFile);
                }
            }
        } else if (thumbFile.type == FileType::MATERIAL) {
            try {
                if (YAML::Node materialNode = YAML::LoadFile(thumbFile.path.string())) {
                    Material material = Stream::decodeMaterial(materialNode);
                    materialRender.applyMaterial(material);

                    Engine::startAsyncThread();
                    Engine::executeSceneOnce(materialRender.getScene());
                    Engine::endAsyncThread();

                    // Set the pending flag before executing the scene
                    {
                        std::lock_guard<std::mutex> lock(materialRenderMutex);
                        pendingMaterialPath = thumbFile.path;
                        hasPendingMaterialRender = true;
                    }

                    // The processMaterialThumbnails method will handle the rest
                    // and set hasPendingMaterialRender to false when done
                }
            } catch (const std::exception& e) {
                // Log the error and continue with other thumbnails
                std::cerr << "Error generating thumbnail for material: " << thumbFile.path.string() << " - " << e.what() << std::endl;

                // Make sure we reset the pending flag if there was an error
                std::lock_guard<std::mutex> lock(materialRenderMutex);
                hasPendingMaterialRender = false;
            }

        }
    }
}

// Load a thumbnail texture for a file entry
bool Editor::ResourcesWindow::loadThumbnail(FileEntry& entry) {
    if (entry.hasThumbnail) {
        return true;
    }

    fs::path filePath = currentPath / entry.name;
    fs::path thumbnailPath = project->getThumbnailPath(filePath);

    if (fs::exists(thumbnailPath)) {
        // Check if we already have this thumbnail loaded
        if (thumbnailTextures.find(thumbnailPath.string()) == thumbnailTextures.end()) {
            // Sync load the thumbnail texture
            Texture thumbTexture(thumbnailPath.string(), TextureData(thumbnailPath.string().c_str()));
            if (thumbTexture.load()){
                thumbnailTextures[thumbnailPath.string()] = thumbTexture;
            }else{
                // only false if there is file but texture is not loaded
                return false;
            }
        }

        entry.hasThumbnail = true;
        entry.thumbnailPath = thumbnailPath.string();
    }

    return true;
}

void Editor::ResourcesWindow::saveMaterialFile(const fs::path& directory, const char* materialContent, size_t contentLen) {
    std::string baseName = "Material";
    std::string fileName = baseName + ".material";
    fs::path targetFile = directory / fileName;
    int counter = 1;
    while (fs::exists(targetFile)) {
        fileName = baseName + "_" + std::to_string(counter) + ".material";
        targetFile = directory / fileName;
        counter++;
    }
    std::ofstream out(targetFile, std::ios::binary);
    if (out.is_open()) {
        out.write(materialContent, contentLen);
        out.close();
        scanDirectory(currentPath);
    }
}

void Editor::ResourcesWindow::saveEntityFile(const fs::path& directory, const char* entityContent, size_t contentLen, const std::string& entityName) {
    std::string baseName = entityName.empty() ? "Entity" : entityName;
    // Replace invalid filename characters with underscores
    for (char& c : baseName) {
        if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
            c = '_';
        }
    }
    std::string fileName = baseName + ".entity";
    fs::path targetFile = directory / fileName;
    int counter = 1;
    while (fs::exists(targetFile)) {
        fileName = baseName + "_" + std::to_string(counter) + ".entity";
        targetFile = directory / fileName;
        counter++;
    }
    std::ofstream out(targetFile, std::ios::binary);
    if (out.is_open()) {
        out.write(entityContent, contentLen);
        out.close();
        scanDirectory(currentPath);
    }
}

void Editor::ResourcesWindow::cleanupThumbnails() {
    // 1. Collect all valid thumbnail paths of existing files
    std::unordered_set<std::string> validThumbPaths;

    // Recursively scan project directory for files that have thumbnails
    for (auto& p : fs::recursive_directory_iterator(project->getProjectPath())) {
        if (!fs::is_regular_file(p.status()))
            continue;

        std::string ext = p.path().extension().string();
        if (isImageFile(ext) || isMaterialFile(ext)) {
            fs::path thumbnailPath = project->getThumbnailPath(p.path());
            validThumbPaths.insert(thumbnailPath.string());
        }
    }

    // 2. Walk .supernova/thumbs/ and remove any orphaned thumbnails
    fs::path thumbsDir = project->getThumbsDir();
    if (!fs::exists(thumbsDir)) return;

    // Recursively iterate through the thumbnails directory to handle subdirectories
    for (auto& p : fs::recursive_directory_iterator(thumbsDir)) {
        if (!fs::is_regular_file(p.status())) continue;

        std::string thumbnailPath = p.path().string();
        if (validThumbPaths.find(thumbnailPath) == validThumbPaths.end()) {
            // This thumbnail file doesn't correspond to any existing image/material file
            std::error_code ec;
            fs::remove(p, ec);

            // Also remove from the loaded thumbnails cache
            thumbnailTextures.erase(thumbnailPath);
        }
    }

    // 3. Remove empty directories in the thumbnails folder
    for (auto& p : fs::recursive_directory_iterator(thumbsDir)) {
        if (fs::is_directory(p.status()) && fs::is_empty(p.path())) {
            std::error_code ec;
            fs::remove(p, ec);
        }
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

        scanDirectory(project->getProjectPath().string());

        firstOpen = false;
    }

    // Process completed thumbnails
    while (true) {
        ThumbnailRequest thumbFile;
        {
            std::lock_guard<std::mutex> lock(completedThumbnailMutex);
            if (completedThumbnailQueue.empty()) {
                break;
            }
            thumbFile = completedThumbnailQueue.front();
            completedThumbnailQueue.pop();
        }
        // Find and update the corresponding file entry
        for (auto& file : files) {
            if ((currentPath / file.name) == thumbFile.path) {
                if (!loadThumbnail(file)){
                    std::lock_guard<std::mutex> lock(completedThumbnailMutex);
                    completedThumbnailQueue.push(thumbFile);
                }
                break;
            }
        }
    }

    timeSinceLastCheck += ImGui::GetIO().DeltaTime;
    if (timeSinceLastCheck >= 1.0f) {
        try {
            auto currentWriteTime = fs::last_write_time(currentPath);
            if (currentWriteTime != lastWriteTime) {
                scanDirectory(currentPath);
            }
        } catch (const fs::filesystem_error& e) {
            // Handle potential filesystem errors silently
        }
        timeSinceLastCheck = 0.0f;
    }

    ctrlPressed = ImGui::GetIO().KeyCtrl;
    shiftPressed = ImGui::GetIO().KeyShift;

    windowPos = ImGui::GetWindowPos();
    scrollOffset = ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());

    ImGui::Begin("Resources");
    windowPos = ImGui::GetWindowPos();
    scrollOffset = ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());

    // Determine effective layout when in AUTO mode
    LayoutType effectiveLayout = currentLayout;
    if (currentLayout == LayoutType::AUTO) {
        float windowWidth = ImGui::GetWindowWidth();
        effectiveLayout = (windowWidth < layoutAutoThreshold) ? LayoutType::GRID : LayoutType::SPLIT;
    }

    ImGuiTableFlags table_flags_for_sort_specs = ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders;

    if (effectiveLayout == LayoutType::GRID) {
        renderHeader();
        ImGui::Separator();
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
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
        renderFileListing(true);
        ImGui::EndChild();
    } else if (effectiveLayout == LayoutType::SPLIT) {
        float splitterWidth = 4.0f;
        ImGui::BeginChild("LeftPanel", ImVec2(leftPanelWidth, 0), true);
        renderHeader();
        ImGui::Separator();
        renderDirectoryTree(project->getProjectPath());
        ImGui::EndChild();
        ImGui::SameLine();
        float splitterX = ImGui::GetCursorPosX();
        ImGui::InvisibleButton("splitter", ImVec2(splitterWidth, -1));

        // Handle visual appearance of the splitter
        ImVec2 splitterMin = ImGui::GetItemRectMin();
        ImVec2 splitterMax = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddRectFilled(
            splitterMin, 
            splitterMax, 
            ImGui::GetColorU32(ImGui::IsItemHovered() || ImGui::IsItemActive() 
                ? ImGuiCol_SliderGrabActive 
                : ImGuiCol_SliderGrab)
        );

        // Handle splitter interaction
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        }
        if (ImGui::IsItemActive()) {
            leftPanelWidth += ImGui::GetIO().MouseDelta.x;
            leftPanelWidth = ImClamp(leftPanelWidth, 100.0f, ImGui::GetWindowWidth() - 100.0f);
        }
        ImGui::SameLine();
        ImGui::BeginChild("RightPanel", ImVec2(0, 0), true);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
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
        renderFileListing(false);
        ImGui::EndChild();
        ImGui::EndChild();
    }

    // Remaining code (drag and drop, context menus, keyboard shortcuts, etc.) remains unchanged
    if (ImGui::BeginDragDropTarget()) {
        isDragDropTarget = true;
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("external_files")) {
            std::vector<std::string> droppedPaths = *(std::vector<std::string>*)payload->Data;
            cmdHistory.addCommand(new CopyFileCmd(droppedPaths, currentPath.string(), true));
            scanDirectory(currentPath);
        }
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("material")) {
            // Assume the payload is a null-terminated string (YAML)
            const char* materialContent = (const char*)payload->Data;
            size_t contentLen = payload->DataSize;

            saveMaterialFile(currentPath, materialContent, contentLen);
        }
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("entity")) {
            const char* entityContent = (const char*)payload->Data;
            size_t contentLen = payload->DataSize;

            // Parse the YAML to get the entity name
            std::string entityName = "Entity";
            YAML::Node entityNode = YAML::Load(std::string(entityContent, contentLen));
            if (entityNode["name"]) {
                entityName = entityNode["name"].as<std::string>();
            }

            saveEntityFile(currentPath, entityContent, contentLen, entityName);
        }
        ImGui::EndDragDropTarget();
    }

    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
        ImGuiDragDropFlags target_flags = ImGuiDragDropFlags_AcceptBeforeDelivery;
        if (const ImGuiPayload* payload = ImGui::GetDragDropPayload()) {
            if (payload->IsDataType("external_files")) highlightDragAndDrop();
        }
    }

    if (isExternalDragHovering) highlightDragAndDrop();

    windowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

    if (windowFocused) {
        if (!selectedFiles.empty() && ImGui::IsKeyPressed(ImGuiKey_Delete)) {
            // Trigger delete confirmation (handled in renderFileListing)
            showDeleteConfirmation = true;
        }
        if (ctrlPressed) {
            if (ImGui::IsKeyPressed(ImGuiKey_C)) copySelectedFiles(false);
            else if (ImGui::IsKeyPressed(ImGuiKey_X)) copySelectedFiles(true);
            else if (ImGui::IsKeyPressed(ImGuiKey_V) && !clipboardFiles.empty()) pasteFiles(currentPath);
            else if (ImGui::IsKeyPressed(ImGuiKey_A)) {
                selectedFiles.clear();
                for (const auto& file : files) {
                    if (currentLayout == LayoutType::GRID || !file.isDirectory) selectedFiles.insert(file.name);
                }
                if (!files.empty()) lastSelectedFile = files.back().name;
            }
            else if (!shiftPressed && ImGui::IsKeyPressed(ImGuiKey_Z)) { cmdHistory.undo(); scanDirectory(currentPath); }
            else if (shiftPressed && ImGui::IsKeyPressed(ImGuiKey_Z)) { cmdHistory.redo(); scanDirectory(currentPath); }
            else if (ImGui::IsKeyPressed(ImGuiKey_Y)) { cmdHistory.redo(); scanDirectory(currentPath); }
        }
    }

    ImGui::End();

    handleNewDirectory();
    handleRename();
}