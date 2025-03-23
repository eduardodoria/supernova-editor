#include "SceneRender3D.h"

#include "resources/sky/Daylight_Box_Back_png.h"
#include "resources/sky/Daylight_Box_Bottom_png.h"
#include "resources/sky/Daylight_Box_Front_png.h"
#include "resources/sky/Daylight_Box_Left_png.h"
#include "resources/sky/Daylight_Box_Right_png.h"
#include "resources/sky/Daylight_Box_Top_png.h"

#include "Project.h"

using namespace Supernova;

Editor::SceneRender3D::SceneRender3D(Scene* scene): SceneRender(scene, false, true, 40.0, 0.01){
    linesOffset = Vector2(0, 0);

    lines = new Lines(scene);
    sun = new Light(scene);
    sky = new SkyBox(scene);

    selLines = new Lines(scene);
    for (int i = 0; i < 12; i++){
        selLines->addLine(Vector3::ZERO, Vector3::ZERO, Vector4(1.0, 0.6, 0.0, 1.0));
    }
    selLines->setVisible(false);

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

    sky->setTextures("default_editor_sky", skyBack, skyFront, skyLeft, skyRight, skyTop, skyBottom);

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

    scene->setAmbientLight(0.2);
    //scene->setSceneAmbientLightEnabled(false);
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

void Editor::SceneRender3D::updateSelLines(AABB aabb){
    selLines->updateLine(0, aabb.getCorner(AABB::FAR_LEFT_BOTTOM), aabb.getCorner(AABB::FAR_LEFT_TOP));
    selLines->updateLine(1, aabb.getCorner(AABB::FAR_LEFT_TOP), aabb.getCorner(AABB::FAR_RIGHT_TOP));
    selLines->updateLine(2, aabb.getCorner(AABB::FAR_RIGHT_TOP), aabb.getCorner(AABB::FAR_RIGHT_BOTTOM));
    selLines->updateLine(3, aabb.getCorner(AABB::FAR_RIGHT_BOTTOM), aabb.getCorner(AABB::FAR_LEFT_BOTTOM));

    selLines->updateLine(4, aabb.getCorner(AABB::NEAR_LEFT_BOTTOM), aabb.getCorner(AABB::NEAR_LEFT_TOP));
    selLines->updateLine(5, aabb.getCorner(AABB::NEAR_LEFT_TOP), aabb.getCorner(AABB::NEAR_RIGHT_TOP));
    selLines->updateLine(6, aabb.getCorner(AABB::NEAR_RIGHT_TOP), aabb.getCorner(AABB::NEAR_RIGHT_BOTTOM));
    selLines->updateLine(7, aabb.getCorner(AABB::NEAR_RIGHT_BOTTOM), aabb.getCorner(AABB::NEAR_LEFT_BOTTOM));

    selLines->updateLine(8, aabb.getCorner(AABB::FAR_LEFT_BOTTOM), aabb.getCorner(AABB::NEAR_LEFT_BOTTOM));
    selLines->updateLine(9, aabb.getCorner(AABB::FAR_LEFT_TOP), aabb.getCorner(AABB::NEAR_LEFT_TOP));
    selLines->updateLine(10, aabb.getCorner(AABB::FAR_RIGHT_TOP), aabb.getCorner(AABB::NEAR_RIGHT_TOP));
    selLines->updateLine(11, aabb.getCorner(AABB::FAR_RIGHT_BOTTOM), aabb.getCorner(AABB::NEAR_RIGHT_BOTTOM));
}

void Editor::SceneRender3D::update(std::vector<Entity> selEntities){
    SceneRender::update(selEntities);

    int linesStepChange = (int)(camera->getFarClip() / 2);
    int cameraLineStepX = (int)(camera->getWorldPosition().x / linesStepChange) * linesStepChange;
    int cameraLineStepZ = (int)(camera->getWorldPosition().z / linesStepChange) * linesStepChange;
    if (cameraLineStepX != linesOffset.x || cameraLineStepZ != linesOffset.y){
        linesOffset = Vector2(cameraLineStepX, cameraLineStepZ);

        createLines();
    }

    viewgizmo.applyRotation(camera);
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