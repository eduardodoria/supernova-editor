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
#include "Stream.h"

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
            } else if (!thumbIt->second.empty()) {
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

void Editor::Properties::dragDropResources(ComponentType cpType, std::string id, SceneProject* sceneProject, std::vector<Entity> entities, int updateFlags){
    if (ImGui::BeginDragDropTarget()){

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource_files", ImGuiDragDropFlags_AcceptBeforeDelivery)) {
            std::vector<std::string> receivedStrings = Editor::Util::getStringsFromPayload(payload);
            if (receivedStrings.size() > 0){
                if (!hasTextureDrag.count(id)){
                    hasTextureDrag[id] = true;
                    for (Entity& entity : entities){
                        Texture* valueRef = Catalog::getPropertyRef<Texture>(sceneProject->scene, entity, cpType, id);
                        originalTex[id][entity] = Texture(*valueRef);
                        if (*valueRef != Texture(receivedStrings[0])){
                            *valueRef = Texture(receivedStrings[0]);
                            if (updateFlags & UpdateFlags_Mesh_Texture){
                                unsigned int numSubmeshes = sceneProject->scene->getComponent<MeshComponent>(entity).numSubmeshes;
                                for (unsigned int i = 0; i < numSubmeshes; i++){
                                    sceneProject->scene->getComponent<MeshComponent>(entity).submeshes[i].needUpdateTexture = true;
                                }
                            }
                            if (updateFlags & UpdateFlags_UI_Texture){
                                sceneProject->scene->getComponent<UIComponent>(entity).needUpdateTexture = true;
                            }
                            //printf("needUpdateTexture %s\n", name.c_str());
                        }
                    }
                }
                if (payload->IsDelivery()){
                    Texture texture(receivedStrings[0]);
                    for (Entity& entity : entities){
                        Texture* valueRef = Catalog::getPropertyRef<Texture>(sceneProject->scene, entity, cpType, id);
                        *valueRef = originalTex[id][entity];
                        cmd = new PropertyCmd<Texture>(sceneProject, entity, cpType, id, updateFlags, texture);
                        cmd->setNoMerge();
                        CommandHandle::get(sceneProject->id)->addCommand(cmd);
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
                Texture* valueRef = Catalog::getPropertyRef<Texture>(sceneProject->scene, entity, cpType, id);
                if (*valueRef != originalTex[id][entity]){
                    *valueRef = originalTex[id][entity];
                    if (updateFlags & UpdateFlags_Mesh_Texture){
                        unsigned int numSubmeshes = sceneProject->scene->getComponent<MeshComponent>(entity).numSubmeshes;
                        for (unsigned int i = 0; i < numSubmeshes; i++){
                            sceneProject->scene->getComponent<MeshComponent>(entity).submeshes[i].needUpdateTexture = true;
                        }
                    }
                    if (updateFlags & UpdateFlags_UI_Texture){
                        sceneProject->scene->getComponent<UIComponent>(entity).needUpdateTexture = true;
                    }
                    //printf("needUpdateTexture %s\n", id.c_str());
                }
            }

            hasTextureDrag.erase(id);
            originalTex.erase(id);
        }
    }
}

Texture Editor::Properties::getMaterialThumbnail(const Material& material){
    if ((materialRender.getMaterial() != material) || !materialRender.getFramebuffer()->isCreated()){
        materialRender.applyMaterial(material);
        Engine::executeSceneOnce(materialRender.getScene());
    }

    return materialRender.getTexture();
}

void Editor::Properties::drawNinePatchesPreview(const ImageComponent& img, Texture* texture, Texture* thumbTexture, const ImVec2& size){
    float availWidth = ImGui::GetContentRegionAvail().x;

    // Calculate display size based on input parameter or default to texture size
    float displayWidth = (size.x > 0) ? size.x : thumbTexture->getWidth();
    float displayHeight = (size.y > 0) ? size.y : thumbTexture->getHeight();

    // If only one dimension is specified, maintain aspect ratio
    if (size.x > 0 && size.y <= 0) {
        float aspectRatio = (float)thumbTexture->getHeight() / thumbTexture->getWidth();
        displayHeight = displayWidth * aspectRatio;
    } else if (size.x <= 0 && size.y > 0) {
        float aspectRatio = (float)thumbTexture->getWidth() / thumbTexture->getHeight();
        displayWidth = displayHeight * aspectRatio;
    }

    // Calculate position to center the image
    float xPos = (availWidth - displayWidth) * 0.5f;
    // Set cursor position to create centering effect
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + xPos);

    // Store the cursor position before drawing the image
    ImVec2 cursor = ImGui::GetCursorScreenPos();

    // Draw the image with the calculated size
    ImGui::Image(thumbTexture->getRender()->getGLHandler(), ImVec2(displayWidth, displayHeight));

    // Get draw list for custom rendering
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Define line color
    ImU32 line_color = IM_COL32(255, 0, 0, 255); // Red color

    // Get the original texture and thumbnail dimensions
    float origWidth = static_cast<float>(texture->getWidth());
    float origHeight = static_cast<float>(texture->getHeight());
    float thumbWidth = static_cast<float>(thumbTexture->getWidth());
    float thumbHeight = static_cast<float>(thumbTexture->getHeight());

    // Calculate scale factors between original texture and display size
    float scaleX = displayWidth / origWidth;
    float scaleY = displayHeight / origHeight;

    // Scale the margin values by the appropriate scale factor
    float scaledLeftMargin = img.patchMarginLeft * scaleX;
    float scaledRightMargin = img.patchMarginRight * scaleX;
    float scaledTopMargin = img.patchMarginTop * scaleY;
    float scaledBottomMargin = img.patchMarginBottom * scaleY;

    // Draw left margin line
    float leftX = cursor.x + scaledLeftMargin;
    draw_list->AddLine(
        ImVec2(leftX, cursor.y),
        ImVec2(leftX, cursor.y + displayHeight),
        line_color, 1.0f
    );

    // Draw right margin line
    float rightX = cursor.x + displayWidth - scaledRightMargin;
    draw_list->AddLine(
        ImVec2(rightX, cursor.y),
        ImVec2(rightX, cursor.y + displayHeight),
        line_color, 1.0f
    );

    // Draw top margin line
    float topY = cursor.y + scaledTopMargin;
    draw_list->AddLine(
        ImVec2(cursor.x, topY),
        ImVec2(cursor.x + displayWidth, topY),
        line_color, 1.0f
    );

    // Draw bottom margin line
    float bottomY = cursor.y + displayHeight - scaledBottomMargin;
    draw_list->AddLine(
        ImVec2(cursor.x, bottomY),
        ImVec2(cursor.x + displayWidth, bottomY),
        line_color, 1.0f
    );
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

bool Editor::Properties::propertyRow(ComponentType cpType, std::map<std::string, PropertyData> props, std::string id, std::string label, SceneProject* sceneProject, std::vector<Entity> entities, float stepSize, float secondColSize, bool child, std::string help){
    PropertyData prop = props[replaceNumberedBrackets(id)];

    bool result = true;

    constexpr float compThreshold = 1e-4;
    constexpr float zeroThreshold = 1e-4;

    if (prop.type == PropertyType::Vector3){
        Vector3* value = nullptr;
        bool difX = false;
        bool difY = false;
        bool difZ = false;
        std::map<Entity, Vector3> eValue;
        for (Entity& entity : entities){
            eValue[entity] = *Catalog::getPropertyRef<Vector3>(sceneProject->scene, entity, cpType, id);
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
                cmd = new PropertyCmd<Vector3>(sceneProject, entity, cpType, id, prop.updateFlags, *static_cast<Vector3*>(prop.def));
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }

        ImGui::BeginGroup();
        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());

        if (difX)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_x_"+id).c_str(), &(newValue.x), stepSize, 0.0f, 0.0f, "%.2f")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector3>(sceneProject, entity, cpType, id, prop.updateFlags, Vector3(newValue.x, eValue[entity].y, eValue[entity].z));
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }
        if (difX)
            ImGui::PopStyleColor();

        ImGui::SameLine();
        if (difY)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_y_"+id).c_str(), &(newValue.y), stepSize, 0.0f, 0.0f, "%.2f")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector3>(sceneProject, entity, cpType, id, prop.updateFlags, Vector3(eValue[entity].x, newValue.y, eValue[entity].z));
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }
        if (difY)
            ImGui::PopStyleColor();

        ImGui::SameLine();
        if (difZ)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_z_"+id).c_str(), &(newValue.z), stepSize, 0.0f, 0.0f, "%.2f")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector3>(sceneProject, entity, cpType, id, prop.updateFlags, Vector3(eValue[entity].x, eValue[entity].y, newValue.z));
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
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
            eValue[entity] = *Catalog::getPropertyRef<Vector4>(sceneProject->scene, entity, cpType, id);
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
                cmd = new PropertyCmd<Vector4>(sceneProject, entity, cpType, id, prop.updateFlags, *static_cast<Vector4*>(prop.def));
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }

        ImGui::BeginGroup();
        ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth());

        if (difX)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_x_"+id).c_str(), &(newValue.x), stepSize, 0.0f, 0.0f, "%.2f")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(sceneProject, entity, cpType, id, prop.updateFlags, Vector4(newValue.x, eValue[entity].y, eValue[entity].z, eValue[entity].w));
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }
        if (difX)
            ImGui::PopStyleColor();

        ImGui::SameLine();
        if (difY)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_y_"+id).c_str(), &(newValue.y), stepSize, 0.0f, 0.0f, "%.2f")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(sceneProject, entity, cpType, id, prop.updateFlags, Vector4(eValue[entity].x, newValue.y, eValue[entity].z, eValue[entity].w));
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }
        if (difY)
            ImGui::PopStyleColor();

        ImGui::SameLine();
        if (difZ)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_z_"+id).c_str(), &(newValue.z), stepSize, 0.0f, 0.0f, "%.2f")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(sceneProject, entity, cpType, id, prop.updateFlags, Vector4(eValue[entity].x, eValue[entity].y, newValue.z, eValue[entity].w));
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }
        if (difZ)
            ImGui::PopStyleColor();

        ImGui::SameLine();
        if (difW)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_w_"+id).c_str(), &(newValue.w), stepSize, 0.0f, 0.0f, "%.2f")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(sceneProject, entity, cpType, id, prop.updateFlags, Vector4(eValue[entity].x, eValue[entity].y, eValue[entity].z, newValue.w));
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
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
            qValue = *Catalog::getPropertyRef<Quaternion>(sceneProject->scene, entity, cpType, id);
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
                cmd = new PropertyCmd<Quaternion>(sceneProject, entity, cpType, id, prop.updateFlags, *static_cast<Quaternion*>(prop.def));
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }

        ImGui::BeginGroup();
        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());

        if (difX)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_x_"+id).c_str(), &(newValue.x), stepSize, 0.0f, 0.0f, "%.2f°")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Quaternion>(sceneProject, entity, cpType, id, prop.updateFlags, Quaternion(newValue.x, eValue[entity].y, eValue[entity].z, order));
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }
        if (difX)
            ImGui::PopStyleColor();

        ImGui::SameLine();
        if (difY)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_y_"+id).c_str(), &(newValue.y), stepSize, 0.0f, 0.0f, "%.2f°")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Quaternion>(sceneProject, entity, cpType, id, prop.updateFlags, Quaternion(eValue[entity].x, newValue.y, eValue[entity].z, order));
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }
        if (difY)
            ImGui::PopStyleColor();

        ImGui::SameLine();
        if (difZ)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_z_"+id).c_str(), &(newValue.z), stepSize, 0.0f, 0.0f, "%.2f°")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Quaternion>(sceneProject, entity, cpType, id, prop.updateFlags, Quaternion(eValue[entity].x, eValue[entity].y, newValue.z, order));
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
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
            eValue[entity] = *Catalog::getPropertyRef<bool>(sceneProject->scene, entity, cpType, id);
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
                cmd = new PropertyCmd<bool>(sceneProject, entity, cpType, id, prop.updateFlags, *static_cast<bool*>(prop.def));
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_CheckMark, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::Checkbox(("##checkbox_"+id).c_str(), &newValue)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<bool>(sceneProject, entity, cpType, id, prop.updateFlags, newValue);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
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
            eValue[entity] = *Catalog::getPropertyRef<float>(sceneProject->scene, entity, cpType, id);
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
                cmd = new PropertyCmd<float>(sceneProject, entity, cpType, id, prop.updateFlags, *static_cast<float*>(prop.def));
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
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
                cmd = new PropertyCmd<float>(sceneProject, entity, cpType, id, prop.updateFlags, newValue);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
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
            eValue[entity] = *Catalog::getPropertyRef<unsigned int>(sceneProject->scene, entity, cpType, id);
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
                cmd = new PropertyCmd<unsigned int>(sceneProject, entity, cpType, id, prop.updateFlags, *static_cast<unsigned int*>(prop.def));
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragInt(("##input_uint_"+id).c_str(), (int*)&newValue, static_cast<unsigned int>(stepSize), 0.0f, INT_MAX)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<unsigned int>(sceneProject, entity, cpType, id, prop.updateFlags, newValue);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
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
            eValue[entity] = *Catalog::getPropertyRef<int>(sceneProject->scene, entity, cpType, id);
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
                cmd = new PropertyCmd<int>(sceneProject, entity, cpType, id, prop.updateFlags, *static_cast<int*>(prop.def));
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragInt(("##input_int_"+id).c_str(), &newValue, static_cast<int>(stepSize), 0.0f, 0.0f)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<int>(sceneProject, entity, cpType, id, prop.updateFlags, newValue);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
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
            eValue[entity] = *Catalog::getPropertyRef<Vector3>(sceneProject->scene, entity, cpType, id);
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
                cmd = new PropertyCmd<Vector3>(sceneProject, entity, cpType, id, prop.updateFlags, *static_cast<Vector3*>(prop.def));
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::ColorEdit3((label+"##checkbox_"+id).c_str(), (float*)&newValue.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector3>(sceneProject, entity, cpType, id, prop.updateFlags, Color::sRGBToLinear(newValue));
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
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
            eValue[entity] = *Catalog::getPropertyRef<Vector4>(sceneProject->scene, entity, cpType, id);
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
                cmd = new PropertyCmd<Vector4>(sceneProject, entity, cpType, id, prop.updateFlags, *static_cast<Vector4*>(prop.def));
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::ColorEdit4((label+"##checkbox_"+id).c_str(), (float*)&newValue.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(sceneProject, entity, cpType, id, prop.updateFlags, Color::sRGBToLinear(newValue));
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
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
            eValue[entity] = *static_cast<int*>(Catalog::getPropertyRef<int>(sceneProject->scene, entity, cpType, id));
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
                cmd = new PropertyCmd<int>(sceneProject, entity, cpType, id, prop.updateFlags, defValue);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
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
                cmd = new PropertyCmd<int>(sceneProject, entity, cpType, id, prop.updateFlags, newValue);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }
        if (dif)
            ImGui::PopStyleColor();

    }else if (prop.type == PropertyType::Texture){
        Texture* value = nullptr;
        std::map<Entity, Texture> eValue;
        bool dif = false;
        for (Entity& entity : entities){
            eValue[entity] = *Catalog::getPropertyRef<Texture>(sceneProject->scene, entity, cpType, id);
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
                cmd = new PropertyCmd<Texture>(sceneProject, entity, cpType, id, prop.updateFlags, *static_cast<Texture*>(prop.def));
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }

        ImGui::BeginGroup();
        ImGui::PushID(("texture_"+id).c_str());

        ImGui::PushStyleColor(ImGuiCol_ChildBg, textureLabel);

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
                        cmd = new PropertyCmd<Texture>(sceneProject, entity, cpType, id, prop.updateFlags, texture);
                        cmd->setNoMerge();
                        CommandHandle::get(sceneProject->id)->addCommand(cmd);
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
        ImGui::EndGroup();

        dragDropResources(cpType, id, sceneProject, entities, prop.updateFlags);

    }else if (prop.type == PropertyType::Material){
        Material* value = nullptr;
        std::map<Entity, Material> eValue;
        bool dif = false;
        for (Entity& entity : entities){
            eValue[entity] = *Catalog::getPropertyRef<Material>(sceneProject->scene, entity, cpType, id);
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }
        Material newValue = *value;

        bool defChanged = false;
        if (prop.def){
            defChanged = (newValue != *static_cast<Material*>(prop.def));
        }
        if (propertyHeader(label, secondColSize, defChanged, child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Material>(sceneProject, entity, cpType, id, prop.updateFlags, *static_cast<Material*>(prop.def));
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }

        ImGui::BeginGroup();

        Texture texRender = getMaterialThumbnail(newValue);
        ImGui::Image(texRender.getRender()->getGLHandler(), ImVec2(64, 64));
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            materialButtonGroups[id] = !materialButtonGroups[id];
        }

        ImGui::PushStyleColor(ImGuiCol_ChildBg, textureLabel);

        ImVec2 arrowButtonSize = ImGui::CalcItemSize(ImVec2(0, 0), ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
        ImGui::BeginChild("textureframe", ImVec2( - arrowButtonSize.x - ImGui::GetStyle().ItemSpacing.x, ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2), 
            false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        std::string matName = newValue.name;
        if (std::filesystem::exists(matName)) {
            matName = std::filesystem::path(matName).filename().string();
        }
        if (matName.empty()) {
            matName = "< Not defined >";
        }

        float textWidth = ImGui::CalcTextSize(matName.c_str()).x;
        float availWidth = ImGui::GetContentRegionAvail().x;
        ImGui::SetCursorPosX(availWidth - textWidth - 2);
        ImGui::SetCursorPosY(ImGui::GetStyle().FramePadding.y);
        if (dif)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        ImGui::Text("%s", matName.c_str());
        if (dif)
            ImGui::PopStyleColor();

        ImGui::EndChild();
        if (!newValue.name.empty()){
            ImGui::SetItemTooltip("%s", newValue.name.c_str());
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();

        if (ImGui::ArrowButton("##toggle_mesh", materialButtonGroups[id] ? ImGuiDir_Up : ImGuiDir_Down)){
            materialButtonGroups[id] = !materialButtonGroups[id];
        }

        ImGui::EndGroup();

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            std::string materialStr = YAML::Dump(Stream::encodeMaterial(newValue));

            ImGui::SetDragDropPayload("material", materialStr.c_str(), materialStr.size());
            ImGui::Text("Moving material");
            float imageDragSize = 32;
            float availWidth = ImGui::GetCurrentWindow()->Size.x;
            float xPos = (availWidth - imageDragSize) * 0.5f;
            ImGui::SetCursorPosX(xPos);
            ImGui::Image(texRender.getRender()->getGLHandler(), ImVec2(imageDragSize, imageDragSize));
            ImGui::EndDragDropSource();
        }

        result = materialButtonGroups[id];
    }

    if (ImGui::IsItemDeactivatedAfterEdit()) {
        cmd->setNoMerge();
        cmd = nullptr;
    }

    return result;
}

void Editor::Properties::drawTransform(ComponentType cpType, std::map<std::string, PropertyData> props, SceneProject* sceneProject, std::vector<Entity> entities){
    // Add this code to calculate appropriate step size based on selected scene
    float stepSize = 0.1f;
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

    propertyRow(cpType, props, "position", "Position", sceneProject, entities, stepSize);
    propertyRow(cpType, props, "rotation", "Rotation", sceneProject, entities);
    propertyRow(cpType, props, "scale", "Scale", sceneProject, entities);
    propertyRow(cpType, props, "visible", "Visible", sceneProject, entities);
    propertyRow(cpType, props, "billboard", "Billboard", sceneProject, entities);

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_GEAR)){
        ImGui::OpenPopup("menusettings_billboard");
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(19 * ImGui::GetFontSize(), 0), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::BeginPopup("menusettings_billboard")){
        ImGui::Text("Billboard settings");
        ImGui::Separator();

        beginTable(cpType, getLabelSize("cylindrical"));

        propertyRow(cpType, props, "fake_billboard", "Fake", sceneProject, entities);
        propertyRow(cpType, props, "cylindrical_billboard", "Cylindrical", sceneProject, entities);
        propertyRow(cpType, props, "rotation_billboard", "Rotation", sceneProject, entities, 0.1f, 12 * ImGui::GetFontSize());

        endTable();

        ImGui::EndPopup();
    }

    endTable();
}

