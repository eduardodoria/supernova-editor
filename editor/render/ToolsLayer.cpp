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
    gizmoSelected = GizmoSelected::NONE;

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

void Editor::ToolsLayer::updateGizmo(Vector3& position, float scale, Ray& mouseRay, bool mouseClicked){
    gizmo->setPosition(position);
    gizmo->setScale(scale);
    if (!mouseClicked){
        checkHoverHighlight(mouseRay);
    }
}

void Editor::ToolsLayer::checkHoverHighlight(Ray& ray){
    bool sphereSelected = false;

    AABB sphereaabb = sphere->getModelMatrix() * Matrix4::scaleMatrix(Vector3(2,2,2)) * sphere->getAABB();

    gizmoSelected = GizmoSelected::NONE;

    if (ray.intersects(sphereaabb)){
        sphere->setColor(sphereColorHightlight);
        sphereSelected = true;
        gizmoSelected = GizmoSelected::XYZ;
    }else{
        sphere->setColor(sphereColor);
    }

    AABB xaabb = xaxis->getModelMatrix() * Matrix4::scaleMatrix(Vector3(2,1,2)) * xaxis->getAABB().merge(xarrow->getAABB());
    AABB yaabb = yaxis->getModelMatrix() * Matrix4::scaleMatrix(Vector3(2,1,2)) * yaxis->getAABB().merge(yarrow->getAABB());
    AABB zaabb = zaxis->getModelMatrix() * Matrix4::scaleMatrix(Vector3(2,1,2)) * zaxis->getAABB().merge(zarrow->getAABB());

    if (!sphereSelected && ray.intersects(xaabb)){
        xaxis->setColor(xaxisColorHightlight);
        xarrow->setColor(xaxisColorHightlight);
        gizmoSelected = GizmoSelected::X;
    }else{
        xaxis->setColor(xaxisColor);
        xarrow->setColor(xaxisColor);
    }

    if (!sphereSelected && ray.intersects(yaabb)){
        yaxis->setColor(yaxisColorHightlight);
        yarrow->setColor(yaxisColorHightlight);
        gizmoSelected = GizmoSelected::Y;
    }else{
        yaxis->setColor(yaxisColor);
        yarrow->setColor(yaxisColor);
    }

    if (!sphereSelected && ray.intersects(zaabb)){
        zaxis->setColor(zaxisColorHightlight);
        zarrow->setColor(zaxisColorHightlight);
        gizmoSelected = GizmoSelected::Z;
    }else{
        zaxis->setColor(zaxisColor);
        zarrow->setColor(zaxisColor);
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
    return gizmo;
}

Editor::GizmoSelected Editor::ToolsLayer::getGizmoSelected() const{
    return gizmoSelected;
}