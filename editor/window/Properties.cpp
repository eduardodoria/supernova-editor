#include "Properties.h"

#include "imgui_internal.h"

#include "util/Util.h"
#include "util/FileDialogs.h"
#include "external/IconsFontAwesome6.h"
#include "command/CommandHandle.h"
#include "command/type/PropertyCmd.h"
#include "command/type/EntityNameCmd.h"
#include "render/SceneRender2D.h"
#include "util/SHA1.h"

#include <map>

using namespace Supernova;

Editor::Properties::Properties(Project* project){
    this->project = project;
    this->cmd = nullptr;
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

float Editor::Properties::getLabelSize(std::string label){
    return ImGui::CalcTextSize((label + ICON_FA_ROTATE_LEFT).c_str()).x;
}

void Editor::Properties::helpMarker(std::string desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

Texture* Editor::Properties::findThumbnail(const std::string& path) {
    if (path.empty()) return nullptr;

    // Compute the thumbnail path
    std::filesystem::path texPath = path;
    std::filesystem::path projectPath = project->getProjectPath();
    std::filesystem::path thumbnailPath;

    // Only try if the texture path is not empty and is inside the project
    if (!texPath.empty() && texPath.is_absolute() && texPath.string().find(projectPath.string()) == 0) {
        // Replicate getThumbnailPath() logic from ResourcesWindow
        std::filesystem::path relPath = std::filesystem::relative(texPath, projectPath);
        std::string relPathStr = relPath.generic_string();

        std::string hash = SHA1::hash(relPathStr);
        thumbnailPath = projectPath / ".supernova" / "thumbs" / (hash + ".thumb.png");

        // If the thumbnail exists, load and use it
        if (std::filesystem::exists(thumbnailPath)) {
            // Check if we already have this thumbnail loaded in cache
            std::string thumbPathStr = thumbnailPath.string();
            auto thumbIt = thumbnailTextures.find(thumbPathStr);

            if (thumbIt == thumbnailTextures.end()) {
                // Load the thumbnail texture if not in cache
                Texture thumbTexture;
                thumbTexture.setPath(thumbPathStr);
                if (thumbTexture.load()) {
                    thumbnailTextures[thumbPathStr] = thumbTexture;
                    return &thumbnailTextures[thumbPathStr];
                }
            } else if (thumbIt->second.getRender()) {
                // Return cached texture
                return &thumbIt->second;
            }
        }
    }

    return nullptr;
}

void Editor::Properties::drawImageWithBorderAndRounding(Texture* texture, const ImVec2& size, float rounding, ImU32 border_col, float border_thickness) {
    if (!texture) return;

    ImTextureID tex_id = (ImTextureID)(intptr_t)texture->getRender()->getGLHandler();
    int texWidth = texture->getWidth();
    int texHeight = texture->getHeight();

    ImVec2 cursor = ImGui::GetCursorScreenPos();

    // Calculate source aspect and target aspect
    float srcAspect = static_cast<float>(texWidth) / texHeight;
    float dstAspect = size.x / size.y;

    // Default UVs (full image)
    ImVec2 uv0(0, 0);
    ImVec2 uv1(1, 1);

    // If aspect ratios differ, calculate the crop
    if (fabs(srcAspect - dstAspect) > 1e-3f) {
        if (srcAspect > dstAspect) {
            // Source is wider; crop left and right
            float newWidth = texHeight * dstAspect;
            float x0 = (texWidth - newWidth) / 2.0f;
            uv0.x = x0 / texWidth;
            uv1.x = (x0 + newWidth) / texWidth;
        } else {
            // Source is taller; crop top and bottom
            float newHeight = texWidth / dstAspect;
            float y0 = (texHeight - newHeight) / 2.0f;
            uv0.y = y0 / texHeight;
            uv1.y = (y0 + newHeight) / texHeight;
        }
    }

    ImVec2 p_min = cursor;
    ImVec2 p_max = ImVec2(cursor.x + size.x, cursor.y + size.y);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Draw the cropped image with rounding
    draw_list->AddImageRounded(tex_id, p_min, p_max, uv0, uv1, IM_COL32_WHITE, rounding, ImDrawFlags_RoundCornersAll);

    // Draw the border
    draw_list->AddRect(p_min, p_max, border_col, rounding, ImDrawFlags_RoundCornersAll, border_thickness);

    // Reserve space for interaction
    ImGui::InvisibleButton("##image", size);
}

void Editor::Properties::dragDropResources(ComponentType cpType, std::string id, Scene* scene, std::vector<Entity> entities, int updateFlags){
    if (ImGui::BeginDragDropTarget()){

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource_files", ImGuiDragDropFlags_AcceptBeforeDelivery)) {
            std::vector<std::string> receivedStrings = Editor::Util::getStringsFromPayload(payload);
            if (receivedStrings.size() > 0){
                if (!hasTextureDrag.count(id)){
                    hasTextureDrag[id] = true;
                    for (Entity& entity : entities){
                        Texture* valueRef = Catalog::getPropertyRef<Texture>(scene, entity, cpType, id);
                        originalTex[id][entity] = Texture(*valueRef);
                        if (*valueRef != Texture(receivedStrings[0])){
                            *valueRef = Texture(receivedStrings[0]);
                            if (updateFlags & UpdateFlags_Mesh_Texture){
                                unsigned int numSubmeshes = scene->getComponent<MeshComponent>(entity).numSubmeshes;
                                for (unsigned int i = 0; i < numSubmeshes; i++){
                                    scene->getComponent<MeshComponent>(entity).submeshes[i].needUpdateTexture = true;
                                }
                            }
                            if (updateFlags & UpdateFlags_UI_Texture){
                                scene->getComponent<UIComponent>(entity).needUpdateTexture = true;
                            }
                            //printf("needUpdateTexture %s\n", name.c_str());
                        }
                    }
                }
                if (payload->IsDelivery()){
                    Texture texture(receivedStrings[0]);
                    for (Entity& entity : entities){
                        Texture* valueRef = Catalog::getPropertyRef<Texture>(scene, entity, cpType, id);
                        *valueRef = originalTex[id][entity];
                        cmd = new PropertyCmd<Texture>(scene, entity, cpType, id, updateFlags, texture);
                        cmd->setNoMerge();
                        CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
                    }

                    ImGui::SetWindowFocus();
                    hasTextureDrag.erase(id);
                    originalTex.erase(id);
                }
            }
        }
        ImGui::EndDragDropTarget();
    }else{
        if (hasTextureDrag.count(id) && hasTextureDrag[id]){
            for (Entity& entity : entities){
                Texture* valueRef = Catalog::getPropertyRef<Texture>(scene, entity, cpType, id);
                if (*valueRef != originalTex[id][entity]){
                    *valueRef = originalTex[id][entity];
                    if (updateFlags & UpdateFlags_Mesh_Texture){
                        unsigned int numSubmeshes = scene->getComponent<MeshComponent>(entity).numSubmeshes;
                        for (unsigned int i = 0; i < numSubmeshes; i++){
                            scene->getComponent<MeshComponent>(entity).submeshes[i].needUpdateTexture = true;
                        }
                    }
                    if (updateFlags & UpdateFlags_UI_Texture){
                        scene->getComponent<UIComponent>(entity).needUpdateTexture = true;
                    }
                    //printf("needUpdateTexture %s\n", id.c_str());
                }
            }

            hasTextureDrag.erase(id);
            originalTex.erase(id);
        }
    }
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

void Editor::Properties::propertyRow(ComponentType cpType, std::map<std::string, PropertyData> props, std::string id, std::string label, Scene* scene, std::vector<Entity> entities, float stepSize, float secondColSize, bool child, std::string help){
    PropertyData prop = props[replaceNumberedBrackets(id)];

    constexpr float compThreshold = 1e-4;
    constexpr float zeroThreshold = 1e-4;

    if (prop.type == PropertyType::Vector3){
        Vector3* value = nullptr;
        bool difX = false;
        bool difY = false;
        bool difZ = false;
        std::map<Entity, Vector3> eValue;
        for (Entity& entity : entities){
            eValue[entity] = *Catalog::getPropertyRef<Vector3>(scene, entity, cpType, id);
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

        bool defChanged = false;
        if (prop.def){
            defChanged = compareVectorFloat((float*)&newValue, static_cast<float*>(prop.def), 3, compThreshold);
        }
        if (propertyHeader(label, secondColSize, defChanged, child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector3>(scene, entity, cpType, id, prop.updateFlags, *static_cast<Vector3*>(prop.def));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }

        ImGui::BeginGroup();
        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());

        if (difX)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_x_"+id).c_str(), &(newValue.x), stepSize, 0.0f, 0.0f, "%.2f")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector3>(scene, entity, cpType, id, prop.updateFlags, Vector3(newValue.x, eValue[entity].y, eValue[entity].z));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (difX)
            ImGui::PopStyleColor();

        ImGui::SameLine();
        if (difY)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_y_"+id).c_str(), &(newValue.y), stepSize, 0.0f, 0.0f, "%.2f")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector3>(scene, entity, cpType, id, prop.updateFlags, Vector3(eValue[entity].x, newValue.y, eValue[entity].z));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (difY)
            ImGui::PopStyleColor();

        ImGui::SameLine();
        if (difZ)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_z_"+id).c_str(), &(newValue.z), stepSize, 0.0f, 0.0f, "%.2f")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector3>(scene, entity, cpType, id, prop.updateFlags, Vector3(eValue[entity].x, eValue[entity].y, newValue.z));
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
            eValue[entity] = *Catalog::getPropertyRef<Vector4>(scene, entity, cpType, id);
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

        bool defChanged = false;
        if (prop.def){
            defChanged = compareVectorFloat((float*)&newValue, static_cast<float*>(prop.def), 4, compThreshold);
        }
        if (propertyHeader(label, secondColSize, defChanged, child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(scene, entity, cpType, id, prop.updateFlags, *static_cast<Vector4*>(prop.def));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }

        ImGui::BeginGroup();
        ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth());

        if (difX)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_x_"+id).c_str(), &(newValue.x), stepSize, 0.0f, 0.0f, "%.2f")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(scene, entity, cpType, id, prop.updateFlags, Vector4(newValue.x, eValue[entity].y, eValue[entity].z, eValue[entity].w));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (difX)
            ImGui::PopStyleColor();

        ImGui::SameLine();
        if (difY)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_y_"+id).c_str(), &(newValue.y), stepSize, 0.0f, 0.0f, "%.2f")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(scene, entity, cpType, id, prop.updateFlags, Vector4(eValue[entity].x, newValue.y, eValue[entity].z, eValue[entity].w));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (difY)
            ImGui::PopStyleColor();

        ImGui::SameLine();
        if (difZ)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_z_"+id).c_str(), &(newValue.z), stepSize, 0.0f, 0.0f, "%.2f")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(scene, entity, cpType, id, prop.updateFlags, Vector4(eValue[entity].x, eValue[entity].y, newValue.z, eValue[entity].w));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (difZ)
            ImGui::PopStyleColor();

        ImGui::SameLine();
        if (difW)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_w_"+id).c_str(), &(newValue.w), stepSize, 0.0f, 0.0f, "%.2f")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(scene, entity, cpType, id, prop.updateFlags, Vector4(eValue[entity].x, eValue[entity].y, eValue[entity].z, newValue.w));
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
            qValue = *Catalog::getPropertyRef<Quaternion>(scene, entity, cpType, id);
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
        bool defChanged = false;
        if (prop.def){
            defChanged = compareVectorFloat((float*)&qValue, static_cast<float*>(prop.def), 4, compThreshold);
        }
        if (propertyHeader(label, secondColSize, defChanged, child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Quaternion>(scene, entity, cpType, id, prop.updateFlags, *static_cast<Quaternion*>(prop.def));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }

        ImGui::BeginGroup();
        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());

        if (difX)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_x_"+id).c_str(), &(newValue.x), stepSize, 0.0f, 0.0f, "%.2f°")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Quaternion>(scene, entity, cpType, id, prop.updateFlags, Quaternion(newValue.x, eValue[entity].y, eValue[entity].z, order));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (difX)
            ImGui::PopStyleColor();

        ImGui::SameLine();
        if (difY)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_y_"+id).c_str(), &(newValue.y), stepSize, 0.0f, 0.0f, "%.2f°")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Quaternion>(scene, entity, cpType, id, prop.updateFlags, Quaternion(eValue[entity].x, newValue.y, eValue[entity].z, order));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (difY)
            ImGui::PopStyleColor();

        ImGui::SameLine();
        if (difZ)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_z_"+id).c_str(), &(newValue.z), stepSize, 0.0f, 0.0f, "%.2f°")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Quaternion>(scene, entity, cpType, id, prop.updateFlags, Quaternion(eValue[entity].x, eValue[entity].y, newValue.z, order));
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
            eValue[entity] = *Catalog::getPropertyRef<bool>(scene, entity, cpType, id);
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }

        bool newValue = *value;

        bool defChanged = false;
        if (prop.def){
            defChanged = (newValue != *static_cast<bool*>(prop.def));
        }
        if (propertyHeader(label, secondColSize, defChanged, child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<bool>(scene, entity, cpType, id, prop.updateFlags, *static_cast<bool*>(prop.def));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_CheckMark, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::Checkbox(("##checkbox_"+id).c_str(), &newValue)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<bool>(scene, entity, cpType, id, prop.updateFlags, newValue);
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (dif)
            ImGui::PopStyleColor();
        //ImGui::SetItemTooltip("%s", prop.label.c_str());

    }else if (prop.type == PropertyType::Float || prop.type == PropertyType::Float_0_1){
        float* value = nullptr;
        std::map<Entity, float> eValue;
        bool dif = false;
        for (Entity& entity : entities){
            eValue[entity] = *Catalog::getPropertyRef<float>(scene, entity, cpType, id);
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }

        float newValue = *value;

        bool defChanged = false;
        if (prop.def){
            defChanged = (newValue != *static_cast<float*>(prop.def));
        }
        if (propertyHeader(label, secondColSize, defChanged, child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<float>(scene, entity, cpType, id, prop.updateFlags, *static_cast<float*>(prop.def));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }

        float v_min = (0.0F);
        float v_max = (0.0F);
        if (prop.type == PropertyType::Float_0_1){
            v_min = 0.0F;
            v_max = 1.0F;
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_float_"+id).c_str(), &newValue, stepSize, v_min, v_max, "%.2f")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<float>(scene, entity, cpType, id, prop.updateFlags, newValue);
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (dif)
            ImGui::PopStyleColor();
        //ImGui::SetItemTooltip("%s", prop.label.c_str());

        if (!help.empty()){
            ImGui::SameLine(); helpMarker(help);
        }

    }else if (prop.type == PropertyType::UInt){
        unsigned int* value = nullptr;
        std::map<Entity, unsigned int> eValue;
        bool dif = false;
        for (Entity& entity : entities){
            eValue[entity] = *Catalog::getPropertyRef<unsigned int>(scene, entity, cpType, id);
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }

        unsigned int newValue = *value;

        bool defChanged = false;
        if (prop.def){
            defChanged = (newValue != *static_cast<unsigned int*>(prop.def));
        }
        if (propertyHeader(label, secondColSize, defChanged, child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<unsigned int>(scene, entity, cpType, id, prop.updateFlags, *static_cast<unsigned int*>(prop.def));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragInt(("##input_uint_"+id).c_str(), (int*)&newValue, static_cast<unsigned int>(stepSize), 0.0f, INT_MAX)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<unsigned int>(scene, entity, cpType, id, prop.updateFlags, newValue);
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (dif)
            ImGui::PopStyleColor();
        //ImGui::SetItemTooltip("%s", prop.label.c_str());

    }else if (prop.type == PropertyType::Int){
        int* value = nullptr;
        std::map<Entity, int> eValue;
        bool dif = false;
        for (Entity& entity : entities){
            eValue[entity] = *Catalog::getPropertyRef<int>(scene, entity, cpType, id);
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }

        int newValue = *value;

        bool defChanged = false;
        if (prop.def){
            defChanged = (newValue != *static_cast<int*>(prop.def));
        }
        if (propertyHeader(label, secondColSize, defChanged, child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<int>(scene, entity, cpType, id, prop.updateFlags, *static_cast<int*>(prop.def));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragInt(("##input_int_"+id).c_str(), &newValue, static_cast<int>(stepSize), 0.0f, 0.0f)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<int>(scene, entity, cpType, id, prop.updateFlags, newValue);
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
            eValue[entity] = *Catalog::getPropertyRef<Vector3>(scene, entity, cpType, id);
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }

        Vector3 newValue = Color::linearTosRGB(*value);

        // using 'value' beacause it is linear too
        bool defChanged = false;
        if (prop.def){
            defChanged = compareVectorFloat((float*)value, static_cast<float*>(prop.def), 3, compThreshold);
        }
        if (propertyHeader(label, secondColSize, defChanged, child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector3>(scene, entity, cpType, id, prop.updateFlags, *static_cast<Vector3*>(prop.def));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::ColorEdit3((label+"##checkbox_"+id).c_str(), (float*)&newValue.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector3>(scene, entity, cpType, id, prop.updateFlags, Color::sRGBToLinear(newValue));
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
            eValue[entity] = *Catalog::getPropertyRef<Vector4>(scene, entity, cpType, id);
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }

        Vector4 newValue = Color::linearTosRGB(*value);

        // using 'value' beacause it is linear too
        bool defChanged = false;
        if (prop.def){
            defChanged = compareVectorFloat((float*)value, static_cast<float*>(prop.def), 4, compThreshold);
        }
        if (propertyHeader(label, secondColSize, defChanged, child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(scene, entity, cpType, id, prop.updateFlags, *static_cast<Vector4*>(prop.def));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::ColorEdit4((label+"##checkbox_"+id).c_str(), (float*)&newValue.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(scene, entity, cpType, id, prop.updateFlags, Color::sRGBToLinear(newValue));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }
        if (dif)
            ImGui::PopStyleColor();
        //ImGui::SetItemTooltip("%s", prop.label.c_str());

    }else if (prop.type == PropertyType::Enum && prop.enumEntries) {
        int* value = nullptr;
        std::map<Entity, int> eValue;
        bool dif = false;
        for (Entity& entity : entities){
            eValue[entity] = *static_cast<int*>(Catalog::getPropertyRef<int>(scene, entity, cpType, id));
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }

        int item_current = 0;
        // Find current index in enumEntries
        for (size_t i = 0; i < prop.enumEntries->size(); ++i) {
            if ((*prop.enumEntries)[i].value == *value) {
                item_current = static_cast<int>(i);
                break;
            }
        }
        int item_default = item_current;

        bool defChanged = false;
        if (prop.def){
            int defValue = *static_cast<int*>(prop.def);
            // Find index of default value in enumEntries
            for (size_t i = 0; i < prop.enumEntries->size(); ++i) {
                if ((*prop.enumEntries)[i].value == defValue) {
                    item_default = static_cast<int>(i);
                    break;
                }
            }
            defChanged = (item_current != item_default);
        }
        if (propertyHeader(label, secondColSize, defChanged, child)){
            for (Entity& entity : entities){
                int defValue = (*prop.enumEntries)[item_default].value;
                cmd = new PropertyCmd<int>(scene, entity, cpType, id, prop.updateFlags, defValue);
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }

        // Build names array
        std::vector<const char*> names;
        for (const auto& entry : *prop.enumEntries) {
            names.push_back(entry.name);
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::Combo(("##combo_" + id).c_str(), &item_current, names.data(), static_cast<int>(names.size()))) {
            int newValue = (*prop.enumEntries)[item_current].value;
            for (Entity& entity : entities){
                cmd = new PropertyCmd<int>(scene, entity, cpType, id, prop.updateFlags, newValue);
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
            eValue[entity] = *Catalog::getPropertyRef<Texture>(scene, entity, cpType, id);
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }

        Texture newValue = *value;

        bool defChanged = false;
        if (prop.def){
            defChanged = (newValue != *static_cast<Texture*>(prop.def));
        }
        if (propertyHeader(label, secondColSize, defChanged, child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Texture>(scene, entity, cpType, id, prop.updateFlags, *static_cast<Texture*>(prop.def));
                CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
            }
        }

        ImGui::PushID(("texture_"+id).c_str());
        ImGui::PushStyleColor(ImGuiCol_ChildBg, textureLabel);

        ImGui::BeginGroup();

        float thumbSize = 64;
        Texture* thumbTexture = findThumbnail(newValue.getId());
        if (thumbTexture) {
            ImU32 border_col = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);
            if (dif){
                border_col = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
            }
            drawImageWithBorderAndRounding(thumbTexture, ImVec2(thumbSize, thumbSize), ImGui::GetStyle().FrameRounding, border_col);
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Image(thumbTexture->getRender()->getGLHandler(), ImVec2(thumbTexture->getWidth(), thumbTexture->getHeight()));
                ImGui::EndTooltip();
            }
        }

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
        if (!newValue.getId().empty()){
            ImGui::SetItemTooltip("%s", newValue.getId().c_str());
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();

        if (ImGui::Button(ICON_FA_FILE_IMPORT)) {
            std::string path = Editor::FileDialogs::openFileDialog(project->getProjectPath().string(), true);
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
                        cmd = new PropertyCmd<Texture>(scene, entity, cpType, id, prop.updateFlags, texture);
                        cmd->setNoMerge();
                        CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);
                    }
                }
            }
        }

        ImGui::EndGroup();

        dragDropResources(cpType, id, scene, entities, prop.updateFlags);

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
    // Add this code to calculate appropriate step size based on selected scene
    float stepSize = 0.1f;
    SceneProject* sceneProject = project->getSelectedScene();
    if (sceneProject) {
        Camera* camera = sceneProject->sceneRender->getCamera();
        if (sceneProject->sceneType == SceneType::SCENE_3D) {
            // For 3D scenes, scale step based on distance from target
            float distanceFromTarget = camera->getDistanceFromTarget();
            stepSize = std::max(0.01f, distanceFromTarget / 200.0f);
        } else {
            // For 2D scenes, use the zoom level
            SceneRender2D* sceneRender2D = static_cast<SceneRender2D*>(sceneProject->sceneRender);
            float zoom = sceneRender2D->getZoom();
            stepSize = std::max(0.01f, zoom * 1.0f);
        }
    }

    beginTable(cpType, getLabelSize("billboard"));

    propertyRow(cpType, props, "position", "Position", scene, entities, stepSize);
    propertyRow(cpType, props, "rotation", "Rotation", scene, entities);
    propertyRow(cpType, props, "scale", "Scale", scene, entities);
    propertyRow(cpType, props, "visible", "Visible", scene, entities);
    propertyRow(cpType, props, "billboard", "Billboard", scene, entities);

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_GEAR)){
        ImGui::OpenPopup("menusettings_billboard");
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(19 * ImGui::GetFontSize(), 0), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::BeginPopup("menusettings_billboard")){
        ImGui::Text("Billboard settings");
        ImGui::Separator();

        beginTable(cpType, getLabelSize("cylindrical"));

        propertyRow(cpType, props, "fake_billboard", "Fake", scene, entities);
        propertyRow(cpType, props, "cylindrical_billboard", "Cylindrical", scene, entities);
        propertyRow(cpType, props, "rotation_billboard", "Rotation", scene, entities, 0.1f, 12 * ImGui::GetFontSize());

        endTable();

        ImGui::EndPopup();
    }

    endTable();
}

void Editor::Properties::drawMeshComponent(ComponentType cpType, std::map<std::string, PropertyData> props, Scene* scene, std::vector<Entity> entities){
    beginTable(cpType, getLabelSize("receive shadows"));

    propertyRow(cpType, props, "cast_shadows", "Cast Shadows", scene, entities);
    propertyRow(cpType, props, "receive_shadows", "Receive Shadows", scene, entities);

    endTable();

    unsigned int numSubmeshes = 1;
    for (Entity& entity : entities){
        numSubmeshes = std::min(numSubmeshes, scene->getComponent<MeshComponent>(entity).numSubmeshes);
    }

    for (int s = 0; s < numSubmeshes; s++){
        ImGui::SeparatorText(("Submesh "+std::to_string(s+1)).c_str());

        float submeshesTableSize = getLabelSize("Texture Rect");

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
            beginTable(cpType, getLabelSize("Met. Roug. Texture"), "material_table");
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.basecolor", "Base Color", scene, entities, 0.1f, -1, true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.basecolortexture", "Base Texture", scene, entities, 0.1f, -1, true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.metallicfactor", "Metallic Factor", scene, entities, 0.01f, 4 * ImGui::GetFontSize(), true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.roughnessfactor", "Roughness Factor", scene, entities, 0.01f, 4 * ImGui::GetFontSize(), true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.metallicroughnesstexture", "Met. Roug. Texture", scene, entities, 0.1f, -1, true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.emissivefactor", "Emissive Factor", scene, entities, 0.1f, -1, true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.emissivetexture", "Emissive Texture", scene, entities, 0.1f, -1, true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.occlusiontexture", "Occlusion Texture", scene, entities, 0.1f, -1, true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.normalTexture", "Normal Texture", scene, entities, 0.1f, -1, true);
            if (!scene->isSceneAmbientLightEnabled()){
                propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.ambientlight", "Ambient Light", scene, entities, 0.1f, -1, true);
                propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.ambientintensity", "Ambient Intensity", scene, entities, 0.1f, 4 * ImGui::GetFontSize(), true);
            }
            endTable();
            beginTable(cpType, submeshesTableSize, "submeshes");
        }

        propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].primitive_type", "Primitive", scene, entities);
        propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].face_culling", "Face Culling", scene, entities);
        propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].texture_rect", "Texture Rect", scene, entities);

        endTable();
    }
}

void Editor::Properties::drawUIComponent(ComponentType cpType, std::map<std::string, PropertyData> props, Scene* scene, std::vector<Entity> entities){
    beginTable(cpType, getLabelSize("Texture"));

    propertyRow(cpType, props, "color", "Color", scene, entities);
    propertyRow(cpType, props, "texture", "Texture", scene, entities);

    endTable();
}

void Editor::Properties::drawUILayoutComponent(ComponentType cpType, std::map<std::string, PropertyData> props, Scene* scene, std::vector<Entity> entities){
    beginTable(cpType, getLabelSize("Ignore Scissor"));

    propertyRow(cpType, props, "width", "Width", scene, entities, 1.0, 6 * ImGui::GetFontSize());
    propertyRow(cpType, props, "height", "Height", scene, entities, 1.0, 6 * ImGui::GetFontSize());
    propertyRow(cpType, props, "ignore_scissor", "Ignore Scissor", scene, entities);

    endTable();
}

void Editor::Properties::drawImageComponent(ComponentType cpType, std::map<std::string, PropertyData> props, Scene* scene, std::vector<Entity> entities){
    ImGui::SeparatorText("Nine-patch rect");
    beginTable(cpType, getLabelSize("Margin Bottom"), "nine_margin_table");

    propertyRow(cpType, props, "patch_margin_left", "Margin Left", scene, entities, 1.0, 6 * ImGui::GetFontSize());
    propertyRow(cpType, props, "patch_margin_right", "Margin Right", scene, entities, 1.0, 6 * ImGui::GetFontSize());
    propertyRow(cpType, props, "patch_margin_top", "Margin Top", scene, entities, 1.0, 6 * ImGui::GetFontSize());
    propertyRow(cpType, props, "patch_margin_bottom", "Margin Bottom", scene, entities, 1.0, 6 * ImGui::GetFontSize());

    endTable();

    beginTable(cpType, getLabelSize("Cut Factor"));

    propertyRow(cpType, props, "texture_cut_factor", "Cut Factor", scene, entities, 0.1f, 6 * ImGui::GetFontSize(), false, "Increase or decrease texture area");

    endTable();
}

void Editor::Properties::drawSpriteComponent(ComponentType cpType, std::map<std::string, PropertyData> props, Scene* scene, std::vector<Entity> entities){
    beginTable(cpType, getLabelSize("Cut Factor"));

    propertyRow(cpType, props, "width", "Width", scene, entities, 1.0, 6 * ImGui::GetFontSize());
    propertyRow(cpType, props, "height", "Height", scene, entities, 1.0, 6 * ImGui::GetFontSize());
    propertyRow(cpType, props, "pivot_preset", "Pivot", scene, entities);
    propertyRow(cpType, props, "texture_cut_factor", "Cut Factor", scene, entities, 0.1f, 6 * ImGui::GetFontSize(), false, "Increase or decrease texture area");

    endTable();
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

        for (ComponentType& cpType : components){

            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::CollapsingHeader(Catalog::getComponentName(cpType).c_str())){

                if (cpType == ComponentType::Transform){
                    drawTransform(cpType, Catalog::getProperties(cpType, nullptr), scene, entities);
                }else if (cpType == ComponentType::MeshComponent){
                    drawMeshComponent(cpType, Catalog::getProperties(cpType, nullptr), scene, entities);
                }else if (cpType == ComponentType::UIComponent){
                    drawUIComponent(cpType, Catalog::getProperties(cpType, nullptr), scene, entities);
                }else if (cpType == ComponentType::UILayoutComponent){
                    drawUILayoutComponent(cpType, Catalog::getProperties(cpType, nullptr), scene, entities);
                }else if (cpType == ComponentType::ImageComponent){
                    drawImageComponent(cpType, Catalog::getProperties(cpType, nullptr), scene, entities);
                }else if (cpType == ComponentType::SpriteComponent){
                    drawSpriteComponent(cpType, Catalog::getProperties(cpType, nullptr), scene, entities);
                }

            }
        }

    }else{
        thumbnailTextures.clear();
    }

    ImGui::End();
}