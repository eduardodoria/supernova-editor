#include "Properties.h"

#include "imgui_internal.h"

#include "Util.h"
#include "external/IconsFontAwesome6.h"
#include "command/CommandHandle.h"
#include "command/type/PropertyCmd.h"
#include "command/type/EntityNameCmd.h"

#include <map>

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

Vector3 Editor::Properties::roundZero(const Vector3& val, const float threshold){
    return Vector3(
        (fabs(val.x) < threshold) ? 0.0f : val.x,
        (fabs(val.y) < threshold) ? 0.0f : val.y,
        (fabs(val.z) < threshold) ? 0.0f : val.z
    );
}

bool Editor::Properties::compareVectorFloat(const float* a, const float* b, size_t elements, const float threshold){
    for (size_t i = 0; i < elements; ++i) {
        if (fabs(a[i] - b[i]) > threshold) {
            return true;
        }
    }
    return false;
}

float Editor::Properties::getMaxLabelSize(std::map<std::string, PropertyData> props, const std::vector<std::string>& includes, const std::vector<std::string>& excludes){
    float maxLabelSize = ImGui::GetFontSize();

    for (auto& [name, prop] : props) {
        bool containsInclude = includes.empty() || 
            std::any_of(includes.begin(), includes.end(), [&name](const std::string& include) {
                return name.find(include) != std::string::npos;
            });

        bool containsExclude = !excludes.empty() && 
            std::any_of(excludes.begin(), excludes.end(), [&name](const std::string& exclude) {
                return name.find(exclude) != std::string::npos;
            });

        if (containsInclude && !containsExclude) {
            maxLabelSize = std::max(
                maxLabelSize, 
                ImGui::CalcTextSize((prop.label + ICON_FA_ROTATE_LEFT).c_str()).x
            );
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

bool Editor::Properties::propertyHeader(std::string label, float secondColSize, bool defChanged, bool child){
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, ImGui::GetStyle().ItemSpacing.y));
    ImGui::TableNextRow();
    if (child){
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_PopupBg]));
    }
    ImGui::TableNextColumn();
    //ImGui::Dummy(ImVec2(0, 10));
    ImGui::Text("%s", label.c_str());
    ImGui::SameLine();

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, ImGui::GetStyle().FramePadding.y));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
    bool button = false;
    if (defChanged){
        button = ImGui::Button((ICON_FA_ROTATE_LEFT"##"+label).c_str());
    }

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);

    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(secondColSize);

    return button;
}

