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

float Editor::Properties::getMaxLabelSize(std::map<std::string, PropertyData> props, const std::string& include, const std::string& exclude){
    float maxLabelSize = ImGui::GetFontSize();

    for (auto& [name, prop] : props){

        bool containsInclude = name.find(include) != std::string::npos;
        bool containsExclude = name.find(exclude) != std::string::npos;

        if ((include.empty() || containsInclude) && (exclude.empty() || !containsExclude)){
            maxLabelSize = std::max(maxLabelSize, ImGui::CalcTextSize(prop.label.c_str()).x);
        }
    }

    return maxLabelSize;
}

void Editor::Properties::beginTable(ComponentType cpType, float firstColSize, std::string nameAddon){
    ImGui::PushItemWidth(-1);
    if (!nameAddon.empty()){
        nameAddon = "_"+nameAddon;
    }
    ImGui::BeginTable(("table_"+Catalog::getComponentName(cpType)+nameAddon).c_str(), 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV);
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

    static Command* cmd = nullptr;

    if (prop.type == PropertyType::Float3){
        Vector3* value = Catalog::getPropertyRef<Vector3>(scene, entity, cpType, name);
        Vector3 newValue = *value;
        if (ImGui::DragFloat3(("##input_"+name).c_str(), &(newValue.x), 0.1f, 0.0f, 0.0f, "%.2f")){
            cmd = new PropertyCmd<Vector3>(scene, entity, cpType, name, prop.updateFlags, newValue);
            CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
        }
        //ImGui::SetItemTooltip("%s (X, Y, Z)", prop.label.c_str());

    }else if (prop.type == PropertyType::Float4){
        Vector4* value = Catalog::getPropertyRef<Vector4>(scene, entity, cpType, name);
        Vector4 newValue = *value;
        if (ImGui::DragFloat4(("##input_"+name).c_str(), &(newValue.x), 1.0f, 0.0f, 0.0f, "%.2f")){
            cmd = new PropertyCmd<Vector4>(scene, entity, cpType, name, prop.updateFlags, newValue);
            CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
        }
        //ImGui::SetItemTooltip("%s (X, Y, Z)", prop.label.c_str());

    }else if (prop.type == PropertyType::Quat){
        Quaternion* value = Catalog::getPropertyRef<Quaternion>(scene, entity, cpType, name);
        Vector3 newValueFmt = value->getEulerAngles();
        if (ImGui::DragFloat3(("##input_"+name).c_str(), &(newValueFmt.x), 0.1f, 0.0f, 0.0f, "%.2f°")){
            Quaternion newValue(newValueFmt.x, newValueFmt.y, newValueFmt.z);
            cmd = new PropertyCmd<Quaternion>(scene, entity, cpType, name, prop.updateFlags, newValue);
            CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
        }
        //ImGui::SetItemTooltip("%s in degrees (X, Y, Z)", prop.label.c_str());

    }else if (prop.type == PropertyType::Bool){
        bool* value = Catalog::getPropertyRef<bool>(scene, entity, cpType, name);
        bool newValue = *value;
        if (ImGui::Checkbox(("##checkbox_"+name).c_str(), &newValue)){
            cmd = new PropertyCmd<bool>(scene, entity, cpType, name, prop.updateFlags, newValue);
            CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
        }
        //ImGui::SetItemTooltip("%s", prop.label.c_str());

    }else if (prop.type == PropertyType::PrimitiveType){
        PrimitiveType* value = Catalog::getPropertyRef<PrimitiveType>(scene, entity, cpType, name);
        int item_current = Catalog::getPrimitiveTypeToIndex(*value);
        if (ImGui::Combo(("##combo_"+name).c_str(), &item_current, Catalog::getPrimitiveTypeArray().data(), Catalog::getPrimitiveTypeArray().size())){
            PrimitiveType newValue = Catalog::getPrimitiveTypeFromIndex(item_current);
            cmd = new PropertyCmd<PrimitiveType>(scene, entity, cpType, name, prop.updateFlags, newValue);
            CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
        }
    }

    if (ImGui::IsItemDeactivatedAfterEdit()) {
        cmd->setNoMerge();
        cmd = nullptr;
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

        beginTable(cpType, getMaxLabelSize(props), "billboard");

        propertyRow(cpType, props, "fake_billboard", scene, entity);
        propertyRow(cpType, props, "cylindrical_billboard", scene, entity);
        propertyRow(cpType, props, "billboard_rotation", scene, entity, 12 * ImGui::GetFontSize());

        endTable();

        ImGui::EndPopup();
    }

    endTable();
}

void Editor::Properties::drawMeshComponent(ComponentType cpType, std::map<std::string, PropertyData> props, Scene* scene, Entity entity){
    beginTable(cpType, getMaxLabelSize(props, "", "submeshes"));

    propertyRow(cpType, props, "cast_shadows", scene, entity);
    propertyRow(cpType, props, "receive_shadows", scene, entity);

    endTable();

    unsigned int numSubmeshes = scene->getComponent<MeshComponent>(entity).numSubmeshes;
    //Submesh* submeshes = Catalog::getPropertyRef<Submesh>(scene, entity, cpType, "submeshes");

    for (int s = 0; s < numSubmeshes; s++){
        ImGui::SeparatorText(("Submesh "+std::to_string(s+1)).c_str());

        beginTable(cpType, getMaxLabelSize(props, "submeshes", "num"), "submeshes");

        propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].primitive_type", scene, entity);
        propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].face_culling", scene, entity);
        propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].texture_rect", scene, entity);

        endTable();
    }
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

        std::vector<ComponentType> oldComponents = components;
        std::vector<ComponentType> newComponents = Catalog::findComponents(scene, entity);

        if (!components.empty()){
            components.clear();
            std::set_intersection(
                oldComponents.begin(), oldComponents.end(),
                newComponents.begin(), newComponents.end(),
                std::back_inserter(components));
        }else{
            components = newComponents;
        }


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
        if (ImGui::CollapsingHeader(Catalog::getComponentName(cpType).c_str())){

            if (cpType == ComponentType::Transform){
                drawTransform(cpType, Catalog::findProperties(scene, entity, cpType), scene, entity);
            }else if (cpType == ComponentType::MeshComponent){
                drawMeshComponent(cpType, Catalog::findProperties(scene, entity, cpType), scene, entity);
            }

        }
    }
    ImGui::End();
}