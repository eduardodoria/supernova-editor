#include "SceneRender3D.h"

#include "resources/sky/Daylight_Box_Back_png.h"
#include "resources/sky/Daylight_Box_Bottom_png.h"
#include "resources/sky/Daylight_Box_Front_png.h"
#include "resources/sky/Daylight_Box_Left_png.h"
#include "resources/sky/Daylight_Box_Right_png.h"
#include "resources/sky/Daylight_Box_Top_png.h"
#include "resources/icons/sun-icon_png.h"
#include "resources/icons/bulb-icon_png.h"
#include "resources/icons/spot-icon_png.h"

#include "Project.h"

using namespace Supernova;

Editor::SceneRender3D::SceneRender3D(Scene* scene): SceneRender(scene, false, true, 40.0, 0.01){
    linesOffset = Vector2(0, 0);

    lines = new Lines(scene);
    sky = new SkyBox(scene);

    TextureData skyBack;
    TextureData skyBottom;
    TextureData skyFront;
    TextureData skyLeft;
    TextureData skyRight;
    TextureData skyTop;

    skyBack.loadTextureFromMemory(Daylight_Box_Back_png, Daylight_Box_Back_png_len);
    skyBottom.loadTextureFromMemory(Daylight_Box_Bottom_png, Daylight_Box_Bottom_png_len);
    skyFront.loadTextureFromMemory(Daylight_Box_Front_png, Daylight_Box_Front_png_len);
    skyLeft.loadTextureFromMemory(Daylight_Box_Left_png, Daylight_Box_Left_png_len);
    skyRight.loadTextureFromMemory(Daylight_Box_Right_png, Daylight_Box_Right_png_len);
    skyTop.loadTextureFromMemory(Daylight_Box_Top_png, Daylight_Box_Top_png_len);

    sky->setTextures("editor:resources:default_sky", skyBack, skyFront, skyLeft, skyRight, skyTop, skyBottom);

    lightObjects.clear();

    createLines();

    //camera->setType(CameraType::CAMERA_2D);
    camera->setPosition(10, 4, 10);
    camera->setTarget(0, 0, 0);

    //camera->setRenderToTexture(true);
    //camera->setUseFramebufferSizes(false);

    scene->setLightState(LightState::ON);
    scene->setGlobalIllumination(0.2);
    scene->setBackgroundColor(Vector4(0.25, 0.45, 0.65, 1.0));

    uilayer.setViewportGizmoTexture(viewgizmo.getFramebuffer());

    Engine::setScalingMode(Scaling::NATIVE);
    Engine::setFixedTimeSceneUpdate(false);
}

Editor::SceneRender3D::~SceneRender3D(){
    delete lines;
    delete sky;
    delete selLines;

    for (auto& pair : lightObjects) {
        delete pair.second.icon;
        delete pair.second.lines;
    }
    lightObjects.clear();
}

void Editor::SceneRender3D::createLines(){
    int gridHeight = 0;
    int gridSize = camera->getFarClip() * 2;

    int xGridStart = -gridSize + linesOffset.x;
    int xGridEnd = gridSize + linesOffset.x;

    int yGridStart = -gridSize + linesOffset.y;
    int yGridEnd = gridSize + linesOffset.y;

    lines->clearLines();

    for (int i = xGridStart; i <= xGridEnd; i++){
        if (i == 0){
            lines->addLine(Vector3(i, gridHeight, yGridStart), Vector3(i, gridHeight, yGridEnd), Vector4(0.5, 0.5, 1.0, 1.0));
        }else{
            if (i % 10 == 0){
                lines->addLine(Vector3(i, gridHeight, yGridStart), Vector3(i, gridHeight, yGridEnd), Vector4(0.5, 0.5, 0.5, 1.0));
            }else{
                lines->addLine(Vector3(i, gridHeight, yGridStart), Vector3(i, gridHeight, yGridEnd), Vector4(0.5, 0.5, 0.5, 0.5));
            }
        }
    }
    for (int i = yGridStart; i <= yGridEnd; i++){
        if (i == 0){
            lines->addLine(Vector3(xGridStart, gridHeight, i), Vector3(xGridEnd, gridHeight, i), Vector4(1.0, 0.5, 0.5, 1.0));
        }else{
            if (i % 10 == 0){
                lines->addLine(Vector3(xGridStart, gridHeight, i), Vector3(xGridEnd, gridHeight, i), Vector4(0.5, 0.5, 0.5, 1.0));
            }else{
                lines->addLine(Vector3(xGridStart, gridHeight, i), Vector3(xGridEnd, gridHeight, i), Vector4(0.5, 0.5, 0.5, 0.5));
            }
        }
    }
    lines->addLine(Vector3(0, -gridSize, 0), Vector3(0, gridSize, 0), Vector4(0.5, 1.0, 0.5, 1.0));
}

