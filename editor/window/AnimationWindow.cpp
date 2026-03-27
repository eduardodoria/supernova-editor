#include "AnimationWindow.h"

#include "imgui_internal.h"
#include "external/IconsFontAwesome6.h"
#include "Catalog.h"
#include "Stream.h"
#include "command/CommandHandle.h"
#include "command/type/PropertyCmd.h"
#include "command/type/MultiPropertyCmd.h"

#include "component/ActionComponent.h"
#include "component/AnimationComponent.h"
#include "component/SpriteAnimationComponent.h"
#include "component/TimedActionComponent.h"
#include "subsystem/ActionSystem.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>

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

    selectedFrameIndex = -1;
    isDraggingPlayhead = false;
    isDraggingFrame = false;
    draggingFrameIndex = -1;
    dragStartTime = 0;
    dragStartTrack = 0;

    isResizingFrame = false;
    resizingFrameIndex = -1;
    resizeSide = 0;
    resizeStartTime = 0;
    resizeStartDuration = 0;

    snapToGrid = true;
    snapInterval = 0.1f;

    autoFocusOnSelection = true;

    isPreviewing = false;

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
    if (selectedEntity != entity || selectedSceneId != sceneId) {
        // Stop any active preview before changing entity
        if (isPreviewing) {
            SceneProject* sceneProject = project->getScene(selectedSceneId);
            if (sceneProject && sceneProject->scene) {
                stopPreview(sceneProject->scene, sceneProject);
            } else {
                previewState.clear();
                isPreviewing = false;
            }
        }
        selectedEntity = entity;
        selectedSceneId = sceneId;
        currentTime = 0;
        isPlaying = false;
        selectedFrameIndex = -1;
    }
}

bool Editor::AnimationWindow::canPreviewEntity(Entity entity, Scene* scene) const {
    if (!scene || entity == NULL_ENTITY || !scene->isEntityCreated(entity)) {
        return false;
    }

    Signature signature = scene->getSignature(entity);
    return signature.test(scene->getComponentId<AnimationComponent>()) &&
           signature.test(scene->getComponentId<ActionComponent>());
}

void Editor::AnimationWindow::collectPreviewEntitiesRecursive(Scene* scene, Entity entity, std::vector<Entity>& entities,
                                                              std::unordered_set<Entity>& visitedAnimations,
                                                              std::unordered_set<Entity>& collectedEntities) const {
    if (!canPreviewEntity(entity, scene) || !visitedAnimations.insert(entity).second) {
        return;
    }

    if (collectedEntities.insert(entity).second) {
        entities.push_back(entity);
    }

    const AnimationComponent& animation = scene->getComponent<AnimationComponent>(entity);
    for (const ActionFrame& frame : animation.actions) {
        Entity actionEntity = frame.action;
        if (actionEntity == NULL_ENTITY || !scene->isEntityCreated(actionEntity)) {
            continue;
        }

        Signature actionSignature = scene->getSignature(actionEntity);
        if (collectedEntities.insert(actionEntity).second) {
            entities.push_back(actionEntity);
        }

        if (actionSignature.test(scene->getComponentId<ActionComponent>())) {
            const ActionComponent& action = scene->getComponent<ActionComponent>(actionEntity);
            if (action.target != NULL_ENTITY && scene->isEntityCreated(action.target) &&
                collectedEntities.insert(action.target).second) {
                entities.push_back(action.target);
            }
        }

        if (actionSignature.test(scene->getComponentId<AnimationComponent>()) &&
            actionSignature.test(scene->getComponentId<ActionComponent>())) {
            collectPreviewEntitiesRecursive(scene, actionEntity, entities, visitedAnimations, collectedEntities);
        }
    }
}

Editor::AnimationWindow::PreviewEntityState Editor::AnimationWindow::buildPreviewEntityState(Scene* scene, Entity entity) const {
    PreviewEntityState state;
    state.entity = entity;

    if (Transform* transform = scene->findComponent<Transform>(entity)) {
        state.parent = transform->parent;
    }

    YAML::Node components = Stream::encodeComponents(entity, scene, scene->getSignature(entity));
    components.remove(Catalog::getComponentName(ComponentType::AnimationComponent, true));
    state.components = components;

    return state;
}

