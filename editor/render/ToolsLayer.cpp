#include "ToolsLayer.h"

using namespace Supernova;

Editor::ToolsLayer::ToolsLayer(bool use2DGizmos){
    if (use2DGizmos){
        gizmoSelected = GizmoSelected::OBJECT2D;
    }else{
        gizmoSelected = GizmoSelected::TRANSLATE;
    }
    gizmoSideSelected = GizmoSideSelected::NONE;
    gizmo2DSideSelected = Gizmo2DSideSelected::NONE;

    scene = new Scene(EntityPool::System);
    camera = new Camera(scene);

    tGizmo = new TranslateGizmo(scene, use2DGizmos);
    rGizmo = new RotateGizmo(scene, use2DGizmos);
    sGizmo = new ScaleGizmo(scene, use2DGizmos);
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
    cameracomp.useTarget = extCamera.useTarget;
    cameracomp.autoResize = extCamera.autoResize;
    if (extCamera.needUpdate){
        cameracomp.needUpdate = extCamera.needUpdate;
    }
}

void Editor::ToolsLayer::updateGizmo(Camera* sceneCam, Vector3& position, Quaternion& rotation, float scale, OBB obb, Ray& mouseRay, bool mouseClicked){
    gizmoScale = scale;

    if (gizmoSelected == GizmoSelected::TRANSLATE){
        tGizmo->setPosition(position);
        tGizmo->setRotation(rotation);
        tGizmo->setScale(scale);
        if (!mouseClicked){
            gizmoSideSelected = tGizmo->checkHover(mouseRay);
            gizmo2DSideSelected = Gizmo2DSideSelected::NONE;
        }
    }
    if (gizmoSelected == GizmoSelected::ROTATE){
        rGizmo->updateRotations(camera);
        rGizmo->setPosition(position);
        rGizmo->setRotation(rotation);
        rGizmo->setScale(scale);
        if (!mouseClicked){
            gizmoSideSelected = rGizmo->checkHover(mouseRay);
            gizmo2DSideSelected = Gizmo2DSideSelected::NONE;
        }
    }
    if (gizmoSelected == GizmoSelected::SCALE){
        sGizmo->setPosition(position);
        sGizmo->setRotation(rotation);
        sGizmo->setScale(scale);
        if (!mouseClicked){
            gizmoSideSelected = sGizmo->checkHover(mouseRay);
            gizmo2DSideSelected = Gizmo2DSideSelected::NONE;
        }
    }
    // only for single selections
    // do not use gizmo position and rotation
    if (gizmoSelected == GizmoSelected::OBJECT2D){
        oGizmo->setPosition(position);
        oGizmo->setRotation(rotation);
        oGizmo->setScale(scale);

        Vector3 center = obb.getCenter();
        Vector3 size = obb.getHalfExtents() * 2.0f;
        oGizmo->setCenter(rotation.getRotationMatrix().inverse() * (center - position) / scale);
        oGizmo->setSize(size.x / scale, size.y / scale);

        if (!mouseClicked){
            gizmoSideSelected = GizmoSideSelected::NONE;
            gizmo2DSideSelected = oGizmo->checkHover(mouseRay, obb);
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

void Editor::ToolsLayer::enableObject2DGizmo(){
    gizmoSelected = GizmoSelected::OBJECT2D;
}

void Editor::ToolsLayer::setGizmoVisible(bool visible){
    tGizmo->setVisible(false);
    rGizmo->setVisible(false);
    sGizmo->setVisible(false);
    oGizmo->setVisible(false);

    if (gizmoSelected == GizmoSelected::TRANSLATE){
        tGizmo->setVisible(visible);
    }
    if (gizmoSelected == GizmoSelected::ROTATE){
        rGizmo->setVisible(visible);
    }
    if (gizmoSelected == GizmoSelected::SCALE){
        sGizmo->setVisible(visible);
    }
    if (gizmoSelected == GizmoSelected::OBJECT2D){
        oGizmo->setVisible(visible);
    }

    if (!visible){
        gizmoSideSelected = GizmoSideSelected::NONE;
        gizmo2DSideSelected = Gizmo2DSideSelected::NONE;
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
    if (gizmoSelected == GizmoSelected::OBJECT2D){
        return oGizmo;
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

Editor::Gizmo2DSideSelected Editor::ToolsLayer::getGizmo2DSideSelected() const{
    return gizmo2DSideSelected;
}