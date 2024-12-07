#include "Properties.h"

#include "imgui.h"
#include "imgui_internal.h"

#include "external/IconsFontAwesome6.h"
#include "command/CommandHandle.h"
#include "command/type/PropertyCmd.h"
#include "command/type/EntityNameCmd.h"

using namespace Supernova;

Editor::Properties::Properties(Project* project){
    this->project = project;
}

std::string Editor::Properties::replaceNumberedBrackets(const std::string& input) {
    std::string result = input;
    size_t pos = 0;

    while ((pos = result.find('[', pos)) != std::string::npos) {
        size_t end_pos = result.find(']', pos);
        if (end_pos != std::string::npos && std::isdigit(result[pos + 1])) {
            result.replace(pos, end_pos - pos + 1, "[]");
            pos += 2;
        } else {
            ++pos;
        }
    }

    return result;
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

void Editor::Properties::propertyRow(ComponentType cpType, std::map<std::string, PropertyData> props, std::string name, Scene* scene, std::vector<Entity> entities, float secondColSize){
    PropertyData prop = props[replaceNumberedBrackets(name)];

    Entity entity = entities[0];

    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text("%s", prop.label.c_str());
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(secondColSize);

    static Command* cmd = nullptr;

    float epsilon = 1e-5;

    if (prop.type == PropertyType::Float3){
        Vector3* value = nullptr;
        bool difX = false;
        bool difY = false;
        bool difZ = false;
        std::map<Entity, Vector3> eValue;
        for (Entity& entity : entities){
            eValue[entity] = *Catalog::getPropertyRef<Vector3>(scene, entity, cpType, name);
            if (value){
                if (std::fabs(value->x - eValue[entity].x) > epsilon)
                    difX = true;
                if (std::fabs(value->y - eValue[entity].y) > epsilon)
                    difY = true;
                if (std::fabs(value->z - eValue[entity].z) > epsilon)
                    difZ = true;
            }
            value = &eValue[entity];
        }

        Vector3 newValue = *value;

        ImGui::BeginGroup();
        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        if (difX)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_x_"+name).c_str(), &(newValue.x), 0.1f, 0.0f, 0.0f, "%.2f")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector3>(scene, entity, cpType, name, prop.updateFlags, Vector3(newValue.x, eValue[entity].y, eValue[entity].z));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (difX)
            ImGui::PopStyleColor();
        ImGui::SameLine();
        if (difY)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_y_"+name).c_str(), &(newValue.y), 0.1f, 0.0f, 0.0f, "%.2f")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector3>(scene, entity, cpType, name, prop.updateFlags, Vector3(eValue[entity].x, newValue.y, eValue[entity].z));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (difY)
            ImGui::PopStyleColor();
        ImGui::SameLine();
        if (difZ)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_z_"+name).c_str(), &(newValue.z), 0.1f, 0.0f, 0.0f, "%.2f")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector3>(scene, entity, cpType, name, prop.updateFlags, Vector3(eValue[entity].x, eValue[entity].y, newValue.z));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (difZ)
            ImGui::PopStyleColor();
        ImGui::EndGroup();
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
        RotationOrder order = RotationOrder::ZYX;
        Vector3* value = nullptr;
        bool difX = false;
        bool difY = false;
        bool difZ = false;
        std::map<Entity, Vector3> eValue;
        for (Entity& entity : entities){
            eValue[entity] = Catalog::getPropertyRef<Quaternion>(scene, entity, cpType, name)->normalize().getEulerAngles(order);
            if (value){
                if (std::fabs(value->x - eValue[entity].x) > epsilon)
                    difX = true;
                if (std::fabs(value->y - eValue[entity].y) > epsilon)
                    difY = true;
                if (std::fabs(value->z - eValue[entity].z) > epsilon)
                    difZ = true;
            }
            value = &eValue[entity];
        }

        Vector3 newValue = *value;

        ImGui::BeginGroup();
        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        if (difX)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_x_"+name).c_str(), &(newValue.x), 0.1f, 0.0f, 0.0f, "%.2f°")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Quaternion>(scene, entity, cpType, name, prop.updateFlags, Quaternion(newValue.x, eValue[entity].y, eValue[entity].z, order));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (difX)
            ImGui::PopStyleColor();
        ImGui::SameLine();
        if (difY)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_y_"+name).c_str(), &(newValue.y), 0.1f, 0.0f, 0.0f, "%.2f°")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Quaternion>(scene, entity, cpType, name, prop.updateFlags, Quaternion(eValue[entity].x, newValue.y, eValue[entity].z, order));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (difY)
            ImGui::PopStyleColor();
        ImGui::SameLine();
        if (difZ)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_z_"+name).c_str(), &(newValue.z), 0.1f, 0.0f, 0.0f, "%.2f°")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Quaternion>(scene, entity, cpType, name, prop.updateFlags, Quaternion(eValue[entity].x, eValue[entity].y, newValue.z, order));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (difZ)
            ImGui::PopStyleColor();
        ImGui::EndGroup();
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

void Editor::Properties::drawTransform(ComponentType cpType, std::map<std::string, PropertyData> props, Scene* scene, std::vector<Entity> entities){
    beginTable(cpType, getMaxLabelSize(props));

    propertyRow(cpType, props, "position", scene, entities);
    propertyRow(cpType, props, "rotation", scene, entities);
    propertyRow(cpType, props, "scale", scene, entities);
    propertyRow(cpType, props, "visible", scene, entities);
    propertyRow(cpType, props, "billboard", scene, entities);

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_GEAR)){
        ImGui::OpenPopup("menusettings_billboard");
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(18 * ImGui::GetFontSize(), 0), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::BeginPopup("menusettings_billboard")){
        ImGui::Text("Billboard settings");
        ImGui::Separator();

        beginTable(cpType, getMaxLabelSize(props), "billboard");

        propertyRow(cpType, props, "fake_billboard", scene, entities);
        propertyRow(cpType, props, "cylindrical_billboard", scene, entities);
        propertyRow(cpType, props, "billboard_rotation", scene, entities, 12 * ImGui::GetFontSize());

        endTable();

        ImGui::EndPopup();
    }

    endTable();
}

void Editor::Properties::drawMeshComponent(ComponentType cpType, std::map<std::string, PropertyData> props, Scene* scene, std::vector<Entity> entities){
    beginTable(cpType, getMaxLabelSize(props, "", "submeshes"));

    propertyRow(cpType, props, "cast_shadows", scene, entities);
    propertyRow(cpType, props, "receive_shadows", scene, entities);

    endTable();

    unsigned int numSubmeshes = 1;
    for (Entity& entity : entities){
        numSubmeshes = std::min(numSubmeshes, scene->getComponent<MeshComponent>(entity).numSubmeshes);
    }

    for (int s = 0; s < numSubmeshes; s++){
        ImGui::SeparatorText(("Submesh "+std::to_string(s+1)).c_str());

        beginTable(cpType, getMaxLabelSize(props, "submeshes", "num"), "submeshes");

        propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].primitive_type", scene, entities);
        propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].face_culling", scene, entities);
        propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].texture_rect", scene, entities);

        endTable();
    }
}

void Editor::Properties::show(){
    ImGui::Begin("Properties");

    SceneProject* sceneProject = project->getSelectedScene();
    std::vector<Entity> entities = project->getSelectedEntities(sceneProject->id);

    std::vector<ComponentType> components;
    Scene* scene = sceneProject->scene;

    if (entities.size() > 0){

        for (Entity& entity : entities){
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
        }

        std::string names;
        bool firstEntity = true;
        for (Entity& entity : entities){
            if (!firstEntity){
                names += ", ";
            }
            names += scene->getEntityName(entity);
            firstEntity = false;
        }
        if (entities.size() == 1){
            ImGui::Text("Entity");
        }else{
            ImGui::Text("Entities (%lu)", entities.size());
        }
        ImGui::SetItemTooltip("%lu selected: %s", entities.size(), names.c_str());
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1);
        static char nameBuffer[128];
        strncpy(nameBuffer, names.c_str(), sizeof(nameBuffer) - 1);
        nameBuffer[sizeof(nameBuffer) - 1] = '\0';
        ImGui::BeginDisabled(entities.size() != 1);
        ImGui::InputText("##input_name", nameBuffer, IM_ARRAYSIZE(nameBuffer));
        ImGui::EndDisabled();
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            if (entities.size() == 1){
                if (nameBuffer[0] != '\0' && strcmp(nameBuffer, scene->getEntityName(entities[0]).c_str()) != 0) {
                    CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new EntityNameCmd(scene, entities[0], nameBuffer));
                }
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
                drawTransform(cpType, Catalog::getProperties(cpType, nullptr), scene, entities);
            }else if (cpType == ComponentType::MeshComponent){
                drawMeshComponent(cpType, Catalog::getProperties(cpType, nullptr), scene, entities);
            }

        }
    }
    ImGui::End();
}