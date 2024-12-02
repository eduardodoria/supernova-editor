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

float Editor::Properties::getMaxLabelSize(std::map<std::string, PropertyData> props){
    float maxLabelSize = ImGui::GetFontSize();

    for (auto& [name, prop] : props){
        maxLabelSize = std::max(maxLabelSize, ImGui::CalcTextSize(prop.label.c_str()).x);
    }

    return maxLabelSize;
}

void Editor::Properties::beginTable(ComponentType cpType, float firstColSize, std::string nameAddon){
    ImGui::PushItemWidth(-1);
    ImGui::BeginTable(("table_"+Metadata::getComponentName(cpType)+nameAddon).c_str(), 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV);
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, firstColSize);
    ImGui::TableSetupColumn("Value");

}

void Editor::Properties::endTable(){
    ImGui::EndTable();
}

void Editor::Properties::propertyRow(ComponentType cpType, std::map<std::string, PropertyData> props, std::string name, Scene* scene, Entity entity, float secondColSize){
    PropertyData prop = props[name];

    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text("%s", prop.label.c_str());
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(secondColSize);

    if (prop.type == PropertyType::Float3){
        Vector3* value = Metadata::getPropertyRef<Vector3>(scene, entity, cpType, name);
        Vector3 newValue = *value;
        ImGui::InputFloat3(("##input_"+name).c_str(), &(newValue.x), "%.2f");
        ImGui::SetItemTooltip("%s (X, Y, Z)", prop.label.c_str());
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new PropertyCmd<Vector3>(scene, entity, cpType, name, prop.updateFlags, newValue));
        }

    }else if (prop.type == PropertyType::Quat){
        Quaternion* value = Metadata::getPropertyRef<Quaternion>(scene, entity, cpType, name);
        Vector3 newValueFmt = value->getEulerAngles();
        ImGui::InputFloat3(("##input_"+name).c_str(), &(newValueFmt.x), "%.2f");
        ImGui::SetItemTooltip("%s in degrees (X, Y, Z)", prop.label.c_str());
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            Quaternion newValue(newValueFmt.x, newValueFmt.y, newValueFmt.z);
            CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new PropertyCmd<Quaternion>(scene, entity, cpType, name, prop.updateFlags, newValue));
        }

    }else if (prop.type == PropertyType::Bool){
        bool* value = Metadata::getPropertyRef<bool>(scene, entity, cpType, name);
        bool newValue = *value;
        ImGui::Checkbox(("##checkbox_"+name).c_str(), &newValue);
        ImGui::SetItemTooltip("%s", prop.label.c_str());
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new PropertyCmd<bool>(scene, entity, cpType, name, prop.updateFlags, newValue));
        }
    }
}

void Editor::Properties::drawTransform(ComponentType cpType, std::map<std::string, PropertyData> props, Scene* scene, Entity entity){
    beginTable(cpType, getMaxLabelSize(props));

    propertyRow(cpType, props, "position", scene, entity);
    propertyRow(cpType, props, "rotation", scene, entity);
    propertyRow(cpType, props, "scale", scene, entity);
    propertyRow(cpType, props, "visible", scene, entity);
    propertyRow(cpType, props, "billboard", scene, entity);

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_GEAR)){
        ImGui::OpenPopup("menusettings_billboard");
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(18 * ImGui::GetFontSize(), 0), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::BeginPopup("menusettings_billboard")){
        ImGui::Text("Billboard settings");
        ImGui::Separator();

        beginTable(cpType, getMaxLabelSize(props), "_billboard");

        propertyRow(cpType, props, "fake_billboard", scene, entity);
        propertyRow(cpType, props, "cylindrical_billboard", scene, entity);
        propertyRow(cpType, props, "billboard_rotation", scene, entity, 12 * ImGui::GetFontSize());

        endTable();

        ImGui::EndPopup();
    }

    endTable();
}

void Editor::Properties::drawMeshComponent(ComponentType cpType, std::map<std::string, PropertyData> props, Scene* scene, Entity entity){
    beginTable(cpType, getMaxLabelSize(props));

    propertyRow(cpType, props, "castShadows", scene, entity);
    propertyRow(cpType, props, "receiveShadows", scene, entity);

    endTable();
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

            if (cpType == ComponentType::Transform){
                drawTransform(cpType, Metadata::findProperties(scene, entity, cpType), scene, entity);
            }else if (cpType == ComponentType::MeshComponent){
                drawMeshComponent(cpType, Metadata::findProperties(scene, entity, cpType), scene, entity);
            }

        }
    }
    ImGui::End();
}