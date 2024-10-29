#include "TranslateGizmo.h"

using namespace Supernova;

const Vector3 Editor::TranslateGizmo::centerColor = Vector3(0.8, 0.8, 0.8);
const Vector3 Editor::TranslateGizmo::xaxisColor = Vector3(0.7, 0.2, 0.2);
const Vector3 Editor::TranslateGizmo::yaxisColor = Vector3(0.2, 0.7, 0.2);
const Vector3 Editor::TranslateGizmo::zaxisColor = Vector3(0.2, 0.2, 0.7);
const Vector3 Editor::TranslateGizmo::centerColorHightlight = Vector3(0.9, 0.9, 0.9);
const Vector3 Editor::TranslateGizmo::xaxisColorHightlight = Vector3(0.9, 0.7, 0.7);
const Vector3 Editor::TranslateGizmo::yaxisColorHightlight = Vector3(0.7, 0.9, 0.7);
const Vector3 Editor::TranslateGizmo::zaxisColorHightlight = Vector3(0.7, 0.7, 0.9);
const float Editor::TranslateGizmo::rectAlpha = 0.6;

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
    xyrect = new Shape(scene);
    xzrect = new Shape(scene);
    yzrect = new Shape(scene);
    
    sphere->createSphere(0.2);
    sphere->setColor(Vector4(centerColor, 1.0));

    xaxis->createCylinder(cylinderRadius, cylinderHeight);
    yaxis->createCylinder(cylinderRadius, cylinderHeight);
    zaxis->createCylinder(cylinderRadius, cylinderHeight);

    xaxis->setColor(Vector4(xaxisColor, 1.0));
    yaxis->setColor(Vector4(yaxisColor, 1.0));
    zaxis->setColor(Vector4(zaxisColor, 1.0));

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

    xarrow->setColor(Vector4(xaxisColor, 1.0));
    yarrow->setColor(Vector4(yaxisColor, 1.0));
    zarrow->setColor(Vector4(zaxisColor, 1.0));

    xyrect->createBox(cylinderHeight/4.0, cylinderHeight/4.0, cylinderRadius);
    xzrect->createBox(cylinderHeight/4.0, cylinderRadius, cylinderHeight/4.0);
    yzrect->createBox(cylinderRadius, cylinderHeight/4.0, cylinderHeight/4.0);

    xyrect->setPosition(cylinderHeight/3.0, cylinderHeight/3.0, 0);
    xzrect->setPosition(cylinderHeight/3.0, 0, cylinderHeight/3.0);
    yzrect->setPosition(0, cylinderHeight/3.0, cylinderHeight/3.0);

    xyrect->setColor(Vector4(zaxisColor, rectAlpha));
    xzrect->setColor(Vector4(yaxisColor, rectAlpha));
    yzrect->setColor(Vector4(xaxisColor, rectAlpha));

    this->addChild(sphere);
    this->addChild(xaxis);
    this->addChild(yaxis);
    this->addChild(zaxis);
    this->addChild(xarrow);
    this->addChild(yarrow);
    this->addChild(zarrow);
    this->addChild(xyrect);
    this->addChild(xzrect);
    this->addChild(yzrect);
}

Editor::GizmoSideSelected Editor::TranslateGizmo::checkHoverHighlight(Ray& ray){

    Editor::GizmoSideSelected gizmoSideSelected = GizmoSideSelected::NONE;

    AABB xaabb = xaxis->getModelMatrix() * Matrix4::scaleMatrix(Vector3(2, 1, 2)) * xaxis->getAABB().merge(xarrow->getAABB());
    AABB yaabb = yaxis->getModelMatrix() * Matrix4::scaleMatrix(Vector3(2, 1, 2)) * yaxis->getAABB().merge(yarrow->getAABB());
    AABB zaabb = zaxis->getModelMatrix() * Matrix4::scaleMatrix(Vector3(2, 1, 2)) * zaxis->getAABB().merge(zarrow->getAABB());
    AABB xyaabb = xyrect->getWorldAABB();
    AABB xzaabb = xzrect->getWorldAABB();
    AABB yzaabb = yzrect->getWorldAABB();
    AABB sphereaabb = sphere->getModelMatrix() * Matrix4::scaleMatrix(Vector3(1.5, 1.5, 1.5)) * sphere->getAABB();

    RayReturn rreturn[7];

    rreturn[0] = ray.intersects(xaabb);
    rreturn[1] = ray.intersects(yaabb);
    rreturn[2] = ray.intersects(zaabb);
    rreturn[3] = ray.intersects(xyaabb);
    rreturn[4] = ray.intersects(xzaabb);
    rreturn[5] = ray.intersects(yzaabb);
    rreturn[6] = ray.intersects(sphereaabb);

    int axis = -1;
    float minDist = FLT_MAX;
    for (int i = 0; i < 7; i++){
        if (rreturn[i]){
            if (rreturn[i].distance <= minDist || (i >= 3 && axis <= 2)){
                minDist = rreturn[i].distance;
                axis = i;
            }
        }
    }

    if (axis == 0){
        xaxis->setColor(Vector4(xaxisColorHightlight, 1.0));
        xarrow->setColor(Vector4(xaxisColorHightlight, 1.0));
        gizmoSideSelected = GizmoSideSelected::X;
    }else{
        xaxis->setColor(Vector4(xaxisColor, 1.0));
        xarrow->setColor(Vector4(xaxisColor, 1.0));
    }

    if (axis == 1){
        yaxis->setColor(Vector4(yaxisColorHightlight, 1.0));
        yarrow->setColor(Vector4(yaxisColorHightlight, 1.0));
        gizmoSideSelected = GizmoSideSelected::Y;
    }else{
        yaxis->setColor(Vector4(yaxisColor, 1.0));
        yarrow->setColor(Vector4(yaxisColor, 1.0));
    }

    if (axis == 2){
        zaxis->setColor(Vector4(zaxisColorHightlight, 1.0));
        zarrow->setColor(Vector4(zaxisColorHightlight, 1.0));
        gizmoSideSelected = GizmoSideSelected::Z;
    }else{
        zaxis->setColor(Vector4(zaxisColor, 1.0));
        zarrow->setColor(Vector4(zaxisColor, 1.0));
    }

    if (axis == 3){
        xyrect->setColor(Vector4(zaxisColorHightlight, rectAlpha));
        gizmoSideSelected = GizmoSideSelected::XY;
    }else{
        xyrect->setColor(Vector4(zaxisColor, rectAlpha));
    }

    if (axis == 4){
        xzrect->setColor(Vector4(yaxisColorHightlight, rectAlpha));
        gizmoSideSelected = GizmoSideSelected::XZ;
    }else{
        xzrect->setColor(Vector4(yaxisColor, rectAlpha));
    }

    if (axis == 5){
        yzrect->setColor(Vector4(xaxisColorHightlight, rectAlpha));
        gizmoSideSelected = GizmoSideSelected::YZ;
    }else{
        yzrect->setColor(Vector4(xaxisColor, rectAlpha));
    }

    if (axis == 6){
        sphere->setColor(Vector4(centerColorHightlight, 1.0));
        gizmoSideSelected = GizmoSideSelected::XYZ;
    }else{
        sphere->setColor(Vector4(centerColor, 1.0));
    }

    return gizmoSideSelected;
}