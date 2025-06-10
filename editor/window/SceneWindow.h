#ifndef SCENEWINDOW_H
#define SCENEWINDOW_H

#include "imgui.h"
#include "imgui_internal.h"
#include "Project.h"
#include "object/Camera.h"
#include "command/CommandHandle.h"
#include "command/type/ScenePropertyCmd.h"
#include "command/type/PropertyCmd.h"
#include <unordered_map>

namespace Supernova::Editor {

    class SceneWindow {
    private:
        Project* project;
        bool windowFocused;

        bool mouseLeftDown = false;
        Vector2 mouseLeftStartPos;
        Vector2 mouseLeftDragPos;
        bool mouseLeftDraggedInside;

        std::map<uint32_t, bool> draggingMouse;
        std::map<uint32_t, float> walkSpeed;

        std::map<uint32_t, int> width;
        std::map<uint32_t, int> height;

        std::vector<uint32_t> closeSceneQueue;

        void handleCloseScene(uint32_t sceneId);
        void sceneEventHandler(Project* project, uint32_t sceneId);
        std::string getWindowTitle(const SceneProject& sceneProject) const;
        
    public:
        SceneWindow(Project* project);

        void show();
        bool isFocused() const;

        int getWidth(uint32_t sceneId) const;
        int getHeight(uint32_t sceneId) const;

        template<typename T>
        void drawSceneProperty(SceneProject* sceneProject, const std::string& propertyName, const char* label, float col2Size = -1.0f) {
            T value = Supernova::Editor::Catalog::getSceneProperty<T>(sceneProject->scene, propertyName);
            bool changed = false;

            Command* cmd = nullptr;

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", label);
            ImGui::TableSetColumnIndex(1);

            //ImGui::SetNextItemWidth(-1);
            if constexpr (std::is_same_v<T, bool>) {
                changed = ImGui::Checkbox(("##" + propertyName).c_str(), &value);
            } else if constexpr (std::is_same_v<T, float>) {
                changed = ImGui::DragFloat(("##" + propertyName).c_str(), &value, 0.01f);
            } else if constexpr (std::is_same_v<T, Vector3>) {
                changed = ImGui::ColorEdit3(("##" + propertyName).c_str(), (float*)&value.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
            } else if constexpr (std::is_same_v<T, Vector4>) {
                changed = ImGui::ColorEdit4(("##" + propertyName).c_str(), (float*)&value.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
            }

            if (changed) {
                cmd = new ScenePropertyCmd<T>(sceneProject, propertyName, value);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }

            if (ImGui::IsItemDeactivatedAfterEdit()) {
                if (cmd){
                    cmd->setNoMerge();
                    cmd = nullptr;
                }
            }
        }

        template<typename T>
        void drawProperty(SceneProject* sceneProject, Entity entity, ComponentType cpType, std::string propertyName, const char* label, float col2Size = -1.0f) {
            PropertyData prop = Catalog::findEntityProperties(sceneProject->scene, entity, cpType)[propertyName];
            T value = *static_cast<T*>(prop.ref);
            bool changed = false;

            Command* cmd = nullptr;

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", label);
            ImGui::TableSetColumnIndex(1);

            //ImGui::SetNextItemWidth(-1);
            if constexpr (std::is_same_v<T, bool>) {
                changed = ImGui::Checkbox(("##" + propertyName).c_str(), &value);
            } else if constexpr (std::is_same_v<T, float>) {
                changed = ImGui::DragFloat(("##" + propertyName).c_str(), &value, 0.01f);
            } else if constexpr (std::is_same_v<T, Vector3>) {
                changed = ImGui::DragFloat3(("##" + propertyName).c_str(), (float*)&value.x);
            } else if constexpr (std::is_same_v<T, Vector4>) {
                changed = ImGui::DragFloat4(("##" + propertyName).c_str(), (float*)&value.x);
            }

            if (changed) {
                cmd = new PropertyCmd<T>(sceneProject, entity, cpType, propertyName, prop.updateFlags, value);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }

            if (ImGui::IsItemDeactivatedAfterEdit()) {
                if (cmd){
                    cmd->setNoMerge();
                    cmd = nullptr;
                }
            }
        }
    };
}

#endif /* SCENEWINDOW_H */