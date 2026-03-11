#include "AnimationWindow.h"

#include "imgui_internal.h"
#include "external/IconsFontAwesome6.h"
#include "Catalog.h"
#include "command/CommandHandle.h"
#include "command/type/PropertyCmd.h"
#include "command/type/MultiPropertyCmd.h"

#include "component/ActionComponent.h"
#include "component/AnimationComponent.h"
#include "component/SpriteAnimationComponent.h"
#include "component/TimedActionComponent.h"

#include <algorithm>
#include <cmath>

using namespace Supernova;

Editor::AnimationWindow::AnimationWindow(Project* project){
    this->project = project;

    isPlaying = false;
    currentTime = 0;
    playbackSpeed = 1.0f;

    pixelsPerSecond = 100.0f;
    scrollX = 0;
    minPixelsPerSecond = 20.0f;
    maxPixelsPerSecond = 500.0f;

    selectedTrackIndex = -1;
    isDraggingPlayhead = false;
    isDraggingTrack = false;
    draggingTrackIndex = -1;
    dragStartTime = 0;

    snapToGrid = true;
    snapInterval = 0.1f;

    selectedEntity = NULL_ENTITY;
    selectedSceneId = 0;
}

float Editor::AnimationWindow::snapTime(float time) const {
    if (!snapToGrid || snapInterval <= 0) return time;
    return std::round(time / snapInterval) * snapInterval;
}

float Editor::AnimationWindow::timeToX(float time, float timeStart, ImVec2 canvasPos) const {
    return canvasPos.x + (time - timeStart) * pixelsPerSecond;
}

float Editor::AnimationWindow::xToTime(float x, float timeStart, ImVec2 canvasPos) const {
    return timeStart + (x - canvasPos.x) / pixelsPerSecond;
}

std::string Editor::AnimationWindow::getActionLabel(Entity actionEntity, Scene* scene) const {
    if (actionEntity == NULL_ENTITY) return "Empty";

    Signature sig = scene->getSignature(actionEntity);

    if (sig.test(scene->getComponentId<SpriteAnimationComponent>())) {
        SpriteAnimationComponent& sa = scene->getComponent<SpriteAnimationComponent>(actionEntity);
        if (!sa.name.empty()) return sa.name;
        return "SpriteAnimation";
    }

    if (sig.test(scene->getComponentId<TimedActionComponent>())) {
        return "TimedAction";
    }

    if (sig.test(scene->getComponentId<AnimationComponent>())) {
        AnimationComponent& a = scene->getComponent<AnimationComponent>(actionEntity);
        if (!a.name.empty()) return a.name;
        return "Animation";
    }

    return "Action";
}

void Editor::AnimationWindow::selectEntity(Entity entity, uint32_t sceneId) {
    selectedEntity = entity;
    selectedSceneId = sceneId;
    currentTime = 0;
    isPlaying = false;
    selectedTrackIndex = -1;
}