bool Editor::SceneRender3D::instanciateLightObject(Entity entity){
    if (lightObjects.find(entity) == lightObjects.end()) {
        lightObjects[entity].icon = new Sprite(scene);
        lightObjects[entity].lines = new Lines(scene);

        return true;
    }

    return false;
}

void Editor::SceneRender3D::createOrUpdateLightIcon(Entity entity, const Transform& transform, LightType lightType, bool newLight) {
    LightObjects& lo = lightObjects[entity];

    if (newLight) {
        TextureData iconData;
        if (lightType == LightType::DIRECTIONAL) {
            iconData.loadTextureFromMemory(sun_icon_png, sun_icon_png_len);
            lo.icon->setTexture("editor:resources:sun_icon", iconData);
        } else if (lightType == LightType::POINT) {
            iconData.loadTextureFromMemory(bulb_icon_png, bulb_icon_png_len);
            lo.icon->setTexture("editor:resources:bulb_icon", iconData);
        } else if (lightType == LightType::SPOT) {
            iconData.loadTextureFromMemory(spot_icon_png, spot_icon_png_len);
            lo.icon->setTexture("editor:resources:spot_icon", iconData);
        }

        lo.icon->setBillboard(true);
        lo.icon->setSize(128, 128);
        lo.icon->setReceiveLights(false);
        lo.icon->setCastShadows(false);
        lo.icon->setReceiveShadows(false);
        lo.icon->setPivotPreset(PivotPreset::CENTER);
    }

    // Update light icon position
    lo.icon->setPosition(transform.worldPosition);
    lo.icon->setVisible(transform.visible);

    // Update light icon scale
    CameraComponent& cameracomp = scene->getComponent<CameraComponent>(camera->getEntity());
    float lightIconScale = 0.25f;
    float scale = lightIconScale * zoom;

    if (cameracomp.type == CameraType::CAMERA_PERSPECTIVE){
        float dist = (lo.icon->getPosition() - camera->getWorldPosition()).length();
        scale = std::tan(cameracomp.yfov) * dist * (lightIconScale / (float)framebuffer.getHeight());
        if (!std::isfinite(scale) || scale <= 0.0f) {
            scale = 1.0f;
        }
    }

    lo.icon->setScale(scale);
}

