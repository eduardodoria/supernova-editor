#include "ToolsLayer.h"

using namespace Supernova;

const Vector4 Editor::ToolsLayer::sphereColor = Vector4(1.0, 0.5, 0.2, 1.0);
const Vector4 Editor::ToolsLayer::xaxisColor = Vector4(0.7, 0.2, 0.2, 1.0);
const Vector4 Editor::ToolsLayer::yaxisColor = Vector4(0.2, 0.7, 0.2, 1.0);
const Vector4 Editor::ToolsLayer::zaxisColor = Vector4(0.2, 0.2, 0.7, 1.0);
const Vector4 Editor::ToolsLayer::sphereColorHightlight = Vector4(0.7, 0.7, 1.0, 1.0);
const Vector4 Editor::ToolsLayer::xaxisColorHightlight = Vector4(0.9, 0.7, 0.7, 1.0);
const Vector4 Editor::ToolsLayer::yaxisColorHightlight = Vector4(0.7, 0.9, 0.7, 1.0);
const Vector4 Editor::ToolsLayer::zaxisColorHightlight = Vector4(0.7, 0.7, 0.9, 1.0);

Editor::ToolsLayer::ToolsLayer(){
    float cylinderRadius = 0.05;
    float cylinderHeight = 2;
    float arrowRadius = 0.1;
    float arrowHeight = 0.4;

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
    sphere->setColor(sphereColor);

    xaxis->createCylinder(cylinderRadius, cylinderHeight);
    yaxis->createCylinder(cylinderRadius, cylinderHeight);
    zaxis->createCylinder(cylinderRadius, cylinderHeight);

    xaxis->setColor(xaxisColor);
    yaxis->setColor(yaxisColor);
    zaxis->setColor(zaxisColor);

    xaxis->setPosition(cylinderHeight/2.0, 0, 0);
    yaxis->setPosition(0, cylinderHeight/2.0, 0);
    zaxis->setPosition(0, 0, cylinderHeight/2.0);

    xaxis->setRotation(0,0,90);
    zaxis->setRotation(90,0,0);

    xarrow->createCylinder(arrowRadius, 0.0, arrowHeight);
    yarrow->createCylinder(arrowRadius, 0.0, arrowHeight);
    zarrow->createCylinder(arrowRadius, 0.0, arrowHeight);

    xarrow->setPosition(cylinderHeight, 0, 0);
    yarrow->setPosition(0, cylinderHeight, 0);
    zarrow->setPosition(0, 0, cylinderHeight);

    xarrow->setRotation(0,0,-90);
    zarrow->setRotation(90,0,0);

    //xho

    xarrow->setColor(xaxisColor);
    yarrow->setColor(yaxisColor);
    zarrow->setColor(zaxisColor);

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

bool Editor::ToolsLayer::updateGizmo(Vector3& position, float scale, Ray& mouseRay){
    gizmo->setPosition(position);
    gizmo->setScale(scale);
    return checkHoverHighlight(mouseRay);
}

bool Editor::ToolsLayer::checkHoverHighlight(Ray& ray){
    bool selected = false;
    bool sphereSelected = false;

    AABB sphereaabb = Matrix4::scaleMatrix(Vector3(2,2,2)) * sphere->getWorldAABB();

    if (ray.intersects(sphereaabb)){
        sphere->setColor(sphereColorHightlight);
        selected = true;
        sphereSelected = true;
    }else{
        sphere->setColor(sphereColor);
    }

    AABB xaabb = (gizmo->getRotation().getRotationMatrix() * Matrix4::scaleMatrix(Vector3(1,2,2))) * xaxis->getWorldAABB().merge(xarrow->getWorldAABB());
    AABB yaabb = (gizmo->getRotation().getRotationMatrix() * Matrix4::scaleMatrix(Vector3(2,1,2))) * yaxis->getWorldAABB().merge(yarrow->getWorldAABB());
    AABB zaabb = (gizmo->getRotation().getRotationMatrix() * Matrix4::scaleMatrix(Vector3(2,2,1))) * zaxis->getWorldAABB().merge(zarrow->getWorldAABB());

    if (!sphereSelected && ray.intersects(xaabb)){
        xaxis->setColor(xaxisColorHightlight);
        xarrow->setColor(xaxisColorHightlight);
        selected = true;
    }else{
        xaxis->setColor(xaxisColor);
        xarrow->setColor(xaxisColor);
    }

    if (!sphereSelected && ray.intersects(yaabb)){
        yaxis->setColor(yaxisColorHightlight);
        yarrow->setColor(yaxisColorHightlight);
        selected = true;
    }else{
        yaxis->setColor(yaxisColor);
        yarrow->setColor(yaxisColor);
    }

    if (!sphereSelected && ray.intersects(zaabb)){
        zaxis->setColor(zaxisColorHightlight);
        zarrow->setColor(zaxisColorHightlight);
        selected = true;
    }else{
        zaxis->setColor(zaxisColor);
        zarrow->setColor(zaxisColor);
    }

    return selected;
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