void Editor::AnimationWindow::drawToolbar(float width, AnimationComponent& anim, Scene* scene) {
    // Add Track
    if (ImGui::Button(ICON_FA_PLUS " Add Track")) {
        anim.actions.push_back({0.0f, 1.0f, NULL_ENTITY});
        selectedTrackIndex = anim.actions.size() - 1;
    }
    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();

    // Play/Pause
    if (isPlaying) {
        if (ImGui::Button(ICON_FA_PAUSE "##anim_pause")) {
            isPlaying = false;
        }
    } else {
        if (ImGui::Button(ICON_FA_PLAY "##anim_play")) {
            isPlaying = true;
        }
    }
    ImGui::SameLine();

    // Stop
    if (ImGui::Button(ICON_FA_STOP "##anim_stop")) {
        isPlaying = false;
        currentTime = 0;
    }
    ImGui::SameLine();

    // Time display
    ImGui::SetNextItemWidth(60);
    ImGui::DragFloat("##anim_time", &currentTime, 0.01f, 0.0f, FLT_MAX, "%.2fs");
    ImGui::SameLine();

    // Speed
    ImGui::SetNextItemWidth(50);
    ImGui::DragFloat("##anim_speed", &playbackSpeed, 0.01f, 0.01f, 10.0f, "%.1fx");
    ImGui::SameLine();

    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();

    // Zoom
    ImGui::Text("Zoom:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    ImGui::SliderFloat("##anim_zoom", &pixelsPerSecond, minPixelsPerSecond, maxPixelsPerSecond, "%.0f");
    ImGui::SameLine();

    // Snap
    ImGui::Checkbox("Snap", &snapToGrid);
    ImGui::SameLine();
    if (snapToGrid) {
        ImGui::SetNextItemWidth(50);
        ImGui::DragFloat("##snap_int", &snapInterval, 0.01f, 0.01f, 1.0f, "%.2f");
    }
}

void Editor::AnimationWindow::drawTimeRuler(ImVec2 canvasPos, ImVec2 canvasSize, float timeStart, float timeEnd) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    float rulerHeight = 20.0f;
    float labelWidth = 120.0f;

    // Label column header
    drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + labelWidth, canvasPos.y + rulerHeight),
                            IM_COL32(50, 50, 50, 255));
    drawList->AddText(ImVec2(canvasPos.x + 4, canvasPos.y + 3), IM_COL32(150, 150, 150, 255), "Track");

    // Timeline ruler background
    drawList->AddRectFilled(ImVec2(canvasPos.x + labelWidth, canvasPos.y),
                            ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + rulerHeight),
                            IM_COL32(40, 40, 40, 255));

    // Determine tick interval based on zoom
    float tickInterval = 1.0f;
    if (pixelsPerSecond > 200) tickInterval = 0.1f;
    else if (pixelsPerSecond > 80) tickInterval = 0.5f;

    float subTickInterval = tickInterval / 5.0f;
    ImVec2 timeOrigin(canvasPos.x + labelWidth, canvasPos.y);
    float timeAreaRight = canvasPos.x + canvasSize.x;

    // Draw ticks
    float t = std::floor(timeStart / tickInterval) * tickInterval;
    while (t <= timeEnd) {
        float x = timeToX(t, timeStart, timeOrigin);
        if (x >= timeOrigin.x && x <= timeAreaRight) {
            drawList->AddLine(ImVec2(x, canvasPos.y + rulerHeight - 10), ImVec2(x, canvasPos.y + rulerHeight),
                              IM_COL32(180, 180, 180, 255));

            char buf[16];
            snprintf(buf, sizeof(buf), "%.1fs", t);
            drawList->AddText(ImVec2(x + 2, canvasPos.y + 2), IM_COL32(180, 180, 180, 255), buf);
        }
        t += tickInterval;
    }

    // Sub-ticks
    t = std::floor(timeStart / subTickInterval) * subTickInterval;
    while (t <= timeEnd) {
        float x = timeToX(t, timeStart, timeOrigin);
        if (x >= timeOrigin.x && x <= timeAreaRight) {
            drawList->AddLine(ImVec2(x, canvasPos.y + rulerHeight - 5), ImVec2(x, canvasPos.y + rulerHeight),
                              IM_COL32(100, 100, 100, 255));
        }
        t += subTickInterval;
    }
}