void Editor::SceneRender3D::createDirectionalLightArrow(Entity entity, const Transform& transform, const LightComponent& light, bool isSelected) {
    LightObjects& lo = lightObjects[entity];

    lo.lines->setPosition(transform.worldPosition);
    lo.lines->setRotation(transform.worldRotation);
    lo.lines->setVisible(isSelected);

    if (light.direction == Vector3::ZERO){
        return;
    }

    if (lo.type == light.type && lo.direction == light.direction) {
        return;
    }

    lo.type = light.type;
    lo.direction = light.direction;

    lo.lines->clearLines();

    Vector3 position = Vector3(0, 0, 0);  // Start position
    float arrowLength = 4.0f;  // Length of the main arrow shaft
    float arrowHeadLength = 3.6f;  // Length of the arrow head
    float arrowHeadWidth = 1.0f;   // Width of the arrow head

    Vector4 arrowColor = Vector4(1.0, 1.0, 0.0, 1.0); // Yellow color for directional light

    // Main arrow shaft
    Vector3 endPos = position + light.direction * arrowLength;
    lo.lines->addLine(position, endPos, arrowColor);

    // Create orthonormal basis vectors perpendicular to light direction
    Vector3 up = Vector3(0, 1, 0);
    if (std::abs(light.direction.dotProduct(up)) > 0.9f) {
        up = Vector3(1, 0, 0);
    }
    Vector3 right = light.direction.crossProduct(up).normalized();
    up = right.crossProduct(light.direction).normalized();

    // Arrow head base position
    Vector3 arrowHeadBase = endPos - light.direction * arrowHeadLength;

    // Arrow head vertices (4 points forming a diamond shape)
    Vector3 arrowHead1 = arrowHeadBase + right * arrowHeadWidth;
    Vector3 arrowHead2 = arrowHeadBase + up * arrowHeadWidth;
    Vector3 arrowHead3 = arrowHeadBase - right * arrowHeadWidth;
    Vector3 arrowHead4 = arrowHeadBase - up * arrowHeadWidth;

    // Draw arrow head lines
    lo.lines->addLine(endPos, arrowHead1, arrowColor);
    lo.lines->addLine(endPos, arrowHead2, arrowColor);
    lo.lines->addLine(endPos, arrowHead3, arrowColor);
    lo.lines->addLine(endPos, arrowHead4, arrowColor);

    // Connect arrow head base points to form a diamond
    lo.lines->addLine(arrowHead1, arrowHead2, arrowColor);
    lo.lines->addLine(arrowHead2, arrowHead3, arrowColor);
    lo.lines->addLine(arrowHead3, arrowHead4, arrowColor);
    lo.lines->addLine(arrowHead4, arrowHead1, arrowColor);

    // Optional: Add some additional lines for better visualization
    lo.lines->addLine(arrowHead1, arrowHead3, arrowColor * 0.7f); // Cross lines with slightly dimmer color
    lo.lines->addLine(arrowHead2, arrowHead4, arrowColor * 0.7f);
}

void Editor::SceneRender3D::createPointLightSphere(Entity entity, const Transform& transform, const LightComponent& light, bool isSelected) {
    LightObjects& lo = lightObjects[entity];

    lo.lines->setPosition(transform.worldPosition);
    lo.lines->setRotation(transform.worldRotation);
    lo.lines->setVisible(isSelected);

    float range = (light.range > 0.0f) ? light.range : camera->getFarClip();

    if (lo.type == light.type && lo.range == range) {
        return;
    }

    lo.type = light.type;
    lo.range = range;

    lo.lines->clearLines();

    Vector3 position = Vector3(0, 0, 0);  // Start position
    Vector4 sphereColor = Vector4(0.0, 1.0, 1.0, 0.8); // Cyan color for point light

    const int numSegments = 16;
    const float angleStep = 2.0f * M_PI / numSegments;

    // Draw three orthogonal circles to represent the sphere

    // XY plane circle
    for (int i = 0; i < numSegments; i++) {
        float angle1 = i * angleStep;
        float angle2 = ((i + 1) % numSegments) * angleStep;

        Vector3 point1 = position + Vector3(std::cos(angle1) * range, std::sin(angle1) * range, 0);
        Vector3 point2 = position + Vector3(std::cos(angle2) * range, std::sin(angle2) * range, 0);

        lo.lines->addLine(point1, point2, sphereColor);
    }

    // XZ plane circle
    for (int i = 0; i < numSegments; i++) {
        float angle1 = i * angleStep;
        float angle2 = ((i + 1) % numSegments) * angleStep;

        Vector3 point1 = position + Vector3(std::cos(angle1) * range, 0, std::sin(angle1) * range);
        Vector3 point2 = position + Vector3(std::cos(angle2) * range, 0, std::sin(angle2) * range);

        lo.lines->addLine(point1, point2, sphereColor);
    }

    // YZ plane circle
    for (int i = 0; i < numSegments; i++) {
        float angle1 = i * angleStep;
        float angle2 = ((i + 1) % numSegments) * angleStep;

        Vector3 point1 = position + Vector3(0, std::cos(angle1) * range, std::sin(angle1) * range);
        Vector3 point2 = position + Vector3(0, std::cos(angle2) * range, std::sin(angle2) * range);

        lo.lines->addLine(point1, point2, sphereColor);
    }
}