void Editor::AnimationWindow::startPreview(Scene* scene, SceneProject* sceneProject) {
    if (isPreviewing || !canPreviewEntity(selectedEntity, scene)) {
        return;
    }

    previewState.clear();

    std::vector<Entity> previewEntities;
    std::unordered_set<Entity> visitedAnimations;
    std::unordered_set<Entity> collectedEntities;
    collectPreviewEntitiesRecursive(scene, selectedEntity, previewEntities, visitedAnimations, collectedEntities);

    previewState.reserve(previewEntities.size());
    for (Entity entity : previewEntities) {
        previewState.push_back(buildPreviewEntityState(scene, entity));
    }

    ActionComponent& action = scene->getComponent<ActionComponent>(selectedEntity);
    action.timecount = 0;
    action.stopTrigger = false;
    action.pauseTrigger = false;
    action.startTrigger = true;

    isPreviewing = true;
}

void Editor::AnimationWindow::stopPreview(Scene* scene, SceneProject* sceneProject) {
    if (!isPreviewing) return;

    for (const PreviewEntityState& state : previewState) {
        if (state.entity == NULL_ENTITY || !scene->isEntityCreated(state.entity) || !state.components || state.components.IsNull()) {
            continue;
        }

        Stream::decodeComponents(state.entity, state.parent, scene, state.components);
    }

    previewState.clear();
    isPreviewing = false;
    isDraggingFrame = false;
    draggingFrameIndex = -1;
    isResizingFrame = false;
    resizingFrameIndex = -1;
    resizeSide = 0;
    if (sceneProject) {
        sceneProject->needUpdateRender = true;
    }
}

std::string Editor::AnimationWindow::getAnimationEntityLabel(Entity entity, AnimationComponent& anim, Scene* scene) const {
    std::string entityName = scene->getEntityName(entity);
    std::string label = entityName.empty() ? "Entity " + std::to_string(entity) : entityName;
    if (!anim.name.empty()) {
        label += " (" + anim.name + ")";
    }
    return label;
}