void Editor::AnimationWindow::drawTracks(ImVec2 canvasPos, ImVec2 canvasSize, float timeStart, float timeEnd,
                                          AnimationComponent& anim, SceneProject* sceneProject) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    Scene* scene = sceneProject->scene;
    float rulerHeight = 20.0f;
    float trackHeight = 24.0f;
    float trackPadding = 2.0f;
    float labelWidth = 120.0f;

    ImU32 trackColors[] = {
        IM_COL32(70, 130, 180, 200),  // Steel blue
        IM_COL32(180, 100, 70, 200),  // Warm red
        IM_COL32(70, 180, 100, 200),  // Green
        IM_COL32(180, 170, 70, 200),  // Gold
        IM_COL32(140, 70, 180, 200),  // Purple
    };
    int numColors = sizeof(trackColors) / sizeof(trackColors[0]);

    for (size_t i = 0; i < anim.actions.size(); i++) {
        ActionFrame& frame = anim.actions[i];
        float trackY = canvasPos.y + rulerHeight + i * (trackHeight + trackPadding);

        // Track background
        ImU32 bgColor = (i % 2 == 0) ? IM_COL32(35, 35, 35, 255) : IM_COL32(45, 45, 45, 255);
        drawList->AddRectFilled(ImVec2(canvasPos.x, trackY),
                                ImVec2(canvasPos.x + canvasSize.x, trackY + trackHeight), bgColor);

        // Label area
        std::string label = std::to_string(i) + ": " + getActionLabel(frame.action, scene);
        drawList->AddRectFilled(ImVec2(canvasPos.x, trackY),
                                ImVec2(canvasPos.x + labelWidth, trackY + trackHeight),
                                IM_COL32(50, 50, 50, 255));
        drawList->AddText(ImVec2(canvasPos.x + 4, trackY + 4), IM_COL32(200, 200, 200, 255), label.c_str());

        // Action frame block
        float blockStart = timeToX(frame.startTime, timeStart, ImVec2(canvasPos.x + labelWidth, 0));
        float blockEnd = timeToX(frame.startTime + frame.duration, timeStart, ImVec2(canvasPos.x + labelWidth, 0));

        // Clamp to visible area
        float visStart = std::max(blockStart, canvasPos.x + labelWidth);
        float visEnd = std::min(blockEnd, canvasPos.x + canvasSize.x);

        if (visEnd > visStart) {
            ImU32 blockColor = trackColors[i % numColors];
            if ((int)i == selectedTrackIndex) {
                blockColor = IM_COL32(255, 200, 50, 220); // Selected highlight
            }
            drawList->AddRectFilled(ImVec2(visStart, trackY + 2), ImVec2(visEnd, trackY + trackHeight - 2),
                                    blockColor, 3.0f);

            // Block label
            std::string blockLabel = getActionLabel(frame.action, scene);
            float textWidth = ImGui::CalcTextSize(blockLabel.c_str()).x;
            if (visEnd - visStart > textWidth + 8) {
                drawList->AddText(ImVec2(visStart + 4, trackY + 4),
                                  IM_COL32(255, 255, 255, 255), blockLabel.c_str());
            }

            // Interaction: click to select, drag to move
            ImVec2 blockMin(visStart, trackY);
            ImVec2 blockMax(visEnd, trackY + trackHeight);
            ImGui::SetCursorScreenPos(blockMin);
            ImGui::InvisibleButton(("track_" + std::to_string(i)).c_str(),
                                   ImVec2(blockMax.x - blockMin.x, blockMax.y - blockMin.y));

            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                selectedTrackIndex = (int)i;
                isDraggingTrack = true;
                draggingTrackIndex = (int)i;
                dragStartTime = frame.startTime;
            }
        }
    }

    // Handle track dragging
    if (isDraggingTrack && ImGui::IsMouseDragging(ImGuiMouseButton_Left) &&
        draggingTrackIndex >= 0 && draggingTrackIndex < (int)anim.actions.size()) {
        float mouseDragDeltaX = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
        float timeDelta = mouseDragDeltaX / pixelsPerSecond;
        float newStart = dragStartTime + timeDelta;

        ActionFrame& frame = anim.actions[draggingTrackIndex];
        if (newStart >= 0) {
            frame.startTime = snapTime(newStart);
            sceneProject->isModified = true; // Mark scene as modified!
        }
    }

    if (isDraggingTrack && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        if (draggingTrackIndex >= 0 && draggingTrackIndex < (int)anim.actions.size()) {
            ActionFrame& frame = anim.actions[draggingTrackIndex];

            if (frame.startTime != dragStartTime) {
                float finalStartTime = frame.startTime;
                frame.startTime = dragStartTime;

                auto* cmd = new PropertyCmd<float>(
                    project, 
                    selectedSceneId, 
                    selectedEntity, 
                    ComponentType::AnimationComponent, 
                    "actions[" + std::to_string(draggingTrackIndex) + "].startTime", 
                    finalStartTime, 
                    [sceneProject]() { sceneProject->isModified = true; }
                );
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }

        isDraggingTrack = false;
        draggingTrackIndex = -1;
    }
}

void Editor::AnimationWindow::drawPlayhead(ImVec2 canvasPos, ImVec2 canvasSize, float timeStart, float timeEnd) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    float labelWidth = 120.0f;

    float playheadX = canvasPos.x + labelWidth + (currentTime - timeStart) * pixelsPerSecond;

    if (playheadX >= canvasPos.x + labelWidth && playheadX <= canvasPos.x + canvasSize.x) {
        drawList->AddLine(ImVec2(playheadX, canvasPos.y),
                          ImVec2(playheadX, canvasPos.y + canvasSize.y),
                          IM_COL32(255, 80, 80, 255), 2.0f);

        // Playhead triangle at top
        drawList->AddTriangleFilled(
            ImVec2(playheadX - 5, canvasPos.y),
            ImVec2(playheadX + 5, canvasPos.y),
            ImVec2(playheadX, canvasPos.y + 8),
            IM_COL32(255, 80, 80, 255));
    }

    // Playhead drag interaction on ruler area
    ImVec2 rulerMin(canvasPos.x + labelWidth, canvasPos.y);
    ImVec2 rulerMax(canvasPos.x + canvasSize.x, canvasPos.y + 20);
    ImGui::SetCursorScreenPos(rulerMin);
    ImGui::InvisibleButton("##playhead_drag", ImVec2(rulerMax.x - rulerMin.x, rulerMax.y - rulerMin.y));

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))) {
        float mouseX = ImGui::GetIO().MousePos.x;
        float newTime = (mouseX - canvasPos.x - labelWidth) / pixelsPerSecond + timeStart;
        currentTime = snapTime(std::max(0.0f, newTime));
        isDraggingPlayhead = true;
    }
    if (isDraggingPlayhead && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        isDraggingPlayhead = false;
    }
}