void Editor::SceneRender3D::createSpotLightCones(Entity entity, const Transform& transform, const LightComponent& light, bool isSelected) {
    LightObjects& lo = lightObjects[entity];

    lo.lines->setPosition(transform.worldPosition);
    lo.lines->setRotation(transform.worldRotation);
    lo.lines->setVisible(isSelected);

    if (light.direction == Vector3::ZERO){
        return;
    }

    float range = (light.range > 0.0f) ? light.range : camera->getFarClip();

    // if light.range = 0.0 then light.shadowCameraNearFar.y is camera.farClip
    if (lo.type == light.type && 
        lo.innerConeCos == light.innerConeCos && 
        lo.outerConeCos == light.outerConeCos && 
        lo.direction == light.direction && 
        lo.range == range) {
        return;
    }

    lo.type = light.type;
    lo.innerConeCos = light.innerConeCos;
    lo.outerConeCos = light.outerConeCos;
    lo.direction = light.direction;
    lo.range = range;

    lo.lines->clearLines();

    Vector3 position = Vector3(0,0,0);  // Start position

    // Calculate cone radii at the end of the light range
    float innerRadius = range * std::tan(std::acos(light.innerConeCos));
    float outerRadius = range * std::tan(std::acos(light.outerConeCos));

    // Create orthonormal basis vectors perpendicular to light direction
    Vector3 up = Vector3(0, 1, 0);
    if (std::abs(light.direction.dotProduct(up)) > 0.9f) {
        up = Vector3(1, 0, 0);
    }
    Vector3 right = light.direction.crossProduct(up).normalized();
    up = right.crossProduct(light.direction).normalized();

    // End position of the cone
    Vector3 endPos = position + light.direction * range;

    const int numSegments = 12;
    const float angleStep = 2.0f * M_PI / numSegments;

    // Colors for inner and outer cones
    Vector4 innerConeColor = Vector4(1.0, 1.0, 0.0, 0.8); // Yellow for inner cone
    Vector4 outerConeColor = Vector4(1.0, 0.5, 0.0, 0.6); // Orange for outer cone

    // Draw outer cone
    for (int i = 0; i < numSegments; i++) {
        float angle1 = i * angleStep;
        float angle2 = ((i + 1) % numSegments) * angleStep;

        // Calculate points on the outer cone circle
        Vector3 point1 = endPos + (right * std::cos(angle1) + up * std::sin(angle1)) * outerRadius;
        Vector3 point2 = endPos + (right * std::cos(angle2) + up * std::sin(angle2)) * outerRadius;

        // Lines from light position to circle points
        lo.lines->addLine(position, point1, outerConeColor);

        // Circle at the end of the cone
        lo.lines->addLine(point1, point2, outerConeColor);
    }

    // Draw inner cone
    for (int i = 0; i < numSegments; i++) {
        float angle1 = i * angleStep;
        float angle2 = ((i + 1) % numSegments) * angleStep;

        // Calculate points on the inner cone circle
        Vector3 point1 = endPos + (right * std::cos(angle1) + up * std::sin(angle1)) * innerRadius;
        Vector3 point2 = endPos + (right * std::cos(angle2) + up * std::sin(angle2)) * innerRadius;

        // Lines from light position to circle points
        lo.lines->addLine(position, point1, innerConeColor);

        // Circle at the end of the cone
        lo.lines->addLine(point1, point2, innerConeColor);
    }

    // Draw central direction line
    lo.lines->addLine(position, endPos, Vector4(0.8, 0.8, 0.8, 1.0));
}