void Editor::AnimationWindow::drawToolbar(float width, AnimationComponent& anim, Scene* scene, SceneProject* sceneProject) {
    // Animation entity combo selector
    auto animations = scene->getComponentArray<AnimationComponent>();
    std::string currentLabel = getAnimationEntityLabel(selectedEntity, anim, scene);
    bool canPreview = canPreviewEntity(selectedEntity, scene);

    float textWidth = ImGui::CalcTextSize(currentLabel.c_str()).x;
    float arrowWidth = ImGui::GetFrameHeight();
    float padding = ImGui::GetStyle().FramePadding.x * 2.0f;
    float minWidth = 150.0f;
    float desiredWidth = textWidth + arrowWidth + padding + 10.0f;

    ImGui::SetNextItemWidth(std::max(minWidth, desiredWidth));
    if (ImGui::BeginCombo("##anim_entity_combo", currentLabel.c_str())) {
        for (int i = 0; i < (int)animations->size(); i++) {
            Entity entity = animations->getEntity(i);
            AnimationComponent& animItem = animations->getComponentFromIndex(i);
            std::string label = getAnimationEntityLabel(entity, animItem, scene);

            bool isSelected = (entity == selectedEntity);
            if (ImGui::Selectable(label.c_str(), isSelected)) {
                selectEntity(entity, selectedSceneId);
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();

    // Add Action
    ImGui::BeginDisabled(isPreviewing);
    if (ImGui::Button(ICON_FA_PLUS " Add Action")) {
        // If a frame is selected, add to the same track, otherwise track 0
        uint32_t targetTrack = 0;
        if (selectedFrameIndex >= 0 && selectedFrameIndex < (int)anim.actions.size()) {
            targetTrack = anim.actions[selectedFrameIndex].track;
        }

        ActionFrame newFrame = {0.0f, 1.0f, NULL_ENTITY, targetTrack};

        // Find non-overlapping track
        bool overlap;
        do {
            overlap = false;
            for (const auto& a : anim.actions) {
                if (a.track == newFrame.track) {
                    float startA = a.startTime;
                    float endA = a.startTime + a.duration;
                    float startB = newFrame.startTime;
                    float endB = newFrame.startTime + newFrame.duration;
                    // Strict less-than for overlap means adjoining frames (e.g. 0-1 and 1-2) are OK
                    if (std::max(startA, startB) < std::min(endA, endB)) {
                        overlap = true;
                        newFrame.track++;
                        break;
                    }
                }
            }
        } while (overlap);

        anim.actions.push_back(newFrame);
        selectedFrameIndex = anim.actions.size() - 1;
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();

    // Play/Pause
    bool sceneIsStopped = (sceneProject->playState == ScenePlayState::STOPPED);
    if (isPlaying) {
        if (ImGui::Button(ICON_FA_PAUSE "##anim_pause")) {
            isPlaying = false;
        }
    } else {
        ImGui::BeginDisabled(sceneIsStopped && !canPreview);
        if (ImGui::Button(ICON_FA_PLAY "##anim_play")) {
            isPlaying = true;
            if (sceneIsStopped && !isPreviewing) {
                currentTime = 0;
                startPreview(scene, sceneProject);
            }
        }
        ImGui::EndDisabled();
        if (sceneIsStopped && !canPreview && ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Animation preview requires an ActionComponent on the selected animation entity.");
        }
    }
    ImGui::SameLine();

    // Stop
    if (ImGui::Button(ICON_FA_STOP "##anim_stop")) {
        isPlaying = false;
        currentTime = 0;
        if (isPreviewing) {
            stopPreview(scene, sceneProject);
        }
    }
    ImGui::SameLine();

    // Time display
    ImGui::SetNextItemWidth(60);
    ImGui::BeginDisabled(isPreviewing);
    ImGui::DragFloat("##anim_time", &currentTime, 0.01f, 0.0f, FLT_MAX, "%.2fs");
    ImGui::EndDisabled();
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

    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();

    // Settings
    if (ImGui::Button(ICON_FA_GEAR "##anim_settings")) {
        ImGui::OpenPopup("AnimationSettingsPopup");
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Animation Settings");
    }

    if (ImGui::BeginPopup("AnimationSettingsPopup")) {
        ImGui::Checkbox("Auto-focus on selection", &autoFocusOnSelection);
        ImGui::EndPopup();
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
    bool allowEditing = !isPreviewing;

    // Find highest track
    uint32_t maxTrack = 0;
    for (const auto& frame : anim.actions) {
        if (frame.track > maxTrack) {
            maxTrack = frame.track;
        }
    }
    // Always draw at least 3 tracks, or maxTrack + 2 to give space to drag down
    uint32_t numTracks = std::max((uint32_t)3, maxTrack + 2);

    // 1. Draw track backgrounds and labels
    for (uint32_t t = 0; t < numTracks; t++) {
        float trackY = canvasPos.y + rulerHeight + t * (trackHeight + trackPadding);

        // Track background
        ImU32 bgColor = (t % 2 == 0) ? IM_COL32(35, 35, 35, 255) : IM_COL32(45, 45, 45, 255);
        drawList->AddRectFilled(ImVec2(canvasPos.x, trackY),
                                ImVec2(canvasPos.x + canvasSize.x, trackY + trackHeight), bgColor);

        // Label area
        std::string label = "Track " + std::to_string(t);
        drawList->AddRectFilled(ImVec2(canvasPos.x, trackY),
                                ImVec2(canvasPos.x + labelWidth, trackY + trackHeight),
                                IM_COL32(50, 50, 50, 255));
        drawList->AddText(ImVec2(canvasPos.x + 4, trackY + 4), IM_COL32(200, 200, 200, 255), label.c_str());
    }

    // 2. Draw blocks
    for (size_t i = 0; i < anim.actions.size(); i++) {
        ActionFrame& frame = anim.actions[i];
        float trackY = canvasPos.y + rulerHeight + frame.track * (trackHeight + trackPadding);

        // Action frame block
        float blockStart = timeToX(frame.startTime, timeStart, ImVec2(canvasPos.x + labelWidth, 0));
        float blockEnd = timeToX(frame.startTime + frame.duration, timeStart, ImVec2(canvasPos.x + labelWidth, 0));

        // Clamp to visible area
        float visStart = std::max(blockStart, canvasPos.x + labelWidth);
        float visEnd = std::min(blockEnd, canvasPos.x + canvasSize.x);

        if (visEnd > visStart) {
            ImU32 blockColor = trackColors[frame.track % numColors];
            if ((int)i == selectedFrameIndex) {
                blockColor = IM_COL32(255, 200, 50, 220); // Selected highlight
            }
            drawList->AddRectFilled(ImVec2(visStart, trackY + 2), ImVec2(visEnd, trackY + trackHeight - 2),
                                    blockColor, 3.0f);

            // Block label
            std::string blockLabel = std::to_string(i) + ": " + getActionLabel(frame.action, scene);
            float textWidth = ImGui::CalcTextSize(blockLabel.c_str()).x;
            if (visEnd - visStart > textWidth + 8) {
                drawList->AddText(ImVec2(visStart + 4, trackY + 4),
                                  IM_COL32(255, 255, 255, 255), blockLabel.c_str());
            }

            // Interaction: click to select, drag to move/resize
            float edgeZone = 5.0f;
            ImVec2 blockMin(visStart, trackY);
            ImVec2 blockMax(visEnd, trackY + trackHeight);
            ImGui::SetCursorScreenPos(blockMin);
            ImGui::InvisibleButton(("frame_" + std::to_string(i)).c_str(),
                                   ImVec2(blockMax.x - blockMin.x, blockMax.y - blockMin.y));

            // Detect edge hover for resize cursor
            bool hovered = ImGui::IsItemHovered();
            if (allowEditing && hovered && !isDraggingFrame && !isResizingFrame) {
                float mouseX = ImGui::GetIO().MousePos.x;
                bool onLeftEdge = (mouseX - blockStart) < edgeZone && (mouseX >= blockStart);
                bool onRightEdge = (blockEnd - mouseX) < edgeZone && (mouseX <= blockEnd);
                if (onLeftEdge || onRightEdge) {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                }
            }

            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                selectedFrameIndex = (int)i;
                if (allowEditing) {
                    float mouseX = ImGui::GetIO().MousePos.x;
                    bool onLeftEdge = (mouseX - blockStart) < edgeZone && (mouseX >= blockStart);
                    bool onRightEdge = (blockEnd - mouseX) < edgeZone && (mouseX <= blockEnd);

                    if (onLeftEdge || onRightEdge) {
                        isResizingFrame = true;
                        resizingFrameIndex = (int)i;
                        resizeSide = onLeftEdge ? -1 : 1;
                        resizeStartTime = frame.startTime;
                        resizeStartDuration = frame.duration;
                    } else {
                        isDraggingFrame = true;
                        draggingFrameIndex = (int)i;
                        dragStartTime = frame.startTime;
                        dragStartTrack = frame.track;
                    }
                }
            }
        }
    }

    // Handle frame dragging
    if (allowEditing && isDraggingFrame && ImGui::IsMouseDragging(ImGuiMouseButton_Left) &&
        draggingFrameIndex >= 0 && draggingFrameIndex < (int)anim.actions.size()) {
        float mouseDragDeltaX = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
        float timeDelta = mouseDragDeltaX / pixelsPerSecond;
        float newStart = dragStartTime + timeDelta;

        float mouseDragDeltaY = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y;
        int trackDelta = (int)std::round(mouseDragDeltaY / (trackHeight + trackPadding));
        int newTrack = std::max(0, (int)dragStartTrack + trackDelta);

        ActionFrame& frame = anim.actions[draggingFrameIndex];
        float snappedStart = std::max(0.0f, snapTime(newStart));
        float frameDur = frame.duration;

        // Find valid position on a track closest to desiredStart without overlap
        auto findValidPosition = [&](float desiredStart, int track) -> float {
            // Collect occupied intervals on this track (excluding dragged frame)
            std::vector<std::pair<float, float>> occupied;
            for (size_t i = 0; i < anim.actions.size(); i++) {
                if ((int)i == draggingFrameIndex) continue;
                const auto& a = anim.actions[i];
                if ((int)a.track == track) {
                    occupied.push_back({a.startTime, a.startTime + a.duration});
                }
            }

            if (occupied.empty()) return std::max(0.0f, desiredStart);

            std::sort(occupied.begin(), occupied.end());

            float bestStart = desiredStart;
            float bestDist = 1e9f;

            auto tryGap = [&](float gapLo, float gapHi) {
                float maxStart = gapHi - frameDur;
                if (maxStart >= gapLo) {
                    float clamped = std::clamp(desiredStart, gapLo, maxStart);
                    float dist = std::abs(clamped - desiredStart);
                    if (dist < bestDist) {
                        bestDist = dist;
                        bestStart = clamped;
                    }
                }
            };

            // Gap before first block
            tryGap(0.0f, occupied[0].first);
            // Gaps between blocks
            for (size_t j = 0; j + 1 < occupied.size(); j++) {
                tryGap(occupied[j].second, occupied[j + 1].first);
            }
            // Gap after last block
            tryGap(occupied.back().second, 1e9f);

            return bestStart;
        };

        float validPos = findValidPosition(snappedStart, newTrack);
        frame.startTime = validPos;
        frame.track = newTrack;
        sceneProject->isModified = true;
    }

    // Handle frame resizing
    if (allowEditing && isResizingFrame && ImGui::IsMouseDragging(ImGuiMouseButton_Left) &&
        resizingFrameIndex >= 0 && resizingFrameIndex < (int)anim.actions.size()) {
        float mouseDragDeltaX = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
        float timeDelta = mouseDragDeltaX / pixelsPerSecond;
        ActionFrame& frame = anim.actions[resizingFrameIndex];
        float minDuration = 0.01f;

        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

        // Find nearest neighbors on the same track to prevent overlap
        float neighborLeftEnd = 0.0f;   // max end time of actions to our left
        float neighborRightStart = 1e9f; // min start time of actions to our right
        float origEnd = resizeStartTime + resizeStartDuration;
        for (size_t i = 0; i < anim.actions.size(); i++) {
            if ((int)i == resizingFrameIndex) continue;
            const auto& a = anim.actions[i];
            if (a.track != frame.track) continue;
            float aEnd = a.startTime + a.duration;
            if (aEnd <= resizeStartTime + 0.001f) {
                neighborLeftEnd = std::max(neighborLeftEnd, aEnd);
            }
            if (a.startTime >= origEnd - 0.001f) {
                neighborRightStart = std::min(neighborRightStart, a.startTime);
            }
        }

        if (resizeSide == 1) {
            // Right edge: only change duration, clamp to neighbor on right
            float newDuration = snapTime(resizeStartDuration + timeDelta);
            float maxDuration = neighborRightStart - frame.startTime;
            frame.duration = std::clamp(newDuration, minDuration, maxDuration);
        } else {
            // Left edge: move start and adjust duration (right end stays fixed)
            float newStart = snapTime(resizeStartTime + timeDelta);
            float endTime = resizeStartTime + resizeStartDuration;
            newStart = std::clamp(newStart, neighborLeftEnd, endTime - minDuration);
            frame.startTime = newStart;
            frame.duration = endTime - newStart;
        }
        sceneProject->isModified = true;
    }

    if (allowEditing && isResizingFrame && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        if (resizingFrameIndex >= 0 && resizingFrameIndex < (int)anim.actions.size()) {
            ActionFrame& frame = anim.actions[resizingFrameIndex];

            bool timeChanged = (frame.startTime != resizeStartTime);
            bool durationChanged = (frame.duration != resizeStartDuration);

            if (timeChanged || durationChanged) {
                float finalStartTime = frame.startTime;
                float finalDuration = frame.duration;

                // Revert so cmd logs exactly old -> new
                frame.startTime = resizeStartTime;
                frame.duration = resizeStartDuration;

                if (timeChanged && durationChanged) {
                    auto* multiCmd = new MultiPropertyCmd();
                    multiCmd->addPropertyCmd<float>(
                        project, selectedSceneId, selectedEntity, ComponentType::AnimationComponent,
                        "actions[" + std::to_string(resizingFrameIndex) + "].startTime", finalStartTime,
                        [sceneProject]() { sceneProject->isModified = true; }
                    );
                    multiCmd->addPropertyCmd<float>(
                        project, selectedSceneId, selectedEntity, ComponentType::AnimationComponent,
                        "actions[" + std::to_string(resizingFrameIndex) + "].duration", finalDuration,
                        [sceneProject]() { sceneProject->isModified = true; }
                    );
                    CommandHandle::get(sceneProject->id)->addCommand(multiCmd);
                } else if (durationChanged) {
                    auto* cmd = new PropertyCmd<float>(
                        project, selectedSceneId, selectedEntity, ComponentType::AnimationComponent,
                        "actions[" + std::to_string(resizingFrameIndex) + "].duration", finalDuration,
                        [sceneProject]() { sceneProject->isModified = true; }
                    );
                    CommandHandle::get(sceneProject->id)->addCommand(cmd);
                } else if (timeChanged) {
                    auto* cmd = new PropertyCmd<float>(
                        project, selectedSceneId, selectedEntity, ComponentType::AnimationComponent,
                        "actions[" + std::to_string(resizingFrameIndex) + "].startTime", finalStartTime,
                        [sceneProject]() { sceneProject->isModified = true; }
                    );
                    CommandHandle::get(sceneProject->id)->addCommand(cmd);
                }
            }
        }

        isResizingFrame = false;
        resizingFrameIndex = -1;
        resizeSide = 0;
    }

    if (allowEditing && isDraggingFrame && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        if (draggingFrameIndex >= 0 && draggingFrameIndex < (int)anim.actions.size()) {
            ActionFrame& frame = anim.actions[draggingFrameIndex];

            bool timeChanged = (frame.startTime != dragStartTime);
            bool trackChanged = (frame.track != dragStartTrack);

            if (timeChanged || trackChanged) {
                float finalStartTime = frame.startTime;
                uint32_t finalTrack = frame.track;

                // Revert so cmd logs exactly old -> new
                frame.startTime = dragStartTime;
                frame.track = dragStartTrack;

                if (timeChanged && trackChanged) {
                    auto* multiCmd = new MultiPropertyCmd();
                    multiCmd->addPropertyCmd<float>(
                        project, selectedSceneId, selectedEntity, ComponentType::AnimationComponent, 
                        "actions[" + std::to_string(draggingFrameIndex) + "].startTime", finalStartTime,
                        [sceneProject]() { sceneProject->isModified = true; }
                    );
                    multiCmd->addPropertyCmd<uint32_t>(
                        project, selectedSceneId, selectedEntity, ComponentType::AnimationComponent, 
                        "actions[" + std::to_string(draggingFrameIndex) + "].track", finalTrack,
                        [sceneProject]() { sceneProject->isModified = true; }
                    );
                    CommandHandle::get(sceneProject->id)->addCommand(multiCmd);
                } else if (timeChanged) {
                    auto* cmd = new PropertyCmd<float>(
                        project, selectedSceneId, selectedEntity, ComponentType::AnimationComponent, 
                        "actions[" + std::to_string(draggingFrameIndex) + "].startTime", finalStartTime, 
                        [sceneProject]() { sceneProject->isModified = true; }
                    );
                    CommandHandle::get(sceneProject->id)->addCommand(cmd);
                } else if (trackChanged) {
                    auto* cmd = new PropertyCmd<uint32_t>(
                        project, selectedSceneId, selectedEntity, ComponentType::AnimationComponent, 
                        "actions[" + std::to_string(draggingFrameIndex) + "].track", finalTrack, 
                        [sceneProject]() { sceneProject->isModified = true; }
                    );
                    CommandHandle::get(sceneProject->id)->addCommand(cmd);
                }
            }
        }

        isDraggingFrame = false;
        draggingFrameIndex = -1;
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

    if (!isPreviewing && (ImGui::IsItemClicked(ImGuiMouseButton_Left) || (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)))) {
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

    auto restorePreviewScene = [&]() {
        SceneProject* previewSceneProject = project->getScene(selectedSceneId);
        if (previewSceneProject && previewSceneProject->scene) {
            stopPreview(previewSceneProject->scene, previewSceneProject);
        } else {
            previewState.clear();
            isPreviewing = false;
        }

        isPlaying = false;
    };

    SceneProject* sceneProject = project->getSelectedScene();
    if (!sceneProject) {
        if (isPreviewing) {
            restorePreviewScene();
        }
        ImGui::TextDisabled("No scene selected.");
        ImGui::End();
        return;
    }

    Scene* scene = sceneProject->scene;
    if (!scene) {
        if (isPreviewing) {
            restorePreviewScene();
        }
        ImGui::TextDisabled("No scene available.");
        ImGui::End();
        return;
    }

    if (isPreviewing && sceneProject->id != selectedSceneId) {
        restorePreviewScene();
        currentTime = 0;
    }

    // Collect all entities with AnimationComponent in the scene
    auto animations = scene->getComponentArray<AnimationComponent>();
    if (animations->size() == 0) {
        if (isPreviewing) {
            restorePreviewScene();
            currentTime = 0;
        }
        selectedEntity = NULL_ENTITY;
        selectedSceneId = 0;
        ImGui::TextDisabled("No entities with AnimationComponent in this scene.");
        ImGui::End();
        return;
    }

    // Validate current selection still exists
    if (selectedEntity != NULL_ENTITY && !scene->findComponent<AnimationComponent>(selectedEntity)) {
        if (isPreviewing) {
            restorePreviewScene();
            currentTime = 0;
        }
        selectedEntity = NULL_ENTITY;
    }

    // Auto-select from scene selection if user clicked a new animation entity
    // We only update if the selected entities in the project changed to avoid overriding combo selection
    static std::vector<Entity> lastSceneSelectedEntities;
    std::vector<Entity> selectedEntities = project->getSelectedEntities(sceneProject->id);

    if (selectedEntities != lastSceneSelectedEntities) {
        lastSceneSelectedEntities = selectedEntities;
        for (Entity entity : selectedEntities) {
            if (scene->findComponent<AnimationComponent>(entity)) {
                if (selectedEntity != entity) {
                    selectEntity(entity, sceneProject->id);
                }
                if (autoFocusOnSelection) {
                    ImGui::SetWindowFocus();
                }
                break;
            }
        }
    }

    // Default to first animation entity if nothing selected
    if (selectedEntity == NULL_ENTITY) {
        selectedEntity = animations->getEntity(0);
        selectedSceneId = sceneProject->id;
        currentTime = 0;
        isPlaying = false;
        selectedFrameIndex = -1;
    }

    AnimationComponent* animComp = &scene->getComponent<AnimationComponent>(selectedEntity);

    bool sceneIsStopped = (sceneProject->playState == ScenePlayState::STOPPED);
    if (isPreviewing && !sceneIsStopped) {
        stopPreview(scene, sceneProject);
        isPlaying = false;
        currentTime = 0;
    }

    // Playback update
    if (isPreviewing && sceneIsStopped) {
        // Engine-driven preview: ActionSystem processes the animation
        if (isPlaying) {
            float dt = ImGui::GetIO().DeltaTime * playbackSpeed;
            scene->getSystem<ActionSystem>()->updateAnimationPreview(dt, selectedEntity);
            sceneProject->needUpdateRender = true;
        }

        // Sync currentTime from engine state
        ActionComponent& action = scene->getComponent<ActionComponent>(selectedEntity);
        currentTime = action.timecount;

        // Check if animation finished naturally
        if (action.state == ActionState::Stopped && isPlaying) {
            float finishedTime = 0;
            if (animComp->duration > 0) {
                finishedTime = animComp->duration;
            } else {
                for (const ActionFrame& frame : animComp->actions) {
                    finishedTime = std::max(finishedTime, frame.startTime + frame.duration);
                }
            }

            isPlaying = false;
            stopPreview(scene, sceneProject);
            currentTime = finishedTime;
        }
    } else if (isPlaying) {
        // Fallback: visual-only playback (when scene is playing or no preview)
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
    drawToolbar(ImGui::GetContentRegionAvail().x, *animComp, scene, sceneProject);

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
    if (selectedFrameIndex >= 0 && selectedFrameIndex < (int)animComp->actions.size()) {
        ImGui::Separator();
        ActionFrame& frame = animComp->actions[selectedFrameIndex];
        ImGui::Text("Frame %d:", selectedFrameIndex);
        ImGui::SameLine();
        ImGui::Text("Track: %d", frame.track);
        ImGui::SameLine();
        ImGui::Text("Start: %.2fs", frame.startTime);
        ImGui::SameLine();
        ImGui::Text("Duration: %.2fs", frame.duration);
        ImGui::SameLine();
        ImGui::Text("Action: %s", getActionLabel(frame.action, scene).c_str());
    }

    ImGui::End();
}
