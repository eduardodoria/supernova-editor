#include "SceneRender3D.h"

#include "resources/sky/Daylight_Box_Back_png.h"
#include "resources/sky/Daylight_Box_Bottom_png.h"
#include "resources/sky/Daylight_Box_Front_png.h"
#include "resources/sky/Daylight_Box_Left_png.h"
#include "resources/sky/Daylight_Box_Right_png.h"
#include "resources/sky/Daylight_Box_Top_png.h"
#include "resources/icons/sun-icon_png.h"

#include "Project.h"

using namespace Supernova;

Editor::SceneRender3D::SceneRender3D(Scene* scene): SceneRender(scene, false, true, 40.0, 0.01){
    linesOffset = Vector2(0, 0);

    lines = new Lines(scene);
    sun = new Light(scene);
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

    sunIcons.clear();

    createLines();

    //camera->setType(CameraType::CAMERA_2D);
    camera->setPosition(10, 4, 10);
    camera->setTarget(0, 0, 0);

    //camera->setRenderToTexture(true);
    //camera->setUseFramebufferSizes(false);

    sun->setType(LightType::DIRECTIONAL);
    sun->setDirection(-0.2, -0.5, 0.3);
    sun->setIntensity(4.0);
    sun->setShadows(true);
    sun->setRange(100);

    scene->setGlobalIllumination(0.2);
    scene->setBackgroundColor(Vector4(0.25, 0.45, 0.65, 1.0));

    uilayer.setViewportGizmoTexture(viewgizmo.getFramebuffer());

    Engine::setScalingMode(Scaling::NATIVE);
    Engine::setFixedTimeSceneUpdate(false);
}

Editor::SceneRender3D::~SceneRender3D(){
    delete lines;
    delete sun;
    delete sky;
    delete selLines;

    for (auto& pair : sunIcons) {
        delete pair.second;
    }
    sunIcons.clear();
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

    std::set<Entity> currentDirectionalLights;

    for (Entity& entity: entities){
        Signature signature = scene->getSignature(entity);

        if (signature.test(scene->getComponentId<LightComponent>()) && signature.test(scene->getComponentId<Transform>())) {
            LightComponent& light = scene->getComponent<LightComponent>(entity);
            Transform& transform = scene->getComponent<Transform>(entity);

            if (light.type == LightType::DIRECTIONAL){
                currentDirectionalLights.insert(entity);

                // Create sun icon if it doesn't exist for this entity
                if (sunIcons.find(entity) == sunIcons.end()) {
                    Sprite* newSunIcon = new Sprite(scene);

                    TextureData sunIconData;
                    sunIconData.loadTextureFromMemory(sun_icon_png, sun_icon_png_len);

                    newSunIcon->setTexture("editor:resources:sun_icon", sunIconData);
                    newSunIcon->setBillboard(true);
                    newSunIcon->setSize(128, 128);
                    newSunIcon->setReceiveLights(false);
                    newSunIcon->setCastShadows(false);
                    newSunIcon->setReceiveShadows(false);
                    newSunIcon->setPivotPreset(PivotPreset::CENTER);

                    sunIcons[entity] = newSunIcon;
                }

                // Update sun icon position
                sunIcons[entity]->setPosition(transform.worldPosition);

                // Update sun icon scale
                CameraComponent& cameracomp = scene->getComponent<CameraComponent>(camera->getEntity());
                float sunIconScale = 0.25f;
                float scale = sunIconScale * zoom;

                if (cameracomp.type == CameraType::CAMERA_PERSPECTIVE){
                    float dist = (sunIcons[entity]->getPosition() - camera->getWorldPosition()).length();
                    scale = std::tan(cameracomp.yfov) * dist * (sunIconScale / (float)framebuffer.getHeight());
                }

                sunIcons[entity]->setScale(scale);
            }
        }
    }

    // Remove sun icons for entities that are no longer directional lights
    auto it = sunIcons.begin();
    while (it != sunIcons.end()) {
        if (currentDirectionalLights.find(it->first) == currentDirectionalLights.end()) {
            delete it->second;
            it = sunIcons.erase(it);
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

Light* Editor::SceneRender3D::getSunLight(){
    return sun;
}

Editor::ViewportGizmo* Editor::SceneRender3D::getViewportGizmo(){
    return &viewgizmo;
}