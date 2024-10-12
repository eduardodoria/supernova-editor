#include "TranslateGizmo.h"

using namespace Supernova;

const Vector4 Editor::TranslateGizmo::centerColor = Vector4(1.0, 0.5, 0.2, 1.0);
const Vector4 Editor::TranslateGizmo::xaxisColor = Vector4(0.7, 0.2, 0.2, 1.0);
const Vector4 Editor::TranslateGizmo::yaxisColor = Vector4(0.2, 0.7, 0.2, 1.0);
const Vector4 Editor::TranslateGizmo::zaxisColor = Vector4(0.2, 0.2, 0.7, 1.0);
const Vector4 Editor::TranslateGizmo::centerColorHightlight = Vector4(0.7, 0.7, 1.0, 1.0);
const Vector4 Editor::TranslateGizmo::xaxisColorHightlight = Vector4(0.9, 0.7, 0.7, 1.0);
const Vector4 Editor::TranslateGizmo::yaxisColorHightlight = Vector4(0.7, 0.9, 0.7, 1.0);
const Vector4 Editor::TranslateGizmo::zaxisColorHightlight = Vector4(0.7, 0.7, 0.9, 1.0);

Editor::TranslateGizmo::TranslateGizmo(Scene* scene): Object(scene){
    float cylinderRadius = 0.05;
    float cylinderHeight = 2;
    float arrowRadius = 0.1;
    float arrowHeight = 0.4;

    sphere = new Shape(scene);
    xaxis = new Shape(scene);
    yaxis = new Shape(scene);
    zaxis = new Shape(scene);
    xarrow = new Shape(scene);
    yarrow = new Shape(scene);
    zarrow = new Shape(scene);
    
    sphere->createSphere(0.2);
    sphere->setColor(centerColor);

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

    xarrow->setColor(xaxisColor);
    yarrow->setColor(yaxisColor);
    zarrow->setColor(zaxisColor);

    this->addChild(sphere);
    this->addChild(xaxis);
    this->addChild(yaxis);
    this->addChild(zaxis);
    this->addChild(xarrow);
    this->addChild(yarrow);
    this->addChild(zarrow);
}

Editor::GizmoSideSelected Editor::TranslateGizmo::checkHoverHighlight(Ray& ray){
    bool sphereSelected = false;

    AABB sphereaabb = sphere->getModelMatrix() * Matrix4::scaleMatrix(Vector3(2,2,2)) * sphere->getAABB();

    Editor::GizmoSideSelected gizmoSideSelected = GizmoSideSelected::NONE;

    if (ray.intersects(sphereaabb)){
        sphere->setColor(centerColorHightlight);
        sphereSelected = true;
        gizmoSideSelected = GizmoSideSelected::XYZ;
    }else{
        sphere->setColor(centerColor);
    }

    AABB xaabb = xaxis->getModelMatrix() * Matrix4::scaleMatrix(Vector3(2,1,2)) * xaxis->getAABB().merge(xarrow->getAABB());
    AABB yaabb = yaxis->getModelMatrix() * Matrix4::scaleMatrix(Vector3(2,1,2)) * yaxis->getAABB().merge(yarrow->getAABB());
    AABB zaabb = zaxis->getModelMatrix() * Matrix4::scaleMatrix(Vector3(2,1,2)) * zaxis->getAABB().merge(zarrow->getAABB());

    if (!sphereSelected && ray.intersects(xaabb)){
        xaxis->setColor(xaxisColorHightlight);
        xarrow->setColor(xaxisColorHightlight);
        gizmoSideSelected = GizmoSideSelected::X;
    }else{
        xaxis->setColor(xaxisColor);
        xarrow->setColor(xaxisColor);
    }

    if (!sphereSelected && ray.intersects(yaabb)){
        yaxis->setColor(yaxisColorHightlight);
        yarrow->setColor(yaxisColorHightlight);
        gizmoSideSelected = GizmoSideSelected::Y;
    }else{
        yaxis->setColor(yaxisColor);
        yarrow->setColor(yaxisColor);
    }

    if (!sphereSelected && ray.intersects(zaabb)){
        zaxis->setColor(zaxisColorHightlight);
        zarrow->setColor(zaxisColorHightlight);
        gizmoSideSelected = GizmoSideSelected::Z;
    }else{
        zaxis->setColor(zaxisColor);
        zarrow->setColor(zaxisColor);
    }

    return gizmoSideSelected;
}