void Editor::AnimationWindow::show() {
    ImGui::Begin(AnimationWindow::WINDOW_NAME);

    SceneProject* sceneProject = project->getSelectedScene();
    if (!sceneProject) {
        ImGui::TextDisabled("No scene selected.");
        ImGui::End();
        return;
    }

    Scene* scene = sceneProject->scene;
    if (!scene) {
        ImGui::TextDisabled("No scene available.");
        ImGui::End();
        return;
    }

    std::vector<Entity> selectedEntities = project->getSelectedEntities(sceneProject->id);

    // Find the first selected entity with AnimationComponent
    AnimationComponent* animComp = nullptr;
    Entity animEntity = NULL_ENTITY;

    for (Entity entity : selectedEntities) {
        if (scene->findComponent<AnimationComponent>(entity)) {
            animEntity = entity;
            animComp = &scene->getComponent<AnimationComponent>(entity);
            break;
        }
    }

    if (!animComp) {
        selectedEntity = NULL_ENTITY;
        selectedSceneId = 0;
        ImGui::TextDisabled("Select an entity with AnimationComponent to edit its timeline.");
        ImGui::End();
        return;
    }

    // Update cached selection
    selectedEntity = animEntity;
    selectedSceneId = sceneProject->id;

    // Playback update
    if (isPlaying) {
        float dt = ImGui::GetIO().DeltaTime * playbackSpeed;
        currentTime += dt;

        float duration = animComp->duration;
        if (duration > 0 && currentTime > duration) {
            if (animComp->loop) {
                currentTime = std::fmod(currentTime, duration);
            } else {
                currentTime = duration;
                isPlaying = false;
            }
        }
    }

    // Draw toolbar
    drawToolbar(ImGui::GetContentRegionAvail().x, *animComp, scene);

    ImGui::Separator();

    // Timeline canvas
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    if (canvasSize.y < 60) canvasSize.y = 60;

    float labelWidth = 120.0f;
    float timeStart = scrollX / pixelsPerSecond;
    float visibleTime = (canvasSize.x - labelWidth) / pixelsPerSecond;
    float timeEnd = timeStart + visibleTime;

    // Handle mouse wheel zoom
    if (ImGui::IsWindowHovered() && ImGui::GetIO().MouseWheel != 0) {
        float zoomFactor = 1.0f + ImGui::GetIO().MouseWheel * 0.1f;
        pixelsPerSecond = std::clamp(pixelsPerSecond * zoomFactor, minPixelsPerSecond, maxPixelsPerSecond);
    }

    // Draw background
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                            IM_COL32(30, 30, 30, 255));

    drawTimeRuler(canvasPos, canvasSize, timeStart, timeEnd);
    drawTracks(canvasPos, canvasSize, timeStart, timeEnd, *animComp, sceneProject);
    drawPlayhead(canvasPos, canvasSize, timeStart, timeEnd);

    // Reserve space
    ImGui::Dummy(canvasSize);

    // Horizontal scroll
    float totalTime = 10.0f; // default visible range
    if (animComp->duration > 0) totalTime = animComp->duration * 1.5f;
    float maxScroll = std::max(0.0f, totalTime * pixelsPerSecond - canvasSize.x + labelWidth);
    if (scrollX > maxScroll) scrollX = maxScroll;

    // Selected track info
    if (selectedTrackIndex >= 0 && selectedTrackIndex < (int)animComp->actions.size()) {
        ImGui::Separator();
        ActionFrame& frame = animComp->actions[selectedTrackIndex];
        ImGui::Text("Track %d:", selectedTrackIndex);
        ImGui::SameLine();
        ImGui::Text("Start: %.2fs", frame.startTime);
        ImGui::SameLine();
        ImGui::Text("Duration: %.2fs", frame.duration);
        ImGui::SameLine();
        ImGui::Text("Action: %s", getActionLabel(frame.action, scene).c_str());
    }

    ImGui::End();
}