void Editor::SceneRender3D::activate(){
    SceneRender::activate();

    Engine::addSceneLayer(viewgizmo.getScene());
}

void Editor::SceneRender3D::updateSelLines(std::vector<OBB> obbs){
    Vector4 color = Vector4(1.0, 0.6, 0.0, 1.0);

    if (selLines->getNumLines() != obbs.size() * 12){
        selLines->clearLines();
        for (OBB& obb : obbs){
            selLines->addLine(obb.getCorner(OBB::FAR_LEFT_BOTTOM), obb.getCorner(OBB::FAR_LEFT_TOP), color);
            selLines->addLine(obb.getCorner(OBB::FAR_LEFT_TOP), obb.getCorner(OBB::FAR_RIGHT_TOP), color);
            selLines->addLine(obb.getCorner(OBB::FAR_RIGHT_TOP), obb.getCorner(OBB::FAR_RIGHT_BOTTOM), color);
            selLines->addLine(obb.getCorner(OBB::FAR_RIGHT_BOTTOM), obb.getCorner(OBB::FAR_LEFT_BOTTOM), color);

            selLines->addLine(obb.getCorner(OBB::NEAR_LEFT_BOTTOM), obb.getCorner(OBB::NEAR_LEFT_TOP), color);
            selLines->addLine(obb.getCorner(OBB::NEAR_LEFT_TOP), obb.getCorner(OBB::NEAR_RIGHT_TOP), color);
            selLines->addLine(obb.getCorner(OBB::NEAR_RIGHT_TOP), obb.getCorner(OBB::NEAR_RIGHT_BOTTOM), color);
            selLines->addLine(obb.getCorner(OBB::NEAR_RIGHT_BOTTOM), obb.getCorner(OBB::NEAR_LEFT_BOTTOM)), color;

            selLines->addLine(obb.getCorner(OBB::FAR_LEFT_BOTTOM), obb.getCorner(OBB::NEAR_LEFT_BOTTOM), color);
            selLines->addLine(obb.getCorner(OBB::FAR_LEFT_TOP), obb.getCorner(OBB::NEAR_LEFT_TOP), color);
            selLines->addLine(obb.getCorner(OBB::FAR_RIGHT_TOP), obb.getCorner(OBB::NEAR_RIGHT_TOP), color);
            selLines->addLine(obb.getCorner(OBB::FAR_RIGHT_BOTTOM), obb.getCorner(OBB::NEAR_RIGHT_BOTTOM), color);
        }
    }else{
        int i = 0;
        for (OBB& obb : obbs){
            selLines->updateLine(i * 12 + 0, obb.getCorner(OBB::FAR_LEFT_BOTTOM), obb.getCorner(OBB::FAR_LEFT_TOP));
            selLines->updateLine(i * 12 + 1, obb.getCorner(OBB::FAR_LEFT_TOP), obb.getCorner(OBB::FAR_RIGHT_TOP));
            selLines->updateLine(i * 12 + 2, obb.getCorner(OBB::FAR_RIGHT_TOP), obb.getCorner(OBB::FAR_RIGHT_BOTTOM));
            selLines->updateLine(i * 12 + 3, obb.getCorner(OBB::FAR_RIGHT_BOTTOM), obb.getCorner(OBB::FAR_LEFT_BOTTOM));

            selLines->updateLine(i * 12 + 4, obb.getCorner(OBB::NEAR_LEFT_BOTTOM), obb.getCorner(OBB::NEAR_LEFT_TOP));
            selLines->updateLine(i * 12 + 5, obb.getCorner(OBB::NEAR_LEFT_TOP), obb.getCorner(OBB::NEAR_RIGHT_TOP));
            selLines->updateLine(i * 12 + 6, obb.getCorner(OBB::NEAR_RIGHT_TOP), obb.getCorner(OBB::NEAR_RIGHT_BOTTOM));
            selLines->updateLine(i * 12 + 7, obb.getCorner(OBB::NEAR_RIGHT_BOTTOM), obb.getCorner(OBB::NEAR_LEFT_BOTTOM));

            selLines->updateLine(i * 12 + 8, obb.getCorner(OBB::FAR_LEFT_BOTTOM), obb.getCorner(OBB::NEAR_LEFT_BOTTOM));
            selLines->updateLine(i * 12 + 9, obb.getCorner(OBB::FAR_LEFT_TOP), obb.getCorner(OBB::NEAR_LEFT_TOP));
            selLines->updateLine(i * 12 + 10, obb.getCorner(OBB::FAR_RIGHT_TOP), obb.getCorner(OBB::NEAR_RIGHT_TOP));
            selLines->updateLine(i * 12 + 11, obb.getCorner(OBB::FAR_RIGHT_BOTTOM), obb.getCorner(OBB::NEAR_RIGHT_BOTTOM));
            i++;
        }
    }
}

