#include "Properties.h"

#include "imgui.h"

#include "external/IconsFontAwesome6.h"
#include "command/CommandHandle.h"
#include "command/type/PropertyCmd.h"
#include "command/type/EntityNameCmd.h"

using namespace Supernova;

Editor::Properties::Properties(Project* project){
    this->project = project;
}

void Editor::Properties::drawPropertiesTable(ComponentType cpType, std::vector<PropertyData> props, Scene* scene, Entity entity, std::string tableNameAddon, float secColSize){
    float firstColSize = ImGui::GetFontSize();

    for (PropertyData& prop : props){
        firstColSize = std::max(firstColSize, ImGui::CalcTextSize(prop.label.c_str()).x);
    }

    ImGui::PushItemWidth(-1);
    ImGui::BeginTable(("table_"+Metadata::getComponentName(cpType)+tableNameAddon).c_str(), 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV);
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, firstColSize);
    ImGui::TableSetupColumn("Value");

    for (PropertyData& prop : props){

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%s", prop.label.c_str());
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(secColSize);

        if (prop.type == PropertyType::Float3){
            Vector3* value = Metadata::getPropertyRef<Vector3>(scene, entity, cpType, prop.name);
            Vector3 newValue = *value;
            ImGui::InputFloat3(("##input_"+prop.name).c_str(), &(newValue.x), "%.2f");
            ImGui::SetItemTooltip("%s (X, Y, Z)", prop.label.c_str());
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new PropertyCmd<Vector3>(scene, entity, cpType, prop.name, prop.updateFlags, newValue));
            }

        }else if (prop.type == PropertyType::Quat){
            Quaternion* value = Metadata::getPropertyRef<Quaternion>(scene, entity, cpType, prop.name);
            Vector3 newValueFmt = value->getEulerAngles();
            ImGui::InputFloat3(("##input_"+prop.name).c_str(), &(newValueFmt.x), "%.2f");
            ImGui::SetItemTooltip("%s in degrees (X, Y, Z)", prop.label.c_str());
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                Quaternion newValue(newValueFmt.x, newValueFmt.y, newValueFmt.z);
                CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new PropertyCmd<Quaternion>(scene, entity, cpType, prop.name, prop.updateFlags, newValue));
            }

        }else if (prop.type == PropertyType::Bool){
            bool* value = Metadata::getPropertyRef<bool>(scene, entity, cpType, prop.name);
            bool newValue = *value;
            ImGui::Checkbox(("##checkbox_"+prop.name).c_str(), &newValue);
            ImGui::SetItemTooltip("%s", prop.label.c_str());
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new PropertyCmd<bool>(scene, entity, cpType, prop.name, prop.updateFlags, newValue));
            }
        }

        if (prop.childs.size() > 0){
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_GEAR)){
                ImGui::OpenPopup(("menusettings_"+prop.name).c_str());
            }
            //ImGui::SetNextWindowSizeConstraints(ImVec2(400, 0), ImVec2(FLT_MAX, FLT_MAX));
            if (ImGui::BeginPopup(("menusettings_"+prop.name).c_str())){
                ImGui::Text("%s settings", prop.label.c_str());
                ImGui::Separator();

                drawPropertiesTable(cpType, prop.childs, scene, entity, "_child_"+prop.name, 200);

                ImGui::EndPopup();
            }
        }

    }
    ImGui::EndTable();
}

void Editor::Properties::show(){
    ImGui::Begin("Properties");

    SceneProject* sceneProject = project->getSelectedScene();
    std::vector<Entity> entities = project->getSelectedEntities(sceneProject->id);

    std::vector<ComponentType> components;
    Entity entity = NULL_ENTITY;
    Scene* scene = sceneProject->scene;

    if (entities.size() > 0){
        entity = entities[0];
        components = Metadata::findComponents(scene, entity);

        ImGui::Text("Entity");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1);
        static char nameBuffer[128];
        strncpy(nameBuffer, scene->getEntityName(entity).c_str(), sizeof(nameBuffer) - 1);
        nameBuffer[sizeof(nameBuffer) - 1] = '\0';
        ImGui::InputText("##input_name", nameBuffer, IM_ARRAYSIZE(nameBuffer));
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            if (nameBuffer[0] != '\0' && strcmp(nameBuffer, scene->getEntityName(entity).c_str()) != 0) {
                CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new EntityNameCmd(scene, entity, nameBuffer));
            }
        }

        ImGui::Separator();

        if (ImGui::Button(ICON_FA_PLUS" New component", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            // Button was clicked
            //ImGui::Text("Button clicked!");
        }
    }

    for (ComponentType& cpType : components){

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader(Metadata::getComponentName(cpType).c_str())){

            std::vector<PropertyData> props = Metadata::findProperties(scene, entity, cpType);

            drawPropertiesTable(cpType, props, scene, entity);

        }
    }
    ImGui::End();
}