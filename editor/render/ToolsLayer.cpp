#include "ToolsLayer.h"

using namespace Supernova;

Editor::ToolsLayer::ToolsLayer(){
    scene = new Scene();
    camera = new Camera(scene);

    gizmo = new Object(scene);
    sphere = new Shape(scene);
    xaxis = new Shape(scene);
    yaxis = new Shape(scene);
    zaxis = new Shape(scene);
    xarrow = new Shape(scene);
    yarrow = new Shape(scene);
    zarrow = new Shape(scene);
    
    sphere->createSphere(0.2);
    sphere->setColor(1.0, 0.5, 0.2, 1.0);

    xaxis->createCylinder(0.05, 2);
    yaxis->createCylinder(0.05, 2);
    zaxis->createCylinder(0.05, 2);

    xaxis->setColor(0.7, 0.2, 0.2, 1.0);
    yaxis->setColor(0.2, 0.7, 0.2, 1.0);
    zaxis->setColor(0.2, 0.2, 0.7, 1.0);

    xaxis->setPosition(1, 0, 0);
    yaxis->setPosition(0, 1, 0);
    zaxis->setPosition(0, 0, 1);

    xaxis->setRotation(0,0,90);
    zaxis->setRotation(90,0,0);

    xarrow->createCylinder(0.1, 0.0, 0.4);
    yarrow->createCylinder(0.1, 0.0, 0.4);
    zarrow->createCylinder(0.1, 0.0, 0.4);

    xarrow->setPosition(2, 0, 0);
    yarrow->setPosition(0, 2, 0);
    zarrow->setPosition(0, 0, 2);

    xarrow->setRotation(0,0,-90);
    zarrow->setRotation(90,0,0);

    xarrow->setColor(0.7, 0.2, 0.2, 1.0);
    yarrow->setColor(0.2, 0.7, 0.2, 1.0);
    zarrow->setColor(0.2, 0.2, 0.7, 1.0);

    gizmo->addChild(sphere);
    gizmo->addChild(xaxis);
    gizmo->addChild(yaxis);
    gizmo->addChild(zaxis);
    gizmo->addChild(xarrow);
    gizmo->addChild(yarrow);
    gizmo->addChild(zarrow);

    scene->setCamera(camera);
}

void Editor::ToolsLayer::updateCamera(CameraComponent& extCamera, Transform& extCameraTransform){
    Entity entity = camera->getEntity();
    CameraComponent& cameracomp = scene->getComponent<CameraComponent>(entity);

    camera->setPosition(extCameraTransform.position);
    camera->setView(extCamera.view);

    cameracomp.type = extCamera.type;
    cameracomp.left = extCamera.left;
    cameracomp.right = extCamera.right;
    cameracomp.bottom = extCamera.bottom;
    cameracomp.top = extCamera.top;
    cameracomp.nearPlane = extCamera.nearPlane;
    cameracomp.farPlane = extCamera.farPlane;
    cameracomp.yfov = extCamera.yfov;
    cameracomp.aspect = extCamera.aspect;
    cameracomp.automatic = extCamera.automatic;
    if (extCamera.needUpdate){
        cameracomp.needUpdate = extCamera.needUpdate;
    }

    AABB aabb = zarrow->getWorldAABB();
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
    return gizmo;
}