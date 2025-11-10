#include "Properties.h"

#include "imgui_internal.h"

#include "util/Util.h"
#include "util/FileDialogs.h"
#include "util/EntityPayload.h"
#include "external/IconsFontAwesome6.h"
#include "command/CommandHandle.h"
#include "command/type/PropertyCmd.h"
#include "command/type/MultiPropertyCmd.h"
#include "command/type/EntityNameCmd.h"
#include "command/type/MeshChangeCmd.h"
#include "command/type/AddComponentCmd.h"
#include "command/type/RemoveComponentCmd.h"
#include "command/type/ComponentToSharedCmd.h"
#include "command/type/ComponentToLocalCmd.h"
#include "render/SceneRender2D.h"
#include "util/SHA1.h"
#include "Stream.h"
#include "Out.h"

#include <map>

using namespace Supernova;

static std::vector<Editor::EnumEntry> entriesPrimitiveType = {
    { (int)PrimitiveType::TRIANGLES, "Triangles" },
    { (int)PrimitiveType::TRIANGLE_STRIP, "Triangle Strip" },
    { (int)PrimitiveType::POINTS, "Points" },
    { (int)PrimitiveType::LINES, "Lines" }
};

static std::vector<Editor::EnumEntry> entriesPivotPreset = {
    { (int)PivotPreset::CENTER, "Center" },
    { (int)PivotPreset::TOP_CENTER, "Top Center" },
    { (int)PivotPreset::BOTTOM_CENTER, "Bottom Center" },
    { (int)PivotPreset::LEFT_CENTER, "Left Center" },
    { (int)PivotPreset::RIGHT_CENTER, "Right Center" },
    { (int)PivotPreset::TOP_LEFT, "Top Left" },
    { (int)PivotPreset::BOTTOM_LEFT, "Bottom Left" },
    { (int)PivotPreset::TOP_RIGHT, "Top Right" },
    { (int)PivotPreset::BOTTOM_RIGHT, "Bottom Right" }
};

static std::vector<Editor::EnumEntry> entriesLightType = {
    { (int)LightType::DIRECTIONAL, "Directional" },
    { (int)LightType::POINT, "Point" },
    { (int)LightType::SPOT, "Spot" }
};

static std::vector<int> cascadeValues = { 1, 2, 3, 4, 5, 6 };
static std::vector<int> po2Values = { 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384 };

Editor::Properties::Properties(Project* project){
    this->project = project;
    this->cmd = nullptr;

    this->finishProperty = false;
}

