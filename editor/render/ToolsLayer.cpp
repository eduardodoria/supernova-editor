#include "ToolsLayer.h"

using namespace Supernova;

Editor::ToolsLayer::ToolsLayer(){
    gizmoSelected = GizmoSelected::TRANSLATE;
    gizmoSideSelected = GizmoSideSelected::NONE;

    scene = new Scene();
    camera = new Camera(scene);

    tGizmo = new TranslateGizmo(scene);
    rGizmo = new RotateGizmo(scene);
    sGizmo = new ScaleGizmo(scene);
    oGizmo = new Object2DGizmo(scene);

    scene->setCamera(camera);

    gizmoScale = 1.0f;
}

Editor::ToolsLayer::~ToolsLayer(){
    delete camera;
    delete tGizmo;
    delete rGizmo;
    delete sGizmo;
    delete oGizmo;

    delete scene;
}

void Editor::ToolsLayer::updateCamera(CameraComponent& extCamera, Transform& extCameraTransform){
    Entity entity = camera->getEntity();
    CameraComponent& cameracomp = scene->getComponent<CameraComponent>(entity);

    camera->setPosition(extCameraTransform.position);
    camera->setTarget(extCamera.target);

    cameracomp.type = extCamera.type;
    cameracomp.leftClip = extCamera.leftClip;
    cameracomp.rightClip = extCamera.rightClip;
    cameracomp.bottomClip = extCamera.bottomClip;
    cameracomp.topClip = extCamera.topClip;
    cameracomp.nearClip = extCamera.nearClip;
    cameracomp.farClip = extCamera.farClip;
    cameracomp.yfov = extCamera.yfov;
    cameracomp.aspect = extCamera.aspect;
    cameracomp.automatic = extCamera.automatic;
    if (extCamera.needUpdate){
        cameracomp.needUpdate = extCamera.needUpdate;
    }
    /*
    float newNearClip;
    float newFarClip;
    if (cameracomp.type == CameraType::CAMERA_PERSPECTIVE){
        //newNearClip = 0.0001;
        newFarClip = extCamera.farClip * gizmoScale;
    }else{
        //newNearClip = extCamera.nearClip * gizmoScale;
        newFarClip = extCamera.farClip * gizmoScale;
    }
    if (newNearClip != cameracomp.nearClip || newFarClip != cameracomp.farClip){
        cameracomp.nearClip = newNearClip;
        cameracomp.farClip = newFarClip;
        cameracomp.needUpdate = true;
    }
    */
}

void Editor::ToolsLayer::updateGizmo(Camera* sceneCam, Vector3& position, Quaternion& rotation, float scale, Ray& mouseRay, bool mouseClicked){
    gizmoScale = scale;

    if (gizmoSelected == GizmoSelected::TRANSLATE){
        tGizmo->setPosition(position);
        tGizmo->setRotation(rotation);
        tGizmo->setScale(scale);
        if (!mouseClicked){
            gizmoSideSelected = tGizmo->checkHoverHighlight(mouseRay);
        }
    }
    if (gizmoSelected == GizmoSelected::ROTATE){
        rGizmo->updateRotations(camera);
        rGizmo->setPosition(position);
        rGizmo->setRotation(rotation);
        rGizmo->setScale(scale);
        if (!mouseClicked){
            gizmoSideSelected = rGizmo->checkHoverHighlight(mouseRay);
        }
    }
    if (gizmoSelected == GizmoSelected::SCALE){
        sGizmo->setPosition(position);
        sGizmo->setRotation(rotation);
        sGizmo->setScale(scale);
        if (!mouseClicked){
            gizmoSideSelected = sGizmo->checkHoverHighlight(mouseRay);
        }
    }
}

void Editor::ToolsLayer::mouseDrag(Vector3 point){
    if (gizmoSelected == GizmoSelected::ROTATE && gizmoSideSelected != GizmoSideSelected::NONE){
        rGizmo->drawLine(point);
    }
}

void Editor::ToolsLayer::mouseRelease(){
    if (gizmoSelected == GizmoSelected::ROTATE){
        rGizmo->removeLine();
    }
}

void Editor::ToolsLayer::enableTranslateGizmo(){
     gizmoSelected = GizmoSelected::TRANSLATE;
}

void Editor::ToolsLayer::enableRotateGizmo(){
    gizmoSelected = GizmoSelected::ROTATE;
}

void Editor::ToolsLayer::enableScaleGizmo(){
    gizmoSelected = GizmoSelected::SCALE;
}

void Editor::ToolsLayer::setGizmoVisible(bool visible){
    tGizmo->setVisible(false);
    rGizmo->setVisible(false);
    sGizmo->setVisible(false);

    if (gizmoSelected == GizmoSelected::TRANSLATE){
        tGizmo->setVisible(visible);
    }
    if (gizmoSelected == GizmoSelected::ROTATE){
        rGizmo->setVisible(visible);
    }
    if (gizmoSelected == GizmoSelected::SCALE){
        sGizmo->setVisible(visible);
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

Object* Editor::ToolsLayer::getGizmoObject() const{
    if (gizmoSelected == GizmoSelected::TRANSLATE){
        return tGizmo;
    }
    if (gizmoSelected == GizmoSelected::ROTATE){
        return rGizmo;
    }
    if (gizmoSelected == GizmoSelected::SCALE){
        return sGizmo;
    }

    return nullptr;
}

Vector3 Editor::ToolsLayer::getGizmoPosition() const{
    return getGizmoObject()->getPosition();
}

Quaternion Editor::ToolsLayer::getGizmoRotation() const{
    return getGizmoObject()->getRotation();
}

Editor::GizmoSelected Editor::ToolsLayer::getGizmoSelected() const{
    return gizmoSelected;
}

Editor::GizmoSideSelected Editor::ToolsLayer::getGizmoSideSelected() const{
    return gizmoSideSelected;
}