void Editor::Properties::drawMeshComponent(ComponentType cpType, std::map<std::string, PropertyData> props, SceneProject* sceneProject, std::vector<Entity> entities){
    beginTable(cpType, getLabelSize("receive shadows"));

    // Add New Geometry property row
    propertyHeader("Geometry");
    if (ImGui::Button("New")){
        ImGui::OpenPopup("menusettings_geometry");
    }

    // Geometry creation popup
    ImGui::SetNextWindowSizeConstraints(ImVec2(20 * ImGui::GetFontSize(), 0), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::BeginPopup("menusettings_geometry")){
        ImGui::Text("Create geometry");
        ImGui::Separator();

        // Static variables for geometry parameters
        static int geometryType = 0;
        static float planeWidth = 1.0f, planeDepth = 1.0f;
        static unsigned int planeTiles = 1;
        static float boxWidth = 1.0f, boxHeight = 1.0f, boxDepth = 1.0f;
        static unsigned int boxTiles = 1;
        static float sphereRadius = 1.0f;
        static unsigned int sphereSlices = 36, sphereStacks = 18;
        static float cylinderBaseRadius = 1.0f, cylinderTopRadius = 1.0f, cylinderHeight = 2.0f;
        static unsigned int cylinderSlices = 36, cylinderStacks = 18;
        static float capsuleBaseRadius = 1.0f, capsuleTopRadius = 1.0f, capsuleHeight = 2.0f;
        static unsigned int capsuleSlices = 36, capsuleStacks = 18;
        static float torusRadius = 1.0f, torusRingRadius = 0.5f;
        static unsigned int torusSides = 36, torusRings = 16;

        const char* geometryTypes[] = { "Plane", "Box", "Sphere", "Cylinder", "Capsule", "Torus" };

        beginTable(cpType, getLabelSize("Geometry Type"), "geometry_popup");

        // Geometry type selection
        propertyHeader("Geometry Type");
        ImGui::Combo("##geometry_type", &geometryType, geometryTypes, IM_ARRAYSIZE(geometryTypes));

        float secondColSize = 11 * ImGui::GetFontSize();
        // Show parameters based on selected geometry type
        switch (geometryType) {
            case 0: // Plane
                propertyHeader("Width", secondColSize);
                ImGui::DragFloat("##plane_width", &planeWidth, 0.1f, 0.1f, 100.0f, "%.2f");

                propertyHeader("Depth", secondColSize);
                ImGui::DragFloat("##plane_depth", &planeDepth, 0.1f, 0.1f, 100.0f, "%.2f");

                propertyHeader("Tiles", secondColSize);
                ImGui::DragInt("##plane_tiles", (int*)&planeTiles, 1, 1, 100);
                break;

            case 1: // Box
                propertyHeader("Width", secondColSize);
                ImGui::DragFloat("##box_width", &boxWidth, 0.1f, 0.1f, 100.0f, "%.2f");

                propertyHeader("Height", secondColSize);
                ImGui::DragFloat("##box_height", &boxHeight, 0.1f, 0.1f, 100.0f, "%.2f");

                propertyHeader("Depth", secondColSize);
                ImGui::DragFloat("##box_depth", &boxDepth, 0.1f, 0.1f, 100.0f, "%.2f");

                propertyHeader("Tiles", secondColSize);
                ImGui::DragInt("##box_tiles", (int*)&boxTiles, 1, 1, 100);
                break;

            case 2: // Sphere
                propertyHeader("Radius", secondColSize);
                ImGui::DragFloat("##sphere_radius", &sphereRadius, 0.1f, 0.1f, 100.0f, "%.2f");

                propertyHeader("Slices", secondColSize);
                ImGui::DragInt("##sphere_slices", (int*)&sphereSlices, 1, 3, 100);

                propertyHeader("Stacks", secondColSize);
                ImGui::DragInt("##sphere_stacks", (int*)&sphereStacks, 1, 3, 100);
                break;

            case 3: // Cylinder
                propertyHeader("Base Radius", secondColSize);
                ImGui::DragFloat("##cylinder_base_radius", &cylinderBaseRadius, 0.1f, 0.1f, 100.0f, "%.2f");

                propertyHeader("Top Radius", secondColSize);
                ImGui::DragFloat("##cylinder_top_radius", &cylinderTopRadius, 0.1f, 0.1f, 100.0f, "%.2f");

                propertyHeader("Height", secondColSize);
                ImGui::DragFloat("##cylinder_height", &cylinderHeight, 0.1f, 0.1f, 100.0f, "%.2f");

                propertyHeader("Slices", secondColSize);
                ImGui::DragInt("##cylinder_slices", (int*)&cylinderSlices, 1, 3, 100);

                propertyHeader("Stacks", secondColSize);
                ImGui::DragInt("##cylinder_stacks", (int*)&cylinderStacks, 1, 1, 100);
                break;

            case 4: // Capsule
                propertyHeader("Base Radius", secondColSize);
                ImGui::DragFloat("##capsule_base_radius", &capsuleBaseRadius, 0.1f, 0.1f, 100.0f, "%.2f");

                propertyHeader("Top Radius", secondColSize);
                ImGui::DragFloat("##capsule_top_radius", &capsuleTopRadius, 0.1f, 0.1f, 100.0f, "%.2f");

                propertyHeader("Height", secondColSize);
                ImGui::DragFloat("##capsule_height", &capsuleHeight, 0.1f, 0.1f, 100.0f, "%.2f");

                propertyHeader("Slices", secondColSize);
                ImGui::DragInt("##capsule_slices", (int*)&capsuleSlices, 1, 3, 100);

                propertyHeader("Stacks", secondColSize);
                ImGui::DragInt("##capsule_stacks", (int*)&capsuleStacks, 1, 1, 100);
                break;

            case 5: // Torus
                propertyHeader("Radius", secondColSize);
                ImGui::DragFloat("##torus_radius", &torusRadius, 0.1f, 0.1f, 100.0f, "%.2f");

                propertyHeader("Ring Radius", secondColSize);
                ImGui::DragFloat("##torus_ring_radius", &torusRingRadius, 0.1f, 0.1f, 100.0f, "%.2f");

                propertyHeader("Sides", secondColSize);
                ImGui::DragInt("##torus_sides", (int*)&torusSides, 1, 3, 100);

                propertyHeader("Rings", secondColSize);
                ImGui::DragInt("##torus_rings", (int*)&torusRings, 1, 3, 100);
                break;
        }

        endTable();

        ImGui::Separator();

        // Create geometry button
        if (ImGui::Button("Apply", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            for (Entity& entity : entities) {
                std::shared_ptr<Supernova::MeshSystem> meshSys = sceneProject->scene->getSystem<MeshSystem>();
                if (meshSys) {
                    switch (geometryType) {
                        case 0: // Plane
                            meshSys->createPlane(entity, planeWidth, planeDepth, planeTiles);
                            break;
                        case 1: // Box
                            meshSys->createBox(entity, boxWidth, boxHeight, boxDepth, boxTiles);
                            break;
                        case 2: // Sphere
                            meshSys->createSphere(entity, sphereRadius, sphereSlices, sphereStacks);
                            break;
                        case 3: // Cylinder
                            meshSys->createCylinder(entity, cylinderBaseRadius, cylinderTopRadius, cylinderHeight, cylinderSlices, cylinderStacks);
                            break;
                        case 4: // Capsule
                            meshSys->createCapsule(entity, capsuleBaseRadius, capsuleTopRadius, capsuleHeight, capsuleSlices, capsuleStacks);
                            break;
                        case 5: // Torus
                            meshSys->createTorus(entity, torusRadius, torusRingRadius, torusSides, torusRings);
                            break;
                    }
                }
            }
        }

        ImGui::EndPopup();
    }

    propertyRow(cpType, props, "cast_shadows", "Cast Shadows", sceneProject, entities);
    propertyRow(cpType, props, "receive_shadows", "Receive Shadows", sceneProject, entities);

    endTable();

    unsigned int numSubmeshes = 1;
    for (Entity& entity : entities){
        numSubmeshes = std::min(numSubmeshes, sceneProject->scene->getComponent<MeshComponent>(entity).numSubmeshes);
    }

    for (int s = 0; s < numSubmeshes; s++){
        ImGui::SeparatorText(("Submesh "+std::to_string(s+1)).c_str());

        float submeshesTableSize = getLabelSize("Texture Shadow");

        beginTable(cpType, submeshesTableSize, "submeshes");

        if (propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material", "Material", sceneProject, entities)){
            endTable();
            beginTable(cpType, getLabelSize("Met. Roug. Texture"), "material_table");
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.basecolor", "Base Color", sceneProject, entities, 0.1f, -1, true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.basecolortexture", "Base Texture", sceneProject, entities, 0.1f, -1, true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.metallicfactor", "Metallic Factor", sceneProject, entities, 0.01f, 4 * ImGui::GetFontSize(), true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.roughnessfactor", "Roughness Factor", sceneProject, entities, 0.01f, 4 * ImGui::GetFontSize(), true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.metallicroughnesstexture", "Met. Roug. Texture", sceneProject, entities, 0.1f, -1, true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.emissivefactor", "Emissive Factor", sceneProject, entities, 0.1f, -1, true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.emissivetexture", "Emissive Texture", sceneProject, entities, 0.1f, -1, true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.occlusiontexture", "Occlusion Texture", sceneProject, entities, 0.1f, -1, true);
            propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].material.normalTexture", "Normal Texture", sceneProject, entities, 0.1f, -1, true);
            endTable();
            beginTable(cpType, submeshesTableSize, "submeshes");
        }

        propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].face_culling", "Face Culling", sceneProject, entities);
        propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].texture_shadow", "Texture Shadow", sceneProject, entities);
        propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].primitive_type", "Primitive", sceneProject, entities);
        propertyRow(cpType, props, "submeshes["+std::to_string(s)+"].texture_rect", "Texture Rect", sceneProject, entities);

        endTable();
    }
}