Editor::RowPropertyType Editor::Properties::scriptPropertyTypeToRowPropertyType(ScriptPropertyType scriptType){
    switch (scriptType) {
        case Supernova::ScriptPropertyType::Bool: return RowPropertyType::Bool;
        case Supernova::ScriptPropertyType::Int: return RowPropertyType::Int;
        case Supernova::ScriptPropertyType::Float: return RowPropertyType::Float;
        case Supernova::ScriptPropertyType::String: return RowPropertyType::String;
        case Supernova::ScriptPropertyType::Vector2: return RowPropertyType::Vector2;
        case Supernova::ScriptPropertyType::Vector3: return RowPropertyType::Vector3;
        case Supernova::ScriptPropertyType::Vector4: return RowPropertyType::Vector4;
        case Supernova::ScriptPropertyType::Color3: return RowPropertyType::Color3L;
        case Supernova::ScriptPropertyType::Color4: return RowPropertyType::Color4L;
        case Supernova::ScriptPropertyType::EntityPointer: return RowPropertyType::EntityPointer;
        default: return RowPropertyType::Custom;
    }
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
        thumbnailPath = project->getThumbnailPath(texPath);

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

void Editor::Properties::drawImageWithBorderAndRounding(Texture* texture, const ImVec2& size, float rounding, ImU32 border_col, float border_thickness, bool flipY) {
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

    if (flipY) {
        float temp = uv0.y;
        uv0.y = uv1.y;
        uv1.y = temp;
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


void Editor::Properties::dragDropResources(ComponentType cpType, std::string id, SceneProject* sceneProject, std::vector<Entity> entities, ComponentType componentType){
    // Block DnD while playing for non-script components
    if (sceneProject) {
        return;
    }

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
                            if (componentType == ComponentType::MeshComponent){
                                unsigned int numSubmeshes = sceneProject->scene->getComponent<MeshComponent>(entity).numSubmeshes;
                                for (unsigned int i = 0; i < numSubmeshes; i++){
                                    sceneProject->scene->getComponent<MeshComponent>(entity).submeshes[i].needUpdateTexture = true;
                                }
                            }
                            if (componentType == ComponentType::UIComponent){
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
                        cmd = new PropertyCmd<Texture>(project, sceneProject->id, entity, cpType, id, texture);
                        CommandHandle::get(sceneProject->id)->addCommand(cmd);
                        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
                            finishProperty = true;
                        }
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
                    if (componentType == ComponentType::MeshComponent){
                        unsigned int numSubmeshes = sceneProject->scene->getComponent<MeshComponent>(entity).numSubmeshes;
                        for (unsigned int i = 0; i < numSubmeshes; i++){
                            sceneProject->scene->getComponent<MeshComponent>(entity).submeshes[i].needUpdateTexture = true;
                        }
                    }
                    if (componentType == ComponentType::UIComponent){
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

void Editor::Properties::handleComponentMenu(SceneProject* sceneProject, std::vector<Entity> entities, ComponentType cpType, bool isSharedGroup, bool isComponentOverridden, bool& headerOpen, bool readOnly) {
    if (ImGui::BeginPopupContextItem(("component_options_menu_" + std::to_string(static_cast<int>(cpType))).c_str())) {
        ImGui::TextDisabled("Component options");
        ImGui::Separator();

        ImGui::BeginDisabled(readOnly); // disable all actions while playing

        if (isSharedGroup){
            if (isComponentOverridden) {
                if (ImGui::MenuItem(ICON_FA_LINK " Revert to Shared")) {
                    for (Entity& entity : entities){
                        cmd = new ComponentToSharedCmd(project, sceneProject->id, entity, cpType);
                        CommandHandle::get(sceneProject->id)->addCommand(cmd);
                    }
                    cmd->setNoMerge();
                }

            } else {
                if (ImGui::MenuItem(ICON_FA_LOCK_OPEN " Make Unique")) {
                    for (Entity& entity : entities){
                        cmd = new ComponentToLocalCmd(project, sceneProject->id, entity, cpType);
                        CommandHandle::get(sceneProject->id)->addCommand(cmd);
                    }
                    cmd->setNoMerge();
                }
            }
        }

        bool canRemove = !(cpType == ComponentType::Transform && isSharedGroup);
        if (ImGui::MenuItem(ICON_FA_TRASH " Remove", nullptr, false, canRemove)) {
            for (Entity& entity : entities){
                cmd = new RemoveComponentCmd(project, sceneProject->id, entity, cpType);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
            cmd->setNoMerge();

            headerOpen = false;
        }

        ImGui::EndDisabled();

        ImGui::EndPopup();
    }
}

bool Editor::Properties::canAddComponent(SceneProject* sceneProject, Entity entity, ComponentType cpType) {
    // Check if entity already has this component
    std::vector<ComponentType> existingComponents = Catalog::findComponents(sceneProject->scene, entity);
    return std::find(existingComponents.begin(), existingComponents.end(), cpType) == existingComponents.end();
}

Texture Editor::Properties::getMaterialPreview(const Material& material, const std::string id){
    MaterialRender& materialRender = materialRenders[id];

    if ((materialRender.getMaterial() != material) || !materialRender.getFramebuffer()->isCreated()){
        materialRender.applyMaterial(material);
        Engine::executeSceneOnce(materialRender.getScene());
    }

    usedPreviewIds.insert(id);

    return materialRender.getTexture();
}

Texture Editor::Properties::getDirectionPreview(const Vector3& direction, const std::string id){
    DirectionRender& directionRender = directionRenders[id];

    if ((directionRender.getDirection() != direction) || !directionRender.getFramebuffer()->isCreated()){
        directionRender.setDirection(direction);
        Engine::executeSceneOnce(directionRender.getScene());
    }

    usedPreviewIds.insert(id);

    return directionRender.getTexture();
}

void Editor::Properties::updateShapePreview(const ShapeParameters& shapeParams){
    ImVec4 frameBgColor = ImGui::GetStyle().Colors[ImGuiCol_FrameBg];
    std::shared_ptr<Supernova::MeshSystem> meshSys = shapePreviewRender.getScene()->getSystem<MeshSystem>();

    MeshComponent meshComp;

    updateMeshShape(meshComp, meshSys.get(), shapeParams);

    shapePreviewRender.applyMesh(Stream::encodeMeshComponent(meshComp), true, true);
    shapePreviewRender.setBackground(Vector4(frameBgColor.x, frameBgColor.y, frameBgColor.z, frameBgColor.w));

    Engine::executeSceneOnce(shapePreviewRender.getScene());
}

void Editor::Properties::updateMeshShape(MeshComponent& meshComp, MeshSystem* meshSys, const ShapeParameters& shapeParams){
    switch (shapeParams.geometryType) {
        case 0: // Plane
            meshSys->createPlane(meshComp, shapeParams.planeWidth, shapeParams.planeDepth, shapeParams.planeTiles);
            break;
        case 1: // Box
            meshSys->createBox(meshComp, shapeParams.boxWidth, shapeParams.boxHeight, shapeParams.boxDepth, shapeParams.boxTiles);
            break;
        case 2: // Sphere
            meshSys->createSphere(meshComp, shapeParams.sphereRadius, shapeParams.sphereSlices, shapeParams.sphereStacks);
            break;
        case 3: // Cylinder
            meshSys->createCylinder(meshComp, shapeParams.cylinderBaseRadius, shapeParams.cylinderTopRadius, shapeParams.cylinderHeight, shapeParams.cylinderSlices, shapeParams.cylinderStacks);
            break;
        case 4: // Capsule
            meshSys->createCapsule(meshComp, shapeParams.capsuleBaseRadius, shapeParams.capsuleTopRadius, shapeParams.capsuleHeight, shapeParams.capsuleSlices, shapeParams.capsuleStacks);
            break;
        case 5: // Torus
            meshSys->createTorus(meshComp, shapeParams.torusRadius, shapeParams.torusRingRadius, shapeParams.torusSides, shapeParams.torusRings);
            break;
    }
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

bool Editor::Properties::propertyRow(RowPropertyType type, ComponentType cpType, std::string id, std::string label, SceneProject* sceneProject, std::vector<Entity> entities, RowSettings settings){
    bool result = true;

    constexpr float compThreshold = 1e-4;
    constexpr float zeroThreshold = 1e-4;

    if (type == RowPropertyType::Vector2){
        Vector2* value = nullptr;
        bool difX = false;
        bool difY = false;
        std::map<Entity, Vector2> eValue;
        float* defArr = nullptr;
        for (Entity& entity : entities){
            PropertyData prop = Catalog::getProperty(sceneProject->scene, entity, cpType, id);
            defArr = static_cast<float*>(prop.def);
            eValue[entity] = *static_cast<Vector2*>(prop.ref);
            if (value){
                if (std::fabs(value->x - eValue[entity].x) > compThreshold)
                    difX = true;
                if (std::fabs(value->y - eValue[entity].y) > compThreshold)
                    difY = true;
            }
            value = &eValue[entity];
        }

        Vector2 newValue = *value;

        bool defChanged = false;
        if (defArr){
            defChanged = compareVectorFloat((float*)&newValue, defArr, 2, compThreshold);
        }
        if (propertyHeader(label, settings.secondColSize, defChanged, settings.child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector2>(project, sceneProject->id, entity, cpType, id, static_cast<Vector2>(defArr), settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
                finishProperty = true;
            }
        }

        ImGui::BeginGroup();
        ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());

        if (difX)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_x_"+id).c_str(), &(newValue.x), settings.stepSize, 0.0f, 0.0f, settings.format)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector2>(project, sceneProject->id, entity, cpType, id, Vector2(newValue.x, eValue[entity].y), settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }
        if (difX)
            ImGui::PopStyleColor();

        ImGui::SameLine();
        if (difY)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_y_"+id).c_str(), &(newValue.y), settings.stepSize, 0.0f, 0.0f, settings.format)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector2>(project, sceneProject->id, entity, cpType, id, Vector2(eValue[entity].x, newValue.y), settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }
        if (difY)
            ImGui::PopStyleColor();

        ImGui::EndGroup();
        //ImGui::SetItemTooltip("%s (X, Y, Z)", prop.label.c_str());

    }else if (type == RowPropertyType::Vector3 || type == RowPropertyType::Direction){
        Vector3* value = nullptr;
        bool difX = false;
        bool difY = false;
        bool difZ = false;
        std::map<Entity, Vector3> eValue;
        float* defArr = nullptr;
        for (Entity& entity : entities){
            PropertyData prop = Catalog::getProperty(sceneProject->scene, entity, cpType, id);
            defArr = static_cast<float*>(prop.def);
            eValue[entity] = *static_cast<Vector3*>(prop.ref);
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
        if (defArr){
            defChanged = compareVectorFloat((float*)&newValue, defArr, 3, compThreshold);
        }
        if (propertyHeader(label, settings.secondColSize, defChanged, settings.child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector3>(project, sceneProject->id, entity, cpType, id, static_cast<Vector3>(defArr), settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
                finishProperty = true;
            }
        }

        ImGui::BeginGroup();

        float min = 0.0f;
        float max = 0.0f;
        if (type == RowPropertyType::Direction){
            min = -1.0f;
            max = 1.0f;

            Texture dirTexRender = getDirectionPreview(newValue, id);
            float thumbSize = ImGui::GetFrameHeight() * 3;
            ImU32 border_col = IM_COL32(128, 128, 128, 255); // Gray border

            drawImageWithBorderAndRounding(&dirTexRender, ImVec2(thumbSize, thumbSize), 4.0f, border_col, 1.0f, true);

            static bool draggingDirection = false;

            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                draggingDirection = true;

                ImVec2 mouseDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);

                float sensitivity = 1.5f;

                float yawAngle = mouseDelta.x * sensitivity;   // Horizontal movement -> roll (around Y)
                float pitchAngle = -mouseDelta.y * sensitivity; // Vertical movement -> pitch (around X)

                Quaternion rollRotation(yawAngle, Vector3(0, 0, 1));     // Roll around world Z-axis
                Quaternion pitchRotation(pitchAngle, Vector3(1, 0, 0)); // Pitch around X-axis
                // Combine rotations (order matters: apply roll first, then pitch)
                Vector3 newDirection =  rollRotation * pitchRotation * newValue;

                // Apply to all entities
                for (Entity& entity : entities) {
                    cmd = new PropertyCmd<Vector3>(project, sceneProject->id, entity, cpType, id, newDirection, settings.onValueChanged);
                    CommandHandle::get(sceneProject->id)->addCommand(cmd);
                }

                newValue = newDirection;
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
            }

            if (draggingDirection && ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
                finishProperty = true;
            }
        }

        ImGui::SetNextItemWidth(settings.secondColSize);

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());

        // Define axis colors
        ImU32 axisColors[3] = {
            IM_COL32(220, 60, 60, 255),   // Red for X
            IM_COL32(60, 220, 60, 255),   // Green for Y
            IM_COL32(60, 60, 220, 255)    // Blue for Z
        };

        // Get draw list for drawing inside input fields
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        float colorBarWidth = 4.0f; // Width of the colored bar

        if (difX)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);

        // Store cursor position before drawing the input
        ImVec2 inputPosX = ImGui::GetCursorScreenPos();

        if (ImGui::DragFloat(("##input_x_"+id).c_str(), &(newValue.x), settings.stepSize, min, max, settings.format)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector3>(project, sceneProject->id, entity, cpType, id, Vector3(newValue.x, eValue[entity].y, eValue[entity].z), settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }

        // Draw red bar inside the input field
        float inputWidth = ImGui::GetItemRectSize().x;
        float inputHeight = ImGui::GetItemRectSize().y;
        drawList->AddRectFilled(
            ImVec2(inputPosX.x + 2, inputPosX.y + 2),
            ImVec2(inputPosX.x + 2 + colorBarWidth, inputPosX.y + inputHeight - 2),
            axisColors[0]
        );

        if (difX)
            ImGui::PopStyleColor();

        ImGui::SameLine();
        if (difY)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);

        // Store cursor position before drawing the input
        ImVec2 inputPosY = ImGui::GetCursorScreenPos();

        if (ImGui::DragFloat(("##input_y_"+id).c_str(), &(newValue.y), settings.stepSize, min, max, settings.format)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector3>(project, sceneProject->id, entity, cpType, id, Vector3(eValue[entity].x, newValue.y, eValue[entity].z), settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }

        // Draw green bar inside the input field
        drawList->AddRectFilled(
            ImVec2(inputPosY.x + 2, inputPosY.y + 2),
            ImVec2(inputPosY.x + 2 + colorBarWidth, inputPosY.y + inputHeight - 2),
            axisColors[1]
        );

        if (difY)
            ImGui::PopStyleColor();

        ImGui::SameLine();
        if (difZ)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);

        // Store cursor position before drawing the input
        ImVec2 inputPosZ = ImGui::GetCursorScreenPos();

        if (ImGui::DragFloat(("##input_z_"+id).c_str(), &(newValue.z), settings.stepSize, min, max, settings.format)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector3>(project, sceneProject->id, entity, cpType, id, Vector3(eValue[entity].x, eValue[entity].y, newValue.z), settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }

        // Draw blue bar inside the input field
        drawList->AddRectFilled(
            ImVec2(inputPosZ.x + 2, inputPosZ.y + 2),
            ImVec2(inputPosZ.x + 2 + colorBarWidth, inputPosZ.y + inputHeight - 2),
            axisColors[2]
        );

        if (difZ)
            ImGui::PopStyleColor();

        ImGui::EndGroup();
        //ImGui::SetItemTooltip("%s (X, Y, Z)", prop.label.c_str());

    }else if (type == RowPropertyType::Vector4){
        Vector4* value = nullptr;
        bool difX = false;
        bool difY = false;
        bool difZ = false;
        bool difW = false;
        std::map<Entity, Vector4> eValue;
        float* defArr = nullptr;
        for (Entity& entity : entities){
            PropertyData prop = Catalog::getProperty(sceneProject->scene, entity, cpType, id);
            defArr = static_cast<float*>(prop.def);
            eValue[entity] = *static_cast<Vector4*>(prop.ref);
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
        if (defArr){
            defChanged = compareVectorFloat((float*)&newValue, defArr, 4, compThreshold);
        }
        if (propertyHeader(label, settings.secondColSize, defChanged, settings.child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(project, sceneProject->id, entity, cpType, id, static_cast<Vector4>(defArr), settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
                finishProperty = true;
            }
        }

        ImGui::BeginGroup();
        ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth());

        if (difX)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_x_"+id).c_str(), &(newValue.x), settings.stepSize, 0.0f, 0.0f, settings.format)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(project, sceneProject->id, entity, cpType, id, Vector4(newValue.x, eValue[entity].y, eValue[entity].z, eValue[entity].w), settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }
        if (difX)
            ImGui::PopStyleColor();

        ImGui::SameLine();
        if (difY)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_y_"+id).c_str(), &(newValue.y), settings.stepSize, 0.0f, 0.0f, settings.format)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(project, sceneProject->id, entity, cpType, id, Vector4(eValue[entity].x, newValue.y, eValue[entity].z, eValue[entity].w), settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }
        if (difY)
            ImGui::PopStyleColor();

        ImGui::SameLine();
        if (difZ)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_z_"+id).c_str(), &(newValue.z), settings.stepSize, 0.0f, 0.0f, settings.format)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(project, sceneProject->id, entity, cpType, id, Vector4(eValue[entity].x, eValue[entity].y, newValue.z, eValue[entity].w), settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }
        if (difZ)
            ImGui::PopStyleColor();

        ImGui::SameLine();
        if (difW)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_w_"+id).c_str(), &(newValue.w), settings.stepSize, 0.0f, 0.0f, settings.format)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(project, sceneProject->id, entity, cpType, id, Vector4(eValue[entity].x, eValue[entity].y, eValue[entity].z, newValue.w), settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }
        if (difW)
            ImGui::PopStyleColor();

        ImGui::EndGroup();
        //ImGui::SetItemTooltip("%s (X, Y, Z)", prop.label.c_str());

    }else if (type == RowPropertyType::Quat){
        RotationOrder order = RotationOrder::ZYX;
        Vector3* value = nullptr;
        bool difX = false;
        bool difY = false;
        bool difZ = false;
        std::map<Entity, Vector3> eValue;
        Quaternion qValue;
        float* defArr = nullptr;
        for (Entity& entity : entities){
            PropertyData prop = Catalog::getProperty(sceneProject->scene, entity, cpType, id);
            defArr = static_cast<float*>(prop.def);
            qValue = *static_cast<Quaternion*>(prop.ref);
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
        if (defArr){
            defChanged = compareVectorFloat((float*)&qValue, defArr, 4, compThreshold);
        }
        if (propertyHeader(label, settings.secondColSize, defChanged, settings.child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Quaternion>(project, sceneProject->id, entity, cpType, id, static_cast<Quaternion>(defArr), settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
                finishProperty = true;
            }
        }

        ImGui::BeginGroup();
        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());

        // Define axis colors
        ImU32 axisColors[3] = {
            IM_COL32(220, 60, 60, 255),   // Red for X
            IM_COL32(60, 220, 60, 255),   // Green for Y
            IM_COL32(60, 60, 220, 255)    // Blue for Z
        };

        // Get draw list for drawing inside input fields
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        float colorBarWidth = 4.0f; // Width of the colored bar

        if (difX)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);

        // Store cursor position before drawing the input
        ImVec2 inputPosX = ImGui::GetCursorScreenPos();

        if (ImGui::DragFloat(("##input_x_"+id).c_str(), &(newValue.x), settings.stepSize, 0.0f, 0.0f, (std::string(settings.format) + "°").c_str())){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Quaternion>(project, sceneProject->id, entity, cpType, id, Quaternion(newValue.x, eValue[entity].y, eValue[entity].z, order), settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }

        // Draw red bar inside the input field
        float inputWidth = ImGui::GetItemRectSize().x;
        float inputHeight = ImGui::GetItemRectSize().y;
        drawList->AddRectFilled(
            ImVec2(inputPosX.x + 2, inputPosX.y + 2),
            ImVec2(inputPosX.x + 2 + colorBarWidth, inputPosX.y + inputHeight - 2),
            axisColors[0]
        );

        if (difX)
            ImGui::PopStyleColor();

        ImGui::SameLine();
        if (difY)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);

        // Store cursor position before drawing the input
        ImVec2 inputPosY = ImGui::GetCursorScreenPos();

        if (ImGui::DragFloat(("##input_y_"+id).c_str(), &(newValue.y), settings.stepSize, 0.0f, 0.0f, "%.2f°")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Quaternion>(project, sceneProject->id, entity, cpType, id, Quaternion(eValue[entity].x, newValue.y, eValue[entity].z, order), settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }

        // Draw green bar inside the input field
        drawList->AddRectFilled(
            ImVec2(inputPosY.x + 2, inputPosY.y + 2),
            ImVec2(inputPosY.x + 2 + colorBarWidth, inputPosY.y + inputHeight - 2),
            axisColors[1]
        );

        if (difY)
            ImGui::PopStyleColor();

        ImGui::SameLine();
        if (difZ)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);

        // Store cursor position before drawing the input
        ImVec2 inputPosZ = ImGui::GetCursorScreenPos();

        if (ImGui::DragFloat(("##input_z_"+id).c_str(), &(newValue.z), settings.stepSize, 0.0f, 0.0f, "%.2f°")){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Quaternion>(project, sceneProject->id, entity, cpType, id, Quaternion(eValue[entity].x, eValue[entity].y, newValue.z, order), settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }

        // Draw blue bar inside the input field
        drawList->AddRectFilled(
            ImVec2(inputPosZ.x + 2, inputPosZ.y + 2),
            ImVec2(inputPosZ.x + 2 + colorBarWidth, inputPosZ.y + inputHeight - 2),
            axisColors[2]
        );

        if (difZ)
            ImGui::PopStyleColor();

        ImGui::EndGroup();
        //ImGui::SetItemTooltip("%s in degrees (X, Y, Z)", prop.label.c_str());

    }else if (type == RowPropertyType::Bool){
        bool* value = nullptr;
        std::map<Entity, bool> eValue;
        bool dif = false;
        bool* defArr = nullptr;
        for (Entity& entity : entities){
            PropertyData prop = Catalog::getProperty(sceneProject->scene, entity, cpType, id);
            defArr = static_cast<bool*>(prop.def);
            eValue[entity] = *static_cast<bool*>(prop.ref);
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }

        bool newValue = *value;

        bool defChanged = false;
        if (defArr){
            defChanged = (newValue != *defArr);
        }
        if (propertyHeader(label, settings.secondColSize, defChanged, settings.child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<bool>(project, sceneProject->id, entity, cpType, id, *defArr, settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
                finishProperty = true;
            }
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_CheckMark, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::Checkbox(("##checkbox_"+id).c_str(), &newValue)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<bool>(project, sceneProject->id, entity, cpType, id, newValue, settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }
        if (dif)
            ImGui::PopStyleColor();
        //ImGui::SetItemTooltip("%s", prop.label.c_str());

    }else if (type == RowPropertyType::Float || type == RowPropertyType::Float_0_1 || type == RowPropertyType::HalfCone){
        float* value = nullptr;
        std::map<Entity, float> eValue;
        bool dif = false;
        float* defArr = nullptr;
        for (Entity& entity : entities){
            PropertyData prop = Catalog::getProperty(sceneProject->scene, entity, cpType, id);
            defArr = static_cast<float*>(prop.def);
            eValue[entity] = *static_cast<float*>(prop.ref);
            if (type == RowPropertyType::HalfCone){
                eValue[entity] = Angle::radToDefault(std::acos(eValue[entity]) * 2);
            }
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }

        float newValue = *value;

        bool defChanged = false;
        if (defArr){
            if (type == RowPropertyType::HalfCone){
                float def = *defArr;
                defChanged = (newValue != Angle::radToDefault(std::acos(def) * 2));
            }else{
                defChanged = (newValue != *defArr);
            }
        }
        if (propertyHeader(label, settings.secondColSize, defChanged, settings.child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<float>(project, sceneProject->id, entity, cpType, id, *defArr, settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
                finishProperty = true;
            }
        }

        float v_min = (0.0F);
        float v_max = (0.0F);
        if (type == RowPropertyType::Float_0_1){
            v_min = 0.0F;
            v_max = 1.0F;
        }

        std::string newFormat = settings.format;
        if (type == RowPropertyType::HalfCone){
            newFormat = std::string(settings.format) + "°";
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragFloat(("##input_float_"+id).c_str(), &newValue, settings.stepSize, v_min, v_max, newFormat.c_str())){
            for (Entity& entity : entities){
                if (type == RowPropertyType::HalfCone){
                    cmd = new PropertyCmd<float>(project, sceneProject->id, entity, cpType, id, cos(Angle::defaultToRad(newValue / 2)), settings.onValueChanged);
                }else{
                    cmd = new PropertyCmd<float>(project, sceneProject->id, entity, cpType, id, newValue, settings.onValueChanged);
                }
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }
        if (dif)
            ImGui::PopStyleColor();
        //ImGui::SetItemTooltip("%s", prop.label.c_str());

        if (!settings.help.empty()){
            ImGui::SameLine(); helpMarker(settings.help);
        }

    }else if (type == RowPropertyType::UInt){
        unsigned int* value = nullptr;
        std::map<Entity, unsigned int> eValue;
        bool dif = false;
        unsigned int* defArr = nullptr;
        for (Entity& entity : entities){
            PropertyData prop = Catalog::getProperty(sceneProject->scene, entity, cpType, id);
            defArr = static_cast<unsigned int*>(prop.def);
            eValue[entity] = *static_cast<unsigned int*>(prop.ref);
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }

        unsigned int newValue = *value;

        bool defChanged = false;
        if (defArr){
            defChanged = (newValue != *defArr);
        }
        if (propertyHeader(label, settings.secondColSize, defChanged, settings.child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<unsigned int>(project, sceneProject->id, entity, cpType, id, *defArr, settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
                finishProperty = true;
            }
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragInt(("##input_uint_"+id).c_str(), (int*)&newValue, static_cast<int>(ceil(settings.stepSize)), 0.0f, INT_MAX)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<unsigned int>(project, sceneProject->id, entity, cpType, id, newValue, settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }
        if (dif)
            ImGui::PopStyleColor();
        //ImGui::SetItemTooltip("%s", prop.label.c_str());

    }else if (type == RowPropertyType::Int){
        int* value = nullptr;
        std::map<Entity, int> eValue;
        bool dif = false;
        int* defArr = nullptr;
        for (Entity& entity : entities){
            PropertyData prop = Catalog::getProperty(sceneProject->scene, entity, cpType, id);
            defArr = static_cast<int*>(prop.def);
            eValue[entity] = *static_cast<int*>(prop.ref);
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }

        int newValue = *value;

        bool defChanged = false;
        if (defArr){
            defChanged = (newValue != *defArr);
        }
        if (propertyHeader(label, settings.secondColSize, defChanged, settings.child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<int>(project, sceneProject->id, entity, cpType, id, *defArr, settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
                finishProperty = true;
            }
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::DragInt(("##input_int_"+id).c_str(), &newValue, static_cast<int>(ceil(settings.stepSize)), 0.0f, 0.0f)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<int>(project, sceneProject->id, entity, cpType, id, newValue, settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }
        if (dif)
            ImGui::PopStyleColor();
        //ImGui::SetItemTooltip("%s", prop.label.c_str());

    }else if (type == RowPropertyType::Color3L){
        Vector3* value = nullptr;
        std::map<Entity, Vector3> eValue;
        bool dif = false;
        float* defArr = nullptr;
        for (Entity& entity : entities){
            PropertyData prop = Catalog::getProperty(sceneProject->scene, entity, cpType, id);
            defArr = static_cast<float*>(prop.def);
            eValue[entity] = *static_cast<Vector3*>(prop.ref);
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }

        Vector3 newValue = Color::linearTosRGB(*value);

        // using 'value' beacause it is linear too
        bool defChanged = false;
        if (defArr){
            defChanged = compareVectorFloat((float*)value, defArr, 3, compThreshold);
        }
        if (propertyHeader(label, settings.secondColSize, defChanged, settings.child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector3>(project, sceneProject->id, entity, cpType, id, static_cast<Vector3>(defArr), settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
                finishProperty = true;
            }
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::ColorEdit3((label+"##checkbox_"+id).c_str(), (float*)&newValue.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector3>(project, sceneProject->id, entity, cpType, id, Color::sRGBToLinear(newValue), settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }
        if (dif)
            ImGui::PopStyleColor();
        //ImGui::SetItemTooltip("%s", prop.label.c_str());

    }else if (type == RowPropertyType::Color4L){
        Vector4* value = nullptr;
        std::map<Entity, Vector4> eValue;
        bool dif = false;
        float* defArr = nullptr;
        for (Entity& entity : entities){
            PropertyData prop = Catalog::getProperty(sceneProject->scene, entity, cpType, id);
            defArr = static_cast<float*>(prop.def);
            eValue[entity] = *static_cast<Vector4*>(prop.ref);
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }

        Vector4 newValue = Color::linearTosRGB(*value);

        // using 'value' beacause it is linear too
        bool defChanged = false;
        if (defArr){
            defChanged = compareVectorFloat((float*)value, defArr, 4, compThreshold);
        }
        if (propertyHeader(label, settings.secondColSize, defChanged, settings.child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(project, sceneProject->id, entity, cpType, id, static_cast<Vector4>(defArr), settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
                finishProperty = true;
            }
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::ColorEdit4((label+"##checkbox_"+id).c_str(), (float*)&newValue.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Vector4>(project, sceneProject->id, entity, cpType, id, Color::sRGBToLinear(newValue), settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }
        if (dif)
            ImGui::PopStyleColor();
        //ImGui::SetItemTooltip("%s", prop.label.c_str());

    }else if ((type == RowPropertyType::IntSlider || type == RowPropertyType::UIntSlider) && settings.sliderValues){
        int* value = nullptr;
        std::map<Entity, int> eValue;
        bool dif = false;
        void* defArr = nullptr;
        std::vector<int>* sliderValues = nullptr;
        for (Entity& entity : entities){
            PropertyData prop = Catalog::getProperty(sceneProject->scene, entity, cpType, id);
            defArr = prop.def;
            sliderValues = settings.sliderValues;
            if (type == RowPropertyType::IntSlider) {
                eValue[entity] = *static_cast<int*>(prop.ref);
            } else {
                eValue[entity] = *static_cast<unsigned int*>(prop.ref);
            }
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }

        int newValue = *value;

        bool defChanged = false;
        if (defArr){
            if (type == RowPropertyType::IntSlider) {
                defChanged = (newValue != *static_cast<int*>(defArr));
            } else {
                defChanged = (newValue != *static_cast<unsigned int*>(defArr));
            }
        }
        if (propertyHeader(label, settings.secondColSize, defChanged, settings.child)){
            for (Entity& entity : entities){
                if (type == RowPropertyType::IntSlider) {
                    cmd = new PropertyCmd<int>(project, sceneProject->id, entity, cpType, id, *static_cast<int*>(defArr), settings.onValueChanged);
                } else {
                    cmd = new PropertyCmd<unsigned int>(project, sceneProject->id, entity, cpType, id, *static_cast<unsigned int*>(defArr), settings.onValueChanged);
                }
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
                finishProperty = true;
            }
        }

        // Find current value index in the slider values array
        int currentIndex = 0;
        for (size_t i = 0; i < sliderValues->size(); ++i) {
            if ((*sliderValues)[i] == newValue) {
                currentIndex = static_cast<int>(i);
                break;
            }
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);

        // Create format string with current value
        char formatStr[32];
        snprintf(formatStr, sizeof(formatStr), "%d", (*sliderValues)[currentIndex]);

        if (ImGui::SliderInt(("##intslider_"+id).c_str(), &currentIndex, 0, static_cast<int>(sliderValues->size() - 1), formatStr)) {
            int newSliderValue = (*sliderValues)[currentIndex];
            for (Entity& entity : entities){
                if (type == RowPropertyType::IntSlider) {
                    cmd = new PropertyCmd<int>(project, sceneProject->id, entity, cpType, id, newSliderValue, settings.onValueChanged);
                } else {
                    cmd = new PropertyCmd<unsigned int>(project, sceneProject->id, entity, cpType, id, static_cast<unsigned int>(newSliderValue), settings.onValueChanged);
                }
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }

        if (dif)
            ImGui::PopStyleColor();

    }else if (type == RowPropertyType::Enum && settings.enumEntries) {
        int* value = nullptr;
        std::map<Entity, int> eValue;
        bool dif = false;
        int* defArr = nullptr;
        std::vector<EnumEntry>* enumEntries = nullptr;
        for (Entity& entity : entities){
            PropertyData prop = Catalog::getProperty(sceneProject->scene, entity, cpType, id);
            defArr = static_cast<int*>(prop.def);
            enumEntries = settings.enumEntries;
            eValue[entity] = *static_cast<int*>(prop.ref);
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }

        int item_current = 0;
        // Find current index in enumEntries
        for (size_t i = 0; i < enumEntries->size(); ++i) {
            if ((*enumEntries)[i].value == *value) {
                item_current = static_cast<int>(i);
                break;
            }
        }
        int item_default = item_current;

        bool defChanged = false;
        if (defArr){
            int defValue = *defArr;
            // Find index of default value in enumEntries
            for (size_t i = 0; i < enumEntries->size(); ++i) {
                if ((*enumEntries)[i].value == defValue) {
                    item_default = static_cast<int>(i);
                    break;
                }
            }
            defChanged = (item_current != item_default);
        }
        if (propertyHeader(label, settings.secondColSize, defChanged, settings.child)){
            for (Entity& entity : entities){
                int defValue = (*enumEntries)[item_default].value;
                cmd = new PropertyCmd<int>(project, sceneProject->id, entity, cpType, id, defValue, settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
                finishProperty = true;
            }
        }

        // Build names array
        std::vector<const char*> names;
        for (const auto& entry : *enumEntries) {
            names.push_back(entry.name);
        }

        if (dif)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        if (ImGui::Combo(("##combo_" + id).c_str(), &item_current, names.data(), static_cast<int>(names.size()))) {
            int newValue = (*enumEntries)[item_current].value;
            for (Entity& entity : entities){
                cmd = new PropertyCmd<int>(project, sceneProject->id, entity, cpType, id, newValue, settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
        }
        if (dif)
            ImGui::PopStyleColor();

    }else if (type == RowPropertyType::Texture){
        Texture* value = nullptr;
        std::map<Entity, Texture> eValue;
        bool dif = false;
        Texture* defArr = nullptr;
        for (Entity& entity : entities){
            PropertyData prop = Catalog::getProperty(sceneProject->scene, entity, cpType, id);
            defArr = static_cast<Texture*>(prop.def);
            eValue[entity] = *static_cast<Texture*>(prop.ref);
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }

        Texture newValue = *value;

        bool defChanged = false;
        if (defArr){
            defChanged = (newValue != *defArr);
        }
        if (propertyHeader(label, settings.secondColSize, defChanged, settings.child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Texture>(project, sceneProject->id, entity, cpType, id, *defArr, settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
                finishProperty = true;
            }
        }

        ImGui::BeginGroup();
        ImGui::PushID(("texture_"+id).c_str());

        ImGui::PushStyleColor(ImGuiCol_ChildBg, textureLabel);

        float thumbSize = ImGui::GetFrameHeight() * 3;
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
                        cmd = new PropertyCmd<Texture>(project, sceneProject->id, entity, cpType, id, texture, settings.onValueChanged);
                        CommandHandle::get(sceneProject->id)->addCommand(cmd);
                        finishProperty = true;
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

        dragDropResources(cpType, id, sceneProject, entities, cpType);

    }else if (type == RowPropertyType::Material){
        Material* value = nullptr;
        std::map<Entity, Material> eValue;
        bool dif = false;
        Material* defArr = nullptr;
        for (Entity& entity : entities){
            PropertyData prop = Catalog::getProperty(sceneProject->scene, entity, cpType, id);
            defArr = static_cast<Material*>(prop.def);
            eValue[entity] = *static_cast<Material*>(prop.ref);
            if (value){
                if (*value != eValue[entity])
                    dif = true;
            }
            value = &eValue[entity];
        }
        Material newValue = *value;

        bool defChanged = false;
        if (defArr){
            defChanged = (newValue != *defArr);
        }
        if (propertyHeader(label, settings.secondColSize, defChanged, settings.child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<Material>(project, sceneProject->id, entity, cpType, id, *defArr, settings.onValueChanged);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
                finishProperty = true;
            }
        }

        ImGui::BeginGroup();

        Texture texRender = getMaterialPreview(newValue, id);
        float thumbSize = ImGui::GetFrameHeight() * 3;
        ImGui::Image(texRender.getRender()->getGLHandler(), ImVec2(thumbSize, thumbSize), ImVec2(0, 1), ImVec2(1, 0));
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
            ImGui::Image(texRender.getRender()->getGLHandler(), ImVec2(imageDragSize, imageDragSize), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::EndDragDropSource();
        }

        result = materialButtonGroups[id];

    }else if (type == RowPropertyType::EntityPointer){
        EntityRef* value = nullptr;
        bool different = false;
        std::map<Entity, EntityRef> eValue;
        EntityRef* defVal = nullptr;

        for (Entity& entity : entities){
            PropertyData prop = Catalog::getProperty(sceneProject->scene, entity, cpType, id);
            defVal = static_cast<EntityRef*>(prop.def);
            eValue[entity] = *static_cast<EntityRef*>(prop.ref);
            if (value){
                if (value->locator.sceneId != eValue[entity].locator.sceneId ||
                    value->locator.scopedEntity != eValue[entity].locator.scopedEntity ||
                    value->locator.sharedPath != eValue[entity].locator.sharedPath)
                    different = true;
            }
            value = &eValue[entity];
        }

        EntityRef newValue = *value;
        project->resolveEntityRef(newValue, sceneProject, entities.back());

        bool defChanged = false;
        if (defVal){
            defChanged = (newValue.entity != defVal->entity || newValue.scene != defVal->scene);
        }

        if (propertyHeader(label, settings.secondColSize, defChanged, settings.child)){
            for (Entity& entity : entities){
                cmd = new PropertyCmd<EntityRef>(project, sceneProject->id, entity, cpType, id, *defVal);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
                finishProperty = true;
            }
        }

        ImGui::BeginGroup();

        // Display entity name or "None"
        std::string entityName = "None";
        if (newValue.entity != NULL_ENTITY && sceneProject->scene->isEntityCreated(newValue.entity)) {
            entityName = sceneProject->scene->getEntityName(newValue.entity);
            if (entityName.empty()) {
                entityName = "Entity " + std::to_string(newValue.entity);
            }
        }

        if (different) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
            entityName = "---";
        }

        // Button to show entity (with icon)
        std::string buttonLabel = ICON_FA_CIRCLE_DOT " " + entityName + "##entity_" + id;
        float clearButtonFramePadding = 2;
        float clearButtonWidth = ImGui::CalcTextSize(ICON_FA_XMARK).x;
        ImVec2 buttonSize = ImVec2(ImGui::GetContentRegionAvail().x - clearButtonWidth - ImGui::GetStyle().ItemSpacing.x - clearButtonFramePadding* 2, 0);

        if (ImGui::Button(buttonLabel.c_str(), buttonSize)) {
            // Optional: Focus on the entity in the structure window
            if (newValue.entity != NULL_ENTITY) {
                project->clearSelectedEntities(sceneProject->id);
                project->addSelectedEntity(sceneProject->id, newValue.entity);
            }
        }

        // Handle drag and drop from Structure window
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("entity", ImGuiDragDropFlags_AcceptBeforeDelivery)) {
                if (payload->IsDelivery()) {
                    const EntityPayload* entityPayload = static_cast<const EntityPayload*>(payload->Data);
                    Entity droppedEntity = entityPayload->entity;

                    // Detect shared group of dropped entity
                    std::filesystem::path droppedGroupPath = project->findGroupPathFor(sceneProject->id, droppedEntity);
                    bool useShared = false;

                    if (!droppedGroupPath.empty()) {
                        // Only use shared reference if ALL selected entities belong to the SAME shared group
                        useShared = true;
                        for (Entity hostEntity : entities) {
                            if (project->findGroupPathFor(sceneProject->id, hostEntity) != droppedGroupPath) {
                                useShared = false;
                                break;
                            }
                        }
                    }

                    EntityRef newEntityRef;
                    if (useShared) {
                        SharedGroup* group = project->getSharedGroup(droppedGroupPath);
                        Entity registryEntity = group->getRegistryEntity(sceneProject->id, droppedEntity);
                        uint32_t instanceId = group->getInstanceId(sceneProject->id, droppedEntity);

                        newEntityRef.locator.kind = EntityRefKind::SharedEntity;
                        newEntityRef.locator.scopedEntity = registryEntity;
                        newEntityRef.locator.sharedPath = droppedGroupPath.generic_string();
                    } else {
                        // Normal scene entity reference
                        newEntityRef.locator.kind = EntityRefKind::LocalEntity;
                        newEntityRef.locator.scopedEntity = droppedEntity;
                        newEntityRef.locator.sceneId = sceneProject->id;
                    }

                    // Apply to all selected entities
                    for (Entity& entity : entities) {
                        cmd = new PropertyCmd<EntityRef>(project, sceneProject->id, entity, cpType, id, newEntityRef);
                        CommandHandle::get(sceneProject->id)->addCommand(cmd);
                    }
                    finishProperty = true;
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (different) {
            ImGui::PopStyleColor();
        }

        // Clear button (X)
        ImGui::SameLine();
        ImGui::BeginDisabled(newValue.entity == NULL_ENTITY);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(clearButtonFramePadding, ImGui::GetStyle().FramePadding.y));
        if (ImGui::Button((ICON_FA_XMARK "##clear_entity_" + id).c_str())) {
            for (Entity& entity : entities) {
                cmd = new PropertyCmd<EntityRef>(project, sceneProject->id, entity, cpType, id, *defVal);
                CommandHandle::get(sceneProject->id)->addCommand(cmd);
            }
            finishProperty = true;
        }
        ImGui::PopStyleVar();
        ImGui::EndDisabled();
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Clear entity reference");
        }

        ImGui::EndGroup();

    }

    if (ImGui::IsItemDeactivatedAfterEdit() || finishProperty) {
        cmd->setNoMerge();
        cmd = nullptr;
        finishProperty = false;
    }

    return result;
}

void Editor::Properties::drawTransform(ComponentType cpType, SceneProject* sceneProject, std::vector<Entity> entities){
    // Add this code to calculate appropriate step size based on selected scene
    RowSettings settingsPos;
    if (sceneProject) {
        Camera* camera = sceneProject->sceneRender->getCamera();
        if (sceneProject->sceneType == SceneType::SCENE_3D) {
            // For 3D scenes, scale step based on distance from target
            float distanceFromTarget = camera->getDistanceFromTarget();
            settingsPos.stepSize = std::max(0.01f, distanceFromTarget / 200.0f);
        } else {
            // For 2D scenes, use the zoom level
            SceneRender2D* sceneRender2D = static_cast<SceneRender2D*>(sceneProject->sceneRender);
            float zoom = sceneRender2D->getZoom();
            settingsPos.stepSize = std::max(0.01f, zoom * 1.0f);
        }
    }

    beginTable(cpType, getLabelSize("billboard"));

    propertyRow(RowPropertyType::Vector3, cpType, "position", "Position", sceneProject, entities, settingsPos);
    propertyRow(RowPropertyType::Quat, cpType, "rotation", "Rotation", sceneProject, entities);
    propertyRow(RowPropertyType::Vector3, cpType, "scale", "Scale", sceneProject, entities);
    propertyRow(RowPropertyType::Bool, cpType, "visible", "Visible", sceneProject, entities);
    propertyRow(RowPropertyType::Bool, cpType, "billboard", "Billboard", sceneProject, entities);

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_GEAR)){
        ImGui::OpenPopup("menusettings_billboard");
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(19 * ImGui::GetFontSize(), 0), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::BeginPopup("menusettings_billboard")){
        ImGui::Text("Billboard settings");
        ImGui::Separator();

        RowSettings settingsRotation;
        settingsRotation.secondColSize = 12 * ImGui::GetFontSize();

        beginTable(cpType, getLabelSize("cylindrical"));
        propertyRow(RowPropertyType::Bool, cpType, "fakeBillboard", "Fake", sceneProject, entities);
        propertyRow(RowPropertyType::Bool, cpType, "cylindricalBillboard", "Cylindrical", sceneProject, entities);
        propertyRow(RowPropertyType::Quat, cpType, "billboardRotation", "Rotation", sceneProject, entities, settingsRotation);
        endTable();

        ImGui::EndPopup();
    }

    endTable();
}

void Editor::Properties::drawMeshComponent(ComponentType cpType, SceneProject* sceneProject, std::vector<Entity> entities){
    beginTable(cpType, getLabelSize("receive shadows"));

    // Static variables for shape parameters
    static ShapeParameters shapeParams;

    // Add New Geometry property row
    propertyHeader("Geometry");
    if (ImGui::Button("Create Shape")){
        ImGui::OpenPopup("menusettings_shape_geometry");

        updateShapePreview(shapeParams);
    }

    // Geometry creation popup
    ImGui::SetNextWindowSizeConstraints(ImVec2(17 * ImGui::GetFontSize(), 0), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::BeginPopup("menusettings_shape_geometry")){
        ImGui::Text("Create Shape");
        ImGui::Separator();

        const char* geometryTypes[] = { "Plane", "Box", "Sphere", "Cylinder", "Capsule", "Torus" };

        beginTable(cpType, getLabelSize("Geometry Type"), "geometry_popup");

        // Geometry type selection
        propertyHeader("Geometry Type");

        float secondColSize = 8 * ImGui::GetFontSize();
        bool updatedPreview = false;

        Texture texRender = shapePreviewRender.getTexture();
        ImGui::Image(texRender.getRender()->getGLHandler(), ImVec2(secondColSize, secondColSize), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::SetNextItemWidth(-1);
        if (ImGui::Combo("##geometry_type", &shapeParams.geometryType, geometryTypes, IM_ARRAYSIZE(geometryTypes))) {
            updatedPreview = true;
        }

        // Show parameters based on selected geometry type
        switch (shapeParams.geometryType) {
            case 0: // Plane
                propertyHeader("Width", secondColSize);
                if (ImGui::DragFloat("##plane_width", &shapeParams.planeWidth, 0.1f, 0.1f, 100.0f, "%.2f")) {
                    updatedPreview = true;
                }

                propertyHeader("Depth", secondColSize);
                if (ImGui::DragFloat("##plane_depth", &shapeParams.planeDepth, 0.1f, 0.1f, 100.0f, "%.2f")) {
                    updatedPreview = true;
                }

                propertyHeader("Tiles", secondColSize);
                if (ImGui::DragInt("##plane_tiles", (int*)&shapeParams.planeTiles, 1, 1, 100)) {
                    updatedPreview = true;
                }
                break;

            case 1: // Box
                propertyHeader("Width", secondColSize);
                if (ImGui::DragFloat("##box_width", &shapeParams.boxWidth, 0.1f, 0.1f, 100.0f, "%.2f")) {
                    updatedPreview = true;
                }

                propertyHeader("Height", secondColSize);
                if (ImGui::DragFloat("##box_height", &shapeParams.boxHeight, 0.1f, 0.1f, 100.0f, "%.2f")) {
                    updatedPreview = true;
                }

                propertyHeader("Depth", secondColSize);
                if (ImGui::DragFloat("##box_depth", &shapeParams.boxDepth, 0.1f, 0.1f, 100.0f, "%.2f")) {
                    updatedPreview = true;
                }

                propertyHeader("Tiles", secondColSize);
                if (ImGui::DragInt("##box_tiles", (int*)&shapeParams.boxTiles, 1, 1, 100)) {
                    updatedPreview = true;
                }
                break;

            case 2: // Sphere
                propertyHeader("Radius", secondColSize);
                if (ImGui::DragFloat("##sphere_radius", &shapeParams.sphereRadius, 0.1f, 0.1f, 100.0f, "%.2f")) {
                    updatedPreview = true;
                }

                propertyHeader("Slices", secondColSize);
                if (ImGui::DragInt("##sphere_slices", (int*)&shapeParams.sphereSlices, 1, 3, 100)) {
                    updatedPreview = true;
                }

                propertyHeader("Stacks", secondColSize);
                if (ImGui::DragInt("##sphere_stacks", (int*)&shapeParams.sphereStacks, 1, 3, 100)) {
                    updatedPreview = true;
                }
                break;

            case 3: // Cylinder
                propertyHeader("Base Radius", secondColSize);
                if (ImGui::DragFloat("##cylinder_base_radius", &shapeParams.cylinderBaseRadius, 0.1f, 0.1f, 100.0f, "%.2f")) {
                    updatedPreview = true;
                }

                propertyHeader("Top Radius", secondColSize);
                if (ImGui::DragFloat("##cylinder_top_radius", &shapeParams.cylinderTopRadius, 0.1f, 0.1f, 100.0f, "%.2f")) {
                    updatedPreview = true;
                }

                propertyHeader("Height", secondColSize);
                if (ImGui::DragFloat("##cylinder_height", &shapeParams.cylinderHeight, 0.1f, 0.1f, 100.0f, "%.2f")) {
                    updatedPreview = true;
                }

                propertyHeader("Slices", secondColSize);
                if (ImGui::DragInt("##cylinder_slices", (int*)&shapeParams.cylinderSlices, 1, 3, 100)) {
                    updatedPreview = true;
                }

                propertyHeader("Stacks", secondColSize);
                if (ImGui::DragInt("##cylinder_stacks", (int*)&shapeParams.cylinderStacks, 1, 1, 100)) {
                    updatedPreview = true;
                }
                break;

            case 4: // Capsule
                propertyHeader("Base Radius", secondColSize);
                if (ImGui::DragFloat("##capsule_base_radius", &shapeParams.capsuleBaseRadius, 0.1f, 0.1f, 100.0f, "%.2f")) {
                    updatedPreview = true;
                }

                propertyHeader("Top Radius", secondColSize);
                if (ImGui::DragFloat("##capsule_top_radius", &shapeParams.capsuleTopRadius, 0.1f, 0.1f, 100.0f, "%.2f")) {
                    updatedPreview = true;
                }

                propertyHeader("Height", secondColSize);
                if (ImGui::DragFloat("##capsule_height", &shapeParams.capsuleHeight, 0.1f, 0.1f, 100.0f, "%.2f")) {
                    updatedPreview = true;
                }

                propertyHeader("Slices", secondColSize);
                if (ImGui::DragInt("##capsule_slices", (int*)&shapeParams.capsuleSlices, 1, 3, 100)) {
                    updatedPreview = true;
                }

                propertyHeader("Stacks", secondColSize);
                if (ImGui::DragInt("##capsule_stacks", (int*)&shapeParams.capsuleStacks, 1, 1, 100)) {
                    updatedPreview = true;
                }
                break;

            case 5: // Torus
                propertyHeader("Radius", secondColSize);
                if (ImGui::DragFloat("##torus_radius", &shapeParams.torusRadius, 0.1f, 0.1f, 100.0f, "%.2f")) {
                    updatedPreview = true;
                }

                propertyHeader("Ring Radius", secondColSize);
                if (ImGui::DragFloat("##torus_ring_radius", &shapeParams.torusRingRadius, 0.1f, 0.1f, 100.0f, "%.2f")) {
                    updatedPreview = true;
                }

                propertyHeader("Sides", secondColSize);
                if (ImGui::DragInt("##torus_sides", (int*)&shapeParams.torusSides, 1, 3, 100)) {
                    updatedPreview = true;
                }

                propertyHeader("Rings", secondColSize);
                if (ImGui::DragInt("##torus_rings", (int*)&shapeParams.torusRings, 1, 3, 100)) {
                    updatedPreview = true;
                }
                break;
        }

        if (updatedPreview){
            updateShapePreview(shapeParams);
        }

        endTable();

        ImGui::Separator();

        // Create geometry button
        if (ImGui::Button("Apply", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            for (Entity& entity : entities) {
                std::shared_ptr<Supernova::MeshSystem> meshSys = sceneProject->scene->getSystem<MeshSystem>();
                MeshComponent meshComp = sceneProject->scene->getComponent<MeshComponent>(entity);

                updateMeshShape(meshComp, meshSys.get(), shapeParams);

                CommandHandle::get(sceneProject->id)->addCommandNoMerge(new MeshChangeCmd(project, sceneProject->id, entities[0], meshComp));
            }

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    propertyRow(RowPropertyType::Bool, cpType, "castShadows", "Cast Shadows", sceneProject, entities);
    propertyRow(RowPropertyType::Bool, cpType, "receiveShadows", "Receive Shadows", sceneProject, entities);

    endTable();

    unsigned int numSubmeshes = sceneProject->scene->getComponent<MeshComponent>(entities[0]).numSubmeshes;
    for (Entity& entity : entities){
        numSubmeshes = std::min(numSubmeshes, sceneProject->scene->getComponent<MeshComponent>(entity).numSubmeshes);
    }

    for (int s = 0; s < numSubmeshes; s++){
        ImGui::SeparatorText(("Submesh "+std::to_string(s+1)).c_str());

        float submeshesTableSize = getLabelSize("Texture Shadow");

        beginTable(cpType, submeshesTableSize, "submeshes");

        if (propertyRow(RowPropertyType::Material, cpType, "submeshes["+std::to_string(s)+"].material", "Material", sceneProject, entities)){

            RowSettings settingsFactor;
            settingsFactor.stepSize = 0.01f;
            settingsFactor.secondColSize = 4 * ImGui::GetFontSize();
            settingsFactor.child = true;

            RowSettings settingsMaterial;
            settingsMaterial.child = true;

            endTable();
            beginTable(cpType, getLabelSize("Met. Roug. Texture"), "material_table");
            propertyRow(RowPropertyType::Color4L, cpType, "submeshes["+std::to_string(s)+"].material.baseColor", "Base Color", sceneProject, entities, settingsMaterial);
            propertyRow(RowPropertyType::Texture, cpType, "submeshes["+std::to_string(s)+"].material.baseColorTexture", "Base Texture", sceneProject, entities,settingsMaterial);
            propertyRow(RowPropertyType::Float_0_1, cpType, "submeshes["+std::to_string(s)+"].material.metallicFactor", "Metallic Factor", sceneProject, entities, settingsFactor);
            propertyRow(RowPropertyType::Float_0_1, cpType, "submeshes["+std::to_string(s)+"].material.roughnessFactor", "Roughness Factor", sceneProject, entities, settingsFactor);
            propertyRow(RowPropertyType::Texture, cpType, "submeshes["+std::to_string(s)+"].material.metallicRoughnessTexture", "Met. Roug. Texture", sceneProject, entities, settingsMaterial);
            propertyRow(RowPropertyType::Color3L, cpType, "submeshes["+std::to_string(s)+"].material.emissiveFactor", "Emissive Factor", sceneProject, entities, settingsMaterial);
            propertyRow(RowPropertyType::Texture, cpType, "submeshes["+std::to_string(s)+"].material.emissiveTexture", "Emissive Texture", sceneProject, entities, settingsMaterial);
            propertyRow(RowPropertyType::Texture, cpType, "submeshes["+std::to_string(s)+"].material.occlusionTexture", "Occlusion Texture", sceneProject, entities, settingsMaterial);
            propertyRow(RowPropertyType::Texture, cpType, "submeshes["+std::to_string(s)+"].material.normalTexture", "Normal Texture", sceneProject, entities, settingsMaterial);
            endTable();
            beginTable(cpType, submeshesTableSize, "submeshes");
        }

        RowSettings settingsPrimitive;
        settingsPrimitive.enumEntries = &entriesPrimitiveType;

        propertyRow(RowPropertyType::Bool, cpType, "submeshes["+std::to_string(s)+"].faceCulling", "Face Culling", sceneProject, entities);
        propertyRow(RowPropertyType::Bool, cpType, "submeshes["+std::to_string(s)+"].textureShadow", "Texture Shadow", sceneProject, entities);
        propertyRow(RowPropertyType::Enum, cpType, "submeshes["+std::to_string(s)+"].primitiveType", "Primitive", sceneProject, entities, settingsPrimitive);
        propertyRow(RowPropertyType::Vector4, cpType, "submeshes["+std::to_string(s)+"].textureRect", "Texture Rect", sceneProject, entities);

        endTable();
    }
}

void Editor::Properties::drawUIComponent(ComponentType cpType, SceneProject* sceneProject, std::vector<Entity> entities){
    beginTable(cpType, getLabelSize("Texture"));
    propertyRow(RowPropertyType::Color4L, cpType, "color", "Color", sceneProject, entities);
    propertyRow(RowPropertyType::Texture, cpType, "texture", "Texture", sceneProject, entities);
    endTable();
}

void Editor::Properties::drawUILayoutComponent(ComponentType cpType, SceneProject* sceneProject, std::vector<Entity> entities){
    RowSettings settings;
    settings.stepSize = 1.0f;
    settings.secondColSize = 6 * ImGui::GetFontSize();

    beginTable(cpType, getLabelSize("Ignore Scissor"));
    propertyRow(RowPropertyType::UInt, cpType, "width", "Width", sceneProject, entities, settings);
    propertyRow(RowPropertyType::UInt, cpType, "height", "Height", sceneProject, entities, settings);
    propertyRow(RowPropertyType::Bool, cpType, "ignoreScissor", "Ignore Scissor", sceneProject, entities);
    endTable();
}

void Editor::Properties::drawImageComponent(ComponentType cpType, SceneProject* sceneProject, std::vector<Entity> entities){
    ImGui::SeparatorText("Nine-patch rect");

    if (entities.size() == 1) {
        if (UIComponent* ui = sceneProject->scene->findComponent<UIComponent>(entities[0])){
            Texture* thumbTexture = findThumbnail(ui->texture.getId());
            if (thumbTexture) {
                drawNinePatchesPreview(sceneProject->scene->getComponent<ImageComponent>(entities[0]), &ui->texture, thumbTexture, ImVec2(THUMBNAIL_SIZE, THUMBNAIL_SIZE));
            }
        }
    }

    RowSettings settingsInt;
    settingsInt.stepSize = 1.0f;
    settingsInt.secondColSize = 6 * ImGui::GetFontSize();

    RowSettings settingsTexScale;
    settingsTexScale.secondColSize = 6 * ImGui::GetFontSize();
    settingsTexScale.help = "Increase or decrease texture area by a factor";

    beginTable(cpType, getLabelSize("Margin Bottom"), "nine_margin_table");
    propertyRow(RowPropertyType::UInt, cpType, "patchMarginLeft", "Margin Left", sceneProject, entities, settingsInt);
    propertyRow(RowPropertyType::UInt, cpType, "patchMarginRight", "Margin Right", sceneProject, entities, settingsInt);
    propertyRow(RowPropertyType::UInt, cpType, "patchMarginTop", "Margin Top", sceneProject, entities, settingsInt);
    propertyRow(RowPropertyType::UInt, cpType, "patchMarginBottom", "Margin Bottom", sceneProject, entities, settingsInt);
    endTable();

    beginTable(cpType, getLabelSize("Texture Scale"));
    propertyRow(RowPropertyType::Float, cpType, "textureScaleFactor", "Texture Scale", sceneProject, entities, settingsTexScale);
    endTable();
}

void Editor::Properties::drawSpriteComponent(ComponentType cpType, SceneProject* sceneProject, std::vector<Entity> entities){
    RowSettings settingsInt;
    settingsInt.stepSize = 1.0f;
    settingsInt.secondColSize = 6 * ImGui::GetFontSize();

    RowSettings settingsTexScale;
    settingsTexScale.secondColSize = 6 * ImGui::GetFontSize();
    settingsTexScale.help = "Increase or decrease texture area by a factor";

    RowSettings settingsPivot;
    settingsPivot.enumEntries = &entriesPivotPreset;

    beginTable(cpType, getLabelSize("Texture Scale"));
    propertyRow(RowPropertyType::UInt, cpType, "width", "Width", sceneProject, entities, settingsInt);
    propertyRow(RowPropertyType::UInt, cpType, "height", "Height", sceneProject, entities, settingsInt);
    propertyRow(RowPropertyType::Enum, cpType, "pivotPreset", "Pivot", sceneProject, entities, settingsPivot);
    propertyRow(RowPropertyType::Float, cpType, "textureScaleFactor", "Texture Scale", sceneProject, entities, settingsTexScale);
    endTable();
}

void Editor::Properties::drawLightComponent(ComponentType cpType, SceneProject* sceneProject, std::vector<Entity> entities){
    LightComponent& light = sceneProject->scene->getComponent<LightComponent>(entities[0]);

    RowSettings settingsFloat;
    settingsFloat.secondColSize = 6 * ImGui::GetFontSize();

    RowSettings settingsCone;
    settingsCone.secondColSize = 6 * ImGui::GetFontSize();
    settingsCone.format = "%.1f";

    RowSettings settingsLightType;
    settingsLightType.enumEntries = &entriesLightType;

    beginTable(cpType, getLabelSize("Inner cone"));
    propertyRow(RowPropertyType::Enum, cpType, "type", "Type", sceneProject, entities, settingsLightType);
    propertyRow(RowPropertyType::Float, cpType, "intensity", "Intensity", sceneProject, entities, settingsFloat);
    propertyRow(RowPropertyType::Float, cpType, "range", "Range", sceneProject, entities, settingsFloat);
    propertyRow(RowPropertyType::Color3L, cpType, "color", "Color", sceneProject, entities);
    if (light.type != LightType::POINT){
        propertyRow(RowPropertyType::Direction, cpType, "direction", "Direction", sceneProject, entities);
    }
    if (light.type == LightType::SPOT){
        propertyRow(RowPropertyType::HalfCone, cpType, "innerConeCos", "Inner Cone", sceneProject, entities, settingsCone);
        propertyRow(RowPropertyType::HalfCone, cpType, "outerConeCos", "Outer Cone", sceneProject, entities, settingsCone);
    }
    endTable();

    ImGui::SeparatorText("Shadow settings");

    RowSettings settingsBias;
    settingsBias.stepSize = 0.000001f;
    settingsBias.secondColSize = 6 * ImGui::GetFontSize();
    settingsBias.format = "%.6f";

    RowSettings settingsMapRes;
    settingsMapRes.sliderValues = &po2Values;

    RowSettings settingsCascade;
    settingsCascade.sliderValues = &cascadeValues;

    beginTable(cpType, getLabelSize("Map Resolution"), "shadow_settings_table");
    propertyRow(RowPropertyType::Bool, cpType, "shadows", "Enabled", sceneProject, entities);
    propertyRow(RowPropertyType::Float, cpType, "shadowBias", "Bias", sceneProject, entities, settingsBias);
    propertyRow(RowPropertyType::UIntSlider, cpType, "mapResolution", "Map Resolution", sceneProject, entities, settingsMapRes);
    propertyRow(RowPropertyType::UIntSlider, cpType, "numShadowCascades", "Num Cascades", sceneProject, entities, settingsCascade);

    propertyHeader("Shadow Camera");
    if (ImGui::Button(ICON_FA_GEAR)){
        ImGui::OpenPopup("menusettings_shadow_camera");
    }

    ImGui::SetNextWindowSizeConstraints(ImVec2(14 * ImGui::GetFontSize(), 0), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::BeginPopup("menusettings_shadow_camera")){
        ImGui::Text("Shadow camera settings");
        ImGui::Separator();

        RowSettings settingsFloat;
        settingsFloat.secondColSize = 6 * ImGui::GetFontSize();

        beginTable(cpType, getLabelSize("Camera Near"), "shadow_camera_popup");

        propertyRow(RowPropertyType::Bool, cpType, "automaticShadowCamera", "Automatic", sceneProject, entities);
        ImGui::BeginDisabled(light.automaticShadowCamera);
        propertyRow(RowPropertyType::Float, cpType, "shadowCameraNear", "Camera Near", sceneProject, entities, settingsFloat);
        propertyRow(RowPropertyType::Float, cpType, "shadowCameraFar", "Camera Far", sceneProject, entities, settingsFloat);
        ImGui::EndDisabled();

        endTable();

        ImGui::EndPopup();
    }
    endTable();
}

void Editor::Properties::drawScriptComponent(ComponentType cpType, SceneProject* sceneProject, std::vector<Entity> entities){
    ScriptComponent& scriptComp = sceneProject->scene->getComponent<ScriptComponent>(entities[0]);

    // Check if there's already a SUBCLASS script
    bool hasSubclass = false;
    for (const auto& script : scriptComp.scripts) {
        if (script.type == ScriptType::SUBCLASS) {
            hasSubclass = true;
            break;
        }
    }

    if (ImGui::Button(ICON_FA_FILE_CIRCLE_PLUS " New Script", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0))) {
        std::string defaultName = "NewScript";
        scriptCreateDialog.open(project->getProjectPath(), defaultName, hasSubclass,
            [this, sceneProject, entities, cpType](const std::filesystem::path& headerPath,
                                                const std::filesystem::path& sourcePath,
                                                const std::string& className,
                                                ScriptType type){
                std::string pathStr = sourcePath.string();
                std::string headerPathStr = headerPath.string();

                for (Entity entity: entities){
                    // Get current scripts vector
                    ScriptComponent& scriptComp = sceneProject->scene->getComponent<ScriptComponent>(entity);
                    std::vector<ScriptEntry> newScripts = scriptComp.scripts;

                    // Create new entry
                    ScriptEntry entry;
                    entry.type = type;
                    entry.path = pathStr;
                    entry.headerPath = headerPathStr;
                    entry.className = className;
                    entry.enabled = true;

                    // Insert SUBCLASS at beginning, SCRIPT_CLASS at end
                    if (type == ScriptType::SUBCLASS) {
                        newScripts.insert(newScripts.begin(), entry);
                    } else {
                        newScripts.push_back(entry);
                    }

                    project->updateScriptProperties(sceneProject, entity, newScripts);

                    // Use PropertyCmd to update the scripts vector
                    cmd = new PropertyCmd<std::vector<ScriptEntry>>(project, sceneProject->id, entity, ComponentType::ScriptComponent, "scripts", newScripts);
                    CommandHandle::get(sceneProject->id)->addCommand(cmd);
                }
                cmd->setNoMerge();
            },
            [](){}
        );
    }

    // Display all scripts
    if (entities.size() == 1) {
        // Sort scripts to ensure SUBCLASS comes first
        std::vector<size_t> scriptIndices;
        for (size_t i = 0; i < scriptComp.scripts.size(); i++) {
            scriptIndices.push_back(i);
        }

        // Sort indices: SUBCLASS first, then SCRIPT_CLASS
        std::sort(scriptIndices.begin(), scriptIndices.end(), [&](size_t a, size_t b) {
            if (scriptComp.scripts[a].type == scriptComp.scripts[b].type) {
                return a < b; // Maintain original order within same type
            }
            return scriptComp.scripts[a].type == ScriptType::SUBCLASS;
        });

        bool removedScriptThisFrame = false;

        for (size_t idx : scriptIndices) {
            if (removedScriptThisFrame) break; // Avoid using invalidated references after removal

            size_t scriptIdx = idx;
            if (scriptIdx >= scriptComp.scripts.size()) continue; // Safety
            ScriptEntry& script = scriptComp.scripts[scriptIdx];

            ImGui::PushID(scriptIdx);

            std::string scriptLabel = script.className.empty() ? "Unnamed Script" : script.className;
            std::string typeLabel = (script.type == ScriptType::SUBCLASS) ? " [Subclass]" : "";

            const float indentation = 10.0f;

            // Indent to show scripts are nested inside ScriptComponent
            ImGui::Indent(indentation);

            // Custom styling for script headers
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.3f, 0.35f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.35f, 0.35f, 0.4f, 1.0f));

            // Add icon to distinguish from component headers
            std::string headerText = ICON_FA_FILE_CODE " " + scriptLabel + typeLabel;

            bool headerOpen = ImGui::CollapsingHeader(headerText.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

            // Right-click menu on the script header
            if (ImGui::BeginPopupContextItem(("script_options_menu_" + std::to_string(scriptIdx)).c_str())) {
                ImGui::TextDisabled("Script options");
                ImGui::Separator();

                if (ImGui::MenuItem(ICON_FA_TRASH " Remove")) {
                    // Remove this script entry
                    std::vector<ScriptEntry> newScripts = scriptComp.scripts;
                    if (scriptIdx < newScripts.size()) {
                        newScripts.erase(newScripts.begin() + scriptIdx);

                        // Refresh parsed properties for remaining scripts
                        project->updateScriptProperties(sceneProject, entities[0], newScripts);

                        // Apply change through command system
                        cmd = new PropertyCmd<std::vector<ScriptEntry>>(project, sceneProject->id, entities[0], ComponentType::ScriptComponent, "scripts", newScripts);
                        CommandHandle::get(sceneProject->id)->addCommand(cmd);
                        cmd->setNoMerge();

                        removedScriptThisFrame = true;
                    }
                }

                ImGui::EndPopup();
            }

            if (headerOpen && !removedScriptThisFrame) {
                ImGui::Unindent(indentation); // Unindent for content

                beginTable(cpType, getLabelSize("Script Path"), "script_" + std::to_string(scriptIdx));

                // Path and enabled status
                propertyHeader("Path");

                // Convert absolute path to relative path from project directory
                std::string displayPath = script.path;
                std::filesystem::path scriptPath = script.path;
                std::filesystem::path projectPath = project->getProjectPath();

                if (scriptPath.is_absolute()) {
                    std::error_code ec;
                    auto relativePath = std::filesystem::relative(scriptPath, projectPath, ec);
                    if (!ec && relativePath.string().find("..") == std::string::npos) {
                        displayPath = relativePath.string();
                    }
                }

                ImGui::Text("%s", displayPath.c_str());
                if (ImGui::IsItemHovered() && displayPath != script.path) {
                    ImGui::SetTooltip("%s", script.path.c_str());
                }

                propertyHeader("Enabled");
                ImGui::Checkbox("##enabled", &script.enabled);

                endTable();

                // Display script properties if available
                if (!script.properties.empty()) {
                    ImGui::SeparatorText("Properties");

                    beginTable(cpType, getLabelSize("Property Name"), "script_properties_" + std::to_string(scriptIdx));

                    for (size_t propIdx = 0; propIdx < script.properties.size(); propIdx++) {
                        ScriptProperty& prop = script.properties[propIdx];

                        std::string propertyId = "script[" + std::to_string(scriptIdx) + "]." + prop.name;
                        std::string displayName = prop.displayName.empty() ? prop.name : prop.displayName;

                        RowPropertyType propType = scriptPropertyTypeToRowPropertyType(prop.type);

                        RowSettings propSettings;
                        propSettings.onValueChanged = [&prop]() {
                            prop.syncToMember();
                        };

                        propertyRow(propType, cpType, propertyId, displayName, sceneProject, entities, propSettings);
                    }

                    endTable();
                }

                ImGui::Indent(indentation); // Re-indent after content
            }

            ImGui::PopStyleColor(3);
            ImGui::Unindent(indentation);

            ImGui::PopID();
        }
    } else {
        ImGui::TextDisabled("Select a single entity to view script details");
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
        std::filesystem::path sharedGroupPath;
        std::string names;
        bool isFirstEntity = true;
        for (Entity& entity : entities){
            std::vector<ComponentType> newComponents = Catalog::findComponents(scene, entity);

            if (!std::is_sorted(newComponents.begin(), newComponents.end())) {
                std::sort(newComponents.begin(), newComponents.end());
            }

            std::filesystem::path newSharedGroupPath = project->findGroupPathFor(sceneProject->id, entity);
            sharedGroupPath = project->findGroupPathFor(sceneProject->id, entity);

            if (isFirstEntity) {
                components = newComponents;
                sharedGroupPath = newSharedGroupPath;
                isFirstEntity = false;
            } else {
                std::vector<ComponentType> intersection;
                intersection.reserve(std::min(components.size(), newComponents.size()));

                std::set_intersection(
                    components.begin(), components.end(),
                    newComponents.begin(), newComponents.end(),
                    std::back_inserter(intersection));

                components = std::move(intersection);

                if (sharedGroupPath != newSharedGroupPath) {
                    sharedGroupPath.clear(); // Different groups, so no shared group
                }

                names += ", ";
            }

            names += scene->getEntityName(entity);
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
                    CommandHandle::get(sceneProject->id)->addCommandNoMerge(new EntityNameCmd(project, sceneProject->id, entities[0], nameBuffer));
                }
            }
        }

        ImGui::Separator();

        bool isReadOnlyComponents = false;
        ImGui::BeginDisabled(isReadOnlyComponents);
        if (ImGui::Button(ICON_FA_PLUS" New component", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            componentAddDialog.open(
                [this, sceneProject, entities](ComponentType cpType) {
                    // Add component to all selected entities
                    for (const Entity& entity : entities) {
                        cmd = new AddComponentCmd(project, sceneProject->id, entity, cpType);
                        CommandHandle::get(sceneProject->id)->addCommand(cmd);
                    }
                    cmd->setNoMerge();

                    // Mark scene as modified
                    sceneProject->isModified = true;
                },
                [this, sceneProject, entities](ComponentType cpType) {
                    // Check if component can be added to all selected entities
                    for (const Entity& entity : entities) {
                        if (!canAddComponent(sceneProject, entity, cpType)) {
                            return false;
                        }
                    }
                    return true;
                },
                [](){}
            );
        }
        ImGui::EndDisabled();

        // Show the component add dialog
        componentAddDialog.show();

        bool isShared = !sharedGroupPath.empty();

        for (ComponentType& cpType : components){

            // Check if this component is overridden for shared entities
            bool isComponentOverridden = false;
            SharedGroup* sharedGroup = nullptr;
            if (isShared) {
                sharedGroup = project->getSharedGroup(sharedGroupPath);
                if (sharedGroup) {
                    for (Entity& entity : entities) {
                        if (sharedGroup->hasComponentOverride(sceneProject->id, entity, cpType)) {
                            // If any entity does have an override, treat as overridden
                            isComponentOverridden = true;
                            break;
                        }
                        isComponentOverridden = false;
                    }
                }
            }

            // Create header with appropriate styling
            ImGui::PushID(static_cast<int>(cpType));

            // Build header text with icon
            std::string headerText;

            // Only apply special styling and icons for shared (non-overridden) components
            if (isShared && !isComponentOverridden) {
                // Shared components - blue color with link icon
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.6f, 1.0f, 1.0f));
                headerText = ICON_FA_LINK " ";
            }
            headerText += Catalog::getComponentName(cpType);

            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            bool headerOpen = ImGui::CollapsingHeader(headerText.c_str());

            if (isShared && !isComponentOverridden) {
                ImGui::PopStyleColor();
            }

            // Context menu disabled while playing
            bool compReadOnly = false;
            handleComponentMenu(sceneProject, entities, cpType, isShared, isComponentOverridden, headerOpen, compReadOnly);

            // Add hover tooltip only for shared components
            if (isShared && !isComponentOverridden && ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::TextColored(ImVec4(0.4f, 0.6f, 1.0f, 1.0f), ICON_FA_LINK " Shared Component");
                ImGui::Text("This component is shared across all instances.");
                ImGui::EndTooltip();
            }

            ImGui::PopID();

            if (headerOpen){
                // Disable non-script components while playing
                if (compReadOnly) ImGui::BeginDisabled(true);

                if (cpType == ComponentType::Transform){
                    drawTransform(cpType, sceneProject, entities);
                }else if (cpType == ComponentType::MeshComponent){
                    drawMeshComponent(cpType, sceneProject, entities);
                }else if (cpType == ComponentType::UIComponent){
                    drawUIComponent(cpType, sceneProject, entities);
                }else if (cpType == ComponentType::UILayoutComponent){
                    drawUILayoutComponent(cpType, sceneProject, entities);
                }else if (cpType == ComponentType::ImageComponent){
                    drawImageComponent(cpType, sceneProject, entities);
                }else if (cpType == ComponentType::SpriteComponent){
                    drawSpriteComponent(cpType, sceneProject, entities);
                }else if (cpType == ComponentType::LightComponent){
                    drawLightComponent(cpType, sceneProject, entities);
                }else if (cpType == ComponentType::ScriptComponent){
                    // ScriptComponent remains editable while playing
                    drawScriptComponent(cpType, sceneProject, entities);
                }

                if (compReadOnly) ImGui::EndDisabled();
            }
        }

    }else{
        thumbnailTextures.clear();
    }

    // Clean up unused material renders
    for (auto it = materialRenders.begin(); it != materialRenders.end(); ) {
        if (usedPreviewIds.find(it->first) == usedPreviewIds.end()) {
            if (!Engine::isSceneRunning(it->second.getScene())){
                it = materialRenders.erase(it);
            }
        } else {
            ++it;
        }
    }

    // Clean up unused direction renders
    for (auto it = directionRenders.begin(); it != directionRenders.end(); ) {
        if (usedPreviewIds.find(it->first) == usedPreviewIds.end()) {
            if (!Engine::isSceneRunning(it->second.getScene())){
                it = directionRenders.erase(it);
            }
        } else {
            ++it;
        }
    }
    usedPreviewIds.clear();

    scriptCreateDialog.show();

    ImGui::End();
}