void Editor::SceneRender3D::update(std::vector<Entity> selEntities, std::vector<Entity> entities){
    SceneRender::update(selEntities, entities);

    int linesStepChange = (int)(camera->getFarClip() / 2);
    int cameraLineStepX = (int)(camera->getWorldPosition().x / linesStepChange) * linesStepChange;
    int cameraLineStepZ = (int)(camera->getWorldPosition().z / linesStepChange) * linesStepChange;
    if (cameraLineStepX != linesOffset.x || cameraLineStepZ != linesOffset.y){
        linesOffset = Vector2(cameraLineStepX, cameraLineStepZ);

        createLines();
    }

    viewgizmo.applyRotation(camera);

    std::set<Entity> currentIconLights;

    for (Entity& entity: entities){
        Signature signature = scene->getSignature(entity);

        if (signature.test(scene->getComponentId<LightComponent>()) && signature.test(scene->getComponentId<Transform>())) {
            LightComponent& light = scene->getComponent<LightComponent>(entity);
            Transform& transform = scene->getComponent<Transform>(entity);

            bool isSelected = std::find(selEntities.begin(), selEntities.end(), entity) != selEntities.end();

            currentIconLights.insert(entity);
            bool newLight = instanciateLightObject(entity) || lightObjects[entity].type != light.type;
            if (light.type == LightType::DIRECTIONAL){
                createOrUpdateLightIcon(entity, transform, LightType::DIRECTIONAL, newLight);
                createDirectionalLightArrow(entity, transform, light, isSelected);
            }else if (light.type == LightType::POINT){
                createOrUpdateLightIcon(entity, transform, LightType::POINT, newLight);
                createPointLightSphere(entity, transform, light, isSelected);
            }else if (light.type == LightType::SPOT){
                createOrUpdateLightIcon(entity, transform, LightType::SPOT, newLight);
                createSpotLightCones(entity, transform, light, isSelected);
            }
        }
    }

    // Remove sun icons for entities that are no longer directional lights
    auto it = lightObjects.begin();
    while (it != lightObjects.end()) {
        if (currentIconLights.find(it->first) == currentIconLights.end()) {
            delete it->second.icon;
            delete it->second.lines;
            it = lightObjects.erase(it);
        } else {
            ++it;
        }
    }
}

void Editor::SceneRender3D::mouseHoverEvent(float x, float y){
    SceneRender::mouseHoverEvent(x, y);
}

void Editor::SceneRender3D::mouseClickEvent(float x, float y, std::vector<Entity> selEntities){
    SceneRender::mouseClickEvent(x, y, selEntities);
}

void Editor::SceneRender3D::mouseReleaseEvent(float x, float y){
    SceneRender::mouseReleaseEvent(x, y);
}

void Editor::SceneRender3D::mouseDragEvent(float x, float y, float origX, float origY, size_t sceneId, SceneProject* sceneProject, std::vector<Entity> selEntities, bool disableSelection){
    SceneRender::mouseDragEvent(x, y, origX, origY, sceneId, sceneProject, selEntities, disableSelection);
}

Editor::ViewportGizmo* Editor::SceneRender3D::getViewportGizmo(){
    return &viewgizmo;
}