void Editor::Properties::drawUIComponent(ComponentType cpType, std::map<std::string, PropertyData> props, SceneProject* sceneProject, std::vector<Entity> entities){
    beginTable(cpType, getLabelSize("Texture"));

    propertyRow(cpType, props, "color", "Color", sceneProject, entities);
    propertyRow(cpType, props, "texture", "Texture", sceneProject, entities);

    endTable();
}

void Editor::Properties::drawUILayoutComponent(ComponentType cpType, std::map<std::string, PropertyData> props, SceneProject* sceneProject, std::vector<Entity> entities){
    beginTable(cpType, getLabelSize("Ignore Scissor"));

    propertyRow(cpType, props, "width", "Width", sceneProject, entities, 1.0, 6 * ImGui::GetFontSize());
    propertyRow(cpType, props, "height", "Height", sceneProject, entities, 1.0, 6 * ImGui::GetFontSize());
    propertyRow(cpType, props, "ignore_scissor", "Ignore Scissor", sceneProject, entities);

    endTable();
}

void Editor::Properties::drawImageComponent(ComponentType cpType, std::map<std::string, PropertyData> props, SceneProject* sceneProject, std::vector<Entity> entities){
    ImGui::SeparatorText("Nine-patch rect");

    if (entities.size() == 1) {
        if (UIComponent* ui = sceneProject->scene->findComponent<UIComponent>(entities[0])){
            Texture* thumbTexture = findThumbnail(ui->texture.getId());
            if (thumbTexture) {
                drawNinePatchesPreview(sceneProject->scene->getComponent<ImageComponent>(entities[0]), &ui->texture, thumbTexture, ImVec2(THUMBNAIL_SIZE, THUMBNAIL_SIZE));
            }
        }
    }

    beginTable(cpType, getLabelSize("Margin Bottom"), "nine_margin_table");

    propertyRow(cpType, props, "patch_margin_left", "Margin Left", sceneProject, entities, 1.0, 6 * ImGui::GetFontSize());
    propertyRow(cpType, props, "patch_margin_right", "Margin Right", sceneProject, entities, 1.0, 6 * ImGui::GetFontSize());
    propertyRow(cpType, props, "patch_margin_top", "Margin Top", sceneProject, entities, 1.0, 6 * ImGui::GetFontSize());
    propertyRow(cpType, props, "patch_margin_bottom", "Margin Bottom", sceneProject, entities, 1.0, 6 * ImGui::GetFontSize());

    endTable();

    beginTable(cpType, getLabelSize("Texture Scale"));

    propertyRow(cpType, props, "texture_scale_factor", "Texture Scale", sceneProject, entities, 0.1f, 6 * ImGui::GetFontSize(), false, "Increase or decrease texture area by a factor");

    endTable();
}

void Editor::Properties::drawSpriteComponent(ComponentType cpType, std::map<std::string, PropertyData> props, SceneProject* sceneProject, std::vector<Entity> entities){
    beginTable(cpType, getLabelSize("Texture Scale"));

    propertyRow(cpType, props, "width", "Width", sceneProject, entities, 1.0, 6 * ImGui::GetFontSize());
    propertyRow(cpType, props, "height", "Height", sceneProject, entities, 1.0, 6 * ImGui::GetFontSize());
    propertyRow(cpType, props, "pivot_preset", "Pivot", sceneProject, entities);
    propertyRow(cpType, props, "texture_scale_factor", "Texture Scale", sceneProject, entities, 0.1f, 6 * ImGui::GetFontSize(), false, "Increase or decrease texture area by a factor");

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
                    CommandHandle::get(sceneProject->id)->addCommandNoMerge(new EntityNameCmd(sceneProject, entities[0], nameBuffer));
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
                    drawTransform(cpType, Catalog::getProperties(cpType, nullptr), sceneProject, entities);
                }else if (cpType == ComponentType::MeshComponent){
                    drawMeshComponent(cpType, Catalog::getProperties(cpType, nullptr), sceneProject, entities);
                }else if (cpType == ComponentType::UIComponent){
                    drawUIComponent(cpType, Catalog::getProperties(cpType, nullptr), sceneProject, entities);
                }else if (cpType == ComponentType::UILayoutComponent){
                    drawUILayoutComponent(cpType, Catalog::getProperties(cpType, nullptr), sceneProject, entities);
                }else if (cpType == ComponentType::ImageComponent){
                    drawImageComponent(cpType, Catalog::getProperties(cpType, nullptr), sceneProject, entities);
                }else if (cpType == ComponentType::SpriteComponent){
                    drawSpriteComponent(cpType, Catalog::getProperties(cpType, nullptr), sceneProject, entities);
                }

            }
        }

    }else{
        thumbnailTextures.clear();
    }

    ImGui::End();
}