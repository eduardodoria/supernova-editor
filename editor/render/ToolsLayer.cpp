#include "ToolsLayer.h"

using namespace Supernova;

Editor::ToolsLayer::ToolsLayer(){
    float cylinderRadius = 0.05;
    float cylinderHeight = 2;
    float arrowRadius = 0.1;
    float arrowHeight = 0.4;
    gizmoSideSelected = GizmoSideSelected::NONE;

    scene = new Scene();
    camera = new Camera(scene);

    tGizmo = new TranslateGizmo(scene);

    scene->setCamera(camera);
}

void Editor::ToolsLayer::updateCamera(CameraComponent& extCamera, Transform& extCameraTransform){
    Entity entity = camera->getEntity();
    CameraComponent& cameracomp = scene->getComponent<CameraComponent>(entity);

    camera->setPosition(extCameraTransform.position);
    camera->setTarget(extCamera.target);

    cameracomp.type = extCamera.type;
    cameracomp.leftPlane = extCamera.leftPlane;
    cameracomp.rightPlane = extCamera.rightPlane;
    cameracomp.bottomPlane = extCamera.bottomPlane;
    cameracomp.topPlane = extCamera.topPlane;
    cameracomp.nearPlane = extCamera.nearPlane;
    cameracomp.farPlane = extCamera.farPlane;
    cameracomp.yfov = extCamera.yfov;
    cameracomp.aspect = extCamera.aspect;
    cameracomp.automatic = extCamera.automatic;
    if (extCamera.needUpdate){
        cameracomp.needUpdate = extCamera.needUpdate;
    }
}

void Editor::ToolsLayer::updateGizmo(Vector3& position, float scale, Ray& mouseRay, bool mouseClicked){
    tGizmo->setPosition(position);
    tGizmo->setScale(scale);
    if (!mouseClicked){
        gizmoSideSelected = tGizmo->checkHoverHighlight(mouseRay);
    }
}

Framebuffer* Editor::ToolsLayer::getFramebuffer(){
    return camera->getFramebuffer();
}

TextureRender& Editor::ToolsLayer::getTexture(){
    return getFramebuffer()->getRender().getColorTexture();
}

Camera* Editor::ToolsLayer::getCamera(){
    return camera;
}

Scene* Editor::ToolsLayer::getScene(){
    return scene;
}

Object* Editor::ToolsLayer::getGizmo(){
    return tGizmo;
}

Editor::GizmoSideSelected Editor::ToolsLayer::getGizmoSideSelected() const{
    return gizmoSideSelected;
}