void Editor::Properties::propertyRow(ComponentType cpType, std::map<std::string, PropertyData> props, std::string name, Scene* scene, std::vector<Entity> entities, float secondColSize, bool child){
    PropertyData prop = props[replaceNumberedBrackets(name)];

    static Command* cmd = nullptr;

    constexpr float compThreshold = 1e-4;
    constexpr float zeroThreshold = 1e-4;

    if (prop.type == PropertyType::Vector3){
        Vector3* value = nullptr;
        bool difX = false;
        bool difY = false;
        bool difZ = false;
        std::map<Entity, Vector3> eValue;
        for (Entity& entity : entities){
            eValue[entity] = *Catalog::getPropertyRef<Vector3>(scene, entity, cpType, name);
            if (value){
                if (std::fabs(value->x - eValue[entity].x) > compThreshold)
                    difX = true;
                if (std::fabs(value->y - eValue[entity].y) > compThreshold)
                    difY = true;
                if (std::fabs(value->z - eValue[entity].z) > compThreshold)
                    difZ = true;
            }
            value = &eValue[entity];
        }

        Vector3 newValue = *value;

        bool changed = compareVectorFloat((float*)&newValue, static_cast<float*>(prop.def), 3, compThreshold);
        if (propertyHeader(prop.label, secondColSize, changed, child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector3>(scene, entity, cpType, name, prop.updateFlags, *static_cast<Vector3*>(prop.def));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }

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

    }else if (prop.type == PropertyType::Vector4){
        Vector4* value = nullptr;
        bool difX = false;
        bool difY = false;
        bool difZ = false;
        bool difW = false;
        std::map<Entity, Vector4> eValue;
        for (Entity& entity : entities){
            eValue[entity] = *Catalog::getPropertyRef<Vector4>(scene, entity, cpType, name);
            if (value){
                if (std::fabs(value->x - eValue[entity].x) > compThreshold)
                    difX = true;
                if (std::fabs(value->y - eValue[entity].y) > compThreshold)
                    difY = true;
                if (std::fabs(value->z - eValue[entity].z) > compThreshold)
                    difZ = true;
                if (std::fabs(value->w - eValue[entity].w) > compThreshold)
                    difW = true;
            }
            value = &eValue[entity];
        }

        Vector4 newValue = *value;

        bool changed = compareVectorFloat((float*)&newValue, static_cast<float*>(prop.def), 4, compThreshold);
        if (propertyHeader(prop.label, secondColSize, changed, child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(scene, entity, cpType, name, prop.updateFlags, *static_cast<Vector4*>(prop.def));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }

        ImGui::BeginGroup();
        ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth());

        if (difX)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_x_"+name).c_str(), &(newValue.x), 0.1f, 0.0f, 0.0f, "%.2f")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(scene, entity, cpType, name, prop.updateFlags, Vector4(newValue.x, eValue[entity].y, eValue[entity].z, eValue[entity].w));
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
                cmd = new PropertyCmd<Vector4>(scene, entity, cpType, name, prop.updateFlags, Vector4(eValue[entity].x, newValue.y, eValue[entity].z, eValue[entity].w));
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
                cmd = new PropertyCmd<Vector4>(scene, entity, cpType, name, prop.updateFlags, Vector4(eValue[entity].x, eValue[entity].y, newValue.z, eValue[entity].w));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (difZ)
            ImGui::PopStyleColor();

        ImGui::SameLine();
        if (difW)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_w_"+name).c_str(), &(newValue.w), 0.1f, 0.0f, 0.0f, "%.2f")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(scene, entity, cpType, name, prop.updateFlags, Vector4(eValue[entity].x, eValue[entity].y, eValue[entity].z, newValue.w));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (difW)
            ImGui::PopStyleColor();

        ImGui::EndGroup();
        //ImGui::SetItemTooltip("%s (X, Y, Z)", prop.label.c_str());

    }else if (prop.type == PropertyType::Quat){
        RotationOrder order = RotationOrder::ZYX;
        Vector3* value = nullptr;
        bool difX = false;
        bool difY = false;
        bool difZ = false;
        std::map<Entity, Vector3> eValue;
        Quaternion qValue;
        for (Entity& entity : entities){
            qValue = *Catalog::getPropertyRef<Quaternion>(scene, entity, cpType, name);
            eValue[entity] = roundZero(Quaternion(qValue).normalize().getEulerAngles(order), zeroThreshold);
            if (value){
                if (std::fabs(value->x - eValue[entity].x) > compThreshold)
                    difX = true;
                if (std::fabs(value->y - eValue[entity].y) > compThreshold)
                    difY = true;
                if (std::fabs(value->z - eValue[entity].z) > compThreshold)
                    difZ = true;
            }
            value = &eValue[entity];
        }

        Vector3 newValue = *value;

        // using 'qValue' to compare quaternions
        bool changed = compareVectorFloat((float*)&qValue, static_cast<float*>(prop.def), 4, compThreshold);
        if (propertyHeader(prop.label, secondColSize, changed, child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Quaternion>(scene, entity, cpType, name, prop.updateFlags, *static_cast<Quaternion*>(prop.def));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }

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
        bool* value = nullptr;
        std::map<Entity, bool> eValue;
        bool dif = false;
        for (Entity& entity : entities){
            eValue[entity] = *Catalog::getPropertyRef<bool>(scene, entity, cpType, name);
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }

        bool newValue = *value;

        if (propertyHeader(prop.label, secondColSize, (newValue != *static_cast<bool*>(prop.def)), child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<bool>(scene, entity, cpType, name, prop.updateFlags, *static_cast<bool*>(prop.def));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_CheckMark, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::Checkbox(("##checkbox_"+name).c_str(), &newValue)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<bool>(scene, entity, cpType, name, prop.updateFlags, newValue);
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (dif)
            ImGui::PopStyleColor();
        //ImGui::SetItemTooltip("%s", prop.label.c_str());

    }else if (prop.type == PropertyType::Float_0_1){
        float* value = nullptr;
        std::map<Entity, float> eValue;
        bool dif = false;
        for (Entity& entity : entities){
            eValue[entity] = *Catalog::getPropertyRef<float>(scene, entity, cpType, name);
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }

        float newValue = *value;

        if (propertyHeader(prop.label, secondColSize, (newValue != *static_cast<float*>(prop.def)), child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<float>(scene, entity, cpType, name, prop.updateFlags, *static_cast<float*>(prop.def));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_CheckMark, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_float_"+name).c_str(), &newValue, 0.01f, 0.0f, 1.0f, "%.2f")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<float>(scene, entity, cpType, name, prop.updateFlags, newValue);
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (dif)
            ImGui::PopStyleColor();
        //ImGui::SetItemTooltip("%s", prop.label.c_str());

    }else if (prop.type == PropertyType::Color3L){
        Vector3* value = nullptr;
        std::map<Entity, Vector3> eValue;
        bool dif = false;
        for (Entity& entity : entities){
            eValue[entity] = *Catalog::getPropertyRef<Vector3>(scene, entity, cpType, name);
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }

        Vector3 newValue = Color::linearTosRGB(*value);

        // using 'value' beacause it is linear too
        bool changed = compareVectorFloat((float*)value, static_cast<float*>(prop.def), 3, compThreshold);
        if (propertyHeader(prop.label, secondColSize, changed, child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector3>(scene, entity, cpType, name, prop.updateFlags, *static_cast<Vector3*>(prop.def));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::ColorEdit3((prop.label+"##checkbox_"+name).c_str(), (float*)&newValue.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector3>(scene, entity, cpType, name, prop.updateFlags, Color::sRGBToLinear(newValue));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (dif)
            ImGui::PopStyleColor();
        //ImGui::SetItemTooltip("%s", prop.label.c_str());

    }else if (prop.type == PropertyType::Color4L){
        Vector4* value = nullptr;
        std::map<Entity, Vector4> eValue;
        bool dif = false;
        for (Entity& entity : entities){
            eValue[entity] = *Catalog::getPropertyRef<Vector4>(scene, entity, cpType, name);
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }

        Vector4 newValue = Color::linearTosRGB(*value);

        // using 'value' beacause it is linear too
        bool changed = compareVectorFloat((float*)value, static_cast<float*>(prop.def), 4, compThreshold);
        if (propertyHeader(prop.label, secondColSize, changed, child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(scene, entity, cpType, name, prop.updateFlags, *static_cast<Vector4*>(prop.def));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::ColorEdit4((prop.label+"##checkbox_"+name).c_str(), (float*)&newValue.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(scene, entity, cpType, name, prop.updateFlags, Color::sRGBToLinear(newValue));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (dif)
            ImGui::PopStyleColor();
        //ImGui::SetItemTooltip("%s", prop.label.c_str());

    }else if (prop.type == PropertyType::PrimitiveType){
        PrimitiveType* value = nullptr;
        std::map<Entity, PrimitiveType> eValue;
        bool dif = false;
        for (Entity& entity : entities){
            eValue[entity] = *Catalog::getPropertyRef<PrimitiveType>(scene, entity, cpType, name);
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }

        int item_current = Catalog::getPrimitiveTypeToIndex(*value);
        int item_default = Catalog::getPrimitiveTypeToIndex(*static_cast<PrimitiveType*>(prop.def));

        if (propertyHeader(prop.label, secondColSize, (item_current != item_default), child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<PrimitiveType>(scene, entity, cpType, name, prop.updateFlags, Catalog::getPrimitiveTypeFromIndex(item_default));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::Combo(("##combo_"+name).c_str(), &item_current, Catalog::getPrimitiveTypeArray().data(), Catalog::getPrimitiveTypeArray().size())){
            PrimitiveType newValue = Catalog::getPrimitiveTypeFromIndex(item_current);
            for (Entity& entity : entities){
                cmd = new PropertyCmd<PrimitiveType>(scene, entity, cpType, name, prop.updateFlags, newValue);
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (dif)
            ImGui::PopStyleColor();

    }else if (prop.type == PropertyType::Texture){
        Texture* value = nullptr;
        std::map<Entity, Texture> eValue;
        bool dif = false;
        for (Entity& entity : entities){
            eValue[entity] = *Catalog::getPropertyRef<Texture>(scene, entity, cpType, name);
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }

        Texture newValue = *value;

        if (propertyHeader(prop.label, secondColSize, (newValue != *static_cast<Texture*>(prop.def)), child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Texture>(scene, entity, cpType, name, prop.updateFlags, *static_cast<Texture*>(prop.def));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }

        ImGui::PushID(("texture_"+name).c_str());
        ImGui::PushStyleColor(ImGuiCol_ChildBg, textureLabel);

        // Use calculated width for the frame
        ImGui::BeginChild("textureframe", ImVec2(- ImGui::CalcTextSize(ICON_FA_GEAR).x - ImGui::GetStyle().ItemSpacing.x * 2 - ImGui::GetStyle().FramePadding.x * 2, ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2), 
            false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        std::string texName = newValue.getId();
        if (std::filesystem::exists(texName)) {
            texName = std::filesystem::path(texName).filename().string();
        }

        float textWidth = ImGui::CalcTextSize(texName.c_str()).x;
        float availWidth = ImGui::GetContentRegionAvail().x;
        ImGui::SetCursorPosX(availWidth - textWidth - 2);
        ImGui::SetCursorPosY(ImGui::GetStyle().FramePadding.y);
        if (dif)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        ImGui::Text("%s", texName.c_str());
        if (dif)
            ImGui::PopStyleColor();

        ImGui::EndChild();
        ImGui::SetItemTooltip("%s", newValue.getId().c_str());

        static std::map<std::string, bool> hasTextureDrag;
        static std::map<std::string, std::map<Entity, Texture>> originalTex;

        if (ImGui::BeginDragDropTarget()){

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource_files", ImGuiDragDropFlags_AcceptBeforeDelivery)) {
                std::vector<std::string> receivedStrings = Editor::Util::getStringsFromPayload(payload);
                if (receivedStrings.size() > 0){
                    if (!hasTextureDrag.count(name)){
                        hasTextureDrag[name] = true;
                        for (Entity& entity : entities){
                            Texture* valueRef = Catalog::getPropertyRef<Texture>(scene, entity, cpType, name);
                            originalTex[name][entity] = Texture(*valueRef);
                            if (*valueRef != Texture(receivedStrings[0])){
                                *valueRef = Texture(receivedStrings[0]);
                                scene->getComponent<MeshComponent>(entity).needReload = true;
                                //printf("reload %s\n", name.c_str());
                            }
                        }
                    }
                    if (payload->IsDelivery()){
                        Texture texture(receivedStrings[0]);
                        for (Entity& entity : entities){
                            Texture* valueRef = Catalog::getPropertyRef<Texture>(scene, entity, cpType, name);
                            *valueRef = originalTex[name][entity];
                            cmd = new PropertyCmd<Texture>(scene, entity, cpType, name, prop.updateFlags, texture);
                            cmd->setNoMerge();
                            CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
                        }

                        ImGui::SetWindowFocus();
                        hasTextureDrag.erase(name);
                        originalTex.erase(name);
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }else{
            if (hasTextureDrag.count(name) && hasTextureDrag[name]){
                for (Entity& entity : entities){
                    Texture* valueRef = Catalog::getPropertyRef<Texture>(scene, entity, cpType, name);
                    if (*valueRef != originalTex[name][entity]){
                        *valueRef = originalTex[name][entity];
                        scene->getComponent<MeshComponent>(entity).needReload = true;
                        //printf("reload %s\n", name.c_str());
                    }
                }

                hasTextureDrag.erase(name);
                originalTex.erase(name);
            }
        }
        //ImGui::SetItemTooltip("%s", currentPath.c_str());
        ImGui::PopStyleColor();

        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_FILE_IMPORT)) {
            std::string path = Editor::Util::openFileDialog(project->getProjectPath().string(), true);
            if (!path.empty()) {
                std::filesystem::path projectPath = project->getProjectPath();
                std::filesystem::path filePath = std::filesystem::absolute(path);

                // Check if file path is within project directory
                std::error_code ec;
                auto relative = std::filesystem::relative(filePath, projectPath, ec);
                if (ec || relative.string().find("..") != std::string::npos) {
                    ImGui::OpenPopup("File Import Error");
                }else{
                    Texture texture(filePath.string());
                    for (Entity& entity : entities){
                        cmd = new PropertyCmd<Texture>(scene, entity, cpType, name, prop.updateFlags, texture);
                        cmd->setNoMerge();
                        CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
                    }
                }
            }
        }

        // Error popup modal
        if (ImGui::BeginPopupModal("File Import Error", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Selected file must be within the project directory.");
            ImGui::Separator();

            float buttonWidth = 120;
            float windowWidth = ImGui::GetWindowSize().x;
            ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();

    }

    if (ImGui::IsItemDeactivatedAfterEdit()) {
        cmd->setNoMerge();
        cmd = nullptr;
    }
}

void Editor::Properties::drawTransform(ComponentType cpType, std::map<std::string, PropertyData> props, Scene* scene, std::vector<Entity> entities){
    beginTable(cpType, getMaxLabelSize(props, {}, {"_billboard"}));

    propertyRow(cpType, props, "position", scene, entities);
    propertyRow(cpType, props, "rotation", scene, entities);
    propertyRow(cpType, props, "scale", scene, entities);
    propertyRow(cpType, props, "visible", scene, entities);
    propertyRow(cpType, props, "billboard", scene, entities);

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_GEAR)){
        ImGui::OpenPopup("menusettings_billboard");
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(19 * ImGui::GetFontSize(), 0), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::BeginPopup("menusettings_billboard")){
        ImGui::Text("Billboard settings");
        ImGui::Separator();

        beginTable(cpType, getMaxLabelSize(props), {"_billboard"});

        propertyRow(cpType, props, "fake_billboard", scene, entities);
        propertyRow(cpType, props, "cylindrical_billboard", scene, entities);
        propertyRow(cpType, props, "rotation_billboard", scene, entities, 12 * ImGui::GetFontSize());

        endTable();

        ImGui::EndPopup();
    }

    endTable();
}

void Editor::Properties::drawMeshComponent(ComponentType cpType, std::map<std::string, PropertyData> props, Scene* scene, std::vector<Entity> entities){
    beginTable(cpType, getMaxLabelSize(props, {}, {"submeshes"}));

    propertyRow(cpType, props, "cast_shadows", scene, entities);
    propertyRow(cpType, props, "receive_shadows", scene, entities);

    endTable();

    unsigned int numSubmeshes = 1;
    for (Entity& entity : entities){
        numSubmeshes = std::min(numSubmeshes, scene->getComponent<MeshComponent>(entity).numSubmeshes);
    }

    for (int s = 0; s < numSubmeshes; s++){
        ImGui::SeparatorText(("Submesh "+std::to_string(s+1)).c_str());

        float submeshesTableSize = getMaxLabelSize(props, {"submeshes"}, {"num", "material"});

        beginTable(cpType, submeshesTableSize, "submeshes");

        propertyHeader("Material");
        ImGui::Button("Load");
        ImGui::SameLine();

        static bool show_button_group = false;
        if (ImGui::ArrowButton("##toggle", show_button_group ? ImGuiDir_Down : ImGuiDir_Right)){
            show_button_group = !show_button_group;
        }

        if (show_button_group){
            endTable();
            beginTable(cpType, getMaxLabelSize(props, {"material"}), "material_table");
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.basecolor", scene, entities, -1, true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.basecolortexture", scene, entities, -1, true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.metallicfactor", scene, entities, 4 * ImGui::GetFontSize(), true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.roughnessfactor", scene, entities, 4 * ImGui::GetFontSize(), true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.metallicroughnesstexture", scene, entities, -1, true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.emissivefactor", scene, entities, -1, true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.emissivetexture", scene, entities, -1, true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.occlusiontexture", scene, entities, -1, true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.normalTexture", scene, entities, -1, true);
            if (!scene->isSceneAmbientLightEnabled()){
                propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.ambientlight", scene, entities, -1, true);
                propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.ambientintensity", scene, entities, 4 * ImGui::GetFontSize(), true);
            }
            endTable();
            beginTable(cpType, submeshesTableSize, "submeshes");
        }

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

        // to change component view order, need change ComponentType
        for (Entity& entity : entities){
            std::vector<ComponentType> oldComponents = components;
            std::vector<ComponentType> newComponents = Catalog::findComponents(scene, entity);

            if (!std::is_sorted(newComponents.begin(), newComponents.end())) {
                std::sort(newComponents.begin(), newComponents.end());
            }

            if (!components.empty()){
                components.clear();

                if (!std::is_sorted(oldComponents.begin(), oldComponents.end())) {
                    std::sort(oldComponents.begin(), oldComponents.end());
                }

                // Reserve memory for efficiency
                components.reserve(std::min(oldComponents.size(), newComponents.size()));
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
                    CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new EntityNameCmd(sceneProject, entities[0], nameBuffer));
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