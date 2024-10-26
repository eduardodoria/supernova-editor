#include "RotateGizmo.h"

using namespace Supernova;

const Vector3 Editor::RotateGizmo::mainColor = Vector3(1.0, 0.5, 0.2);
const Vector3 Editor::RotateGizmo::xaxisColor = Vector3(0.7, 0.2, 0.2);
const Vector3 Editor::RotateGizmo::yaxisColor = Vector3(0.2, 0.7, 0.2);
const Vector3 Editor::RotateGizmo::zaxisColor = Vector3(0.2, 0.2, 0.7);
const Vector3 Editor::RotateGizmo::centerColorHightlight = Vector3(0.7, 0.7, 1.0);
const Vector3 Editor::RotateGizmo::xaxisColorHightlight = Vector3(0.9, 0.7, 0.7);
const Vector3 Editor::RotateGizmo::yaxisColorHightlight = Vector3(0.7, 0.9, 0.7);
const Vector3 Editor::RotateGizmo::zaxisColorHightlight = Vector3(0.7, 0.7, 0.9);
const float Editor::RotateGizmo::circleAlpha = 0.6;

Editor::RotateGizmo::RotateGizmo(Scene* scene): Object(scene){
    maincircle = new Shape(scene);
    xcircle = new Shape(scene);
    ycircle = new Shape(scene);
    zcircle = new Shape(scene);

    maincircle->createTorus(2, 0.02);
    xcircle->createTorus(2, 0.05);
    ycircle->createTorus(2, 0.05);
    zcircle->createTorus(2, 0.05);

    maincircle->setColor(Vector4(mainColor, circleAlpha));
    xcircle->setColor(Vector4(xaxisColor, 1.0));
    ycircle->setColor(Vector4(yaxisColor, 1.0));
    zcircle->setColor(Vector4(zaxisColor, 1.0));

    maincircle->setRotation(90,0,0);
    maincircle->setBillboard(true, false, false);

    ycircle->setRotation(0,0,90);
    zcircle->setRotation(90,0,0);

    this->addChild(maincircle);
    this->addChild(xcircle);
    this->addChild(ycircle);
    this->addChild(zcircle);
}

Editor::GizmoSideSelected Editor::RotateGizmo::checkHoverHighlight(Ray& ray){

    Editor::GizmoSideSelected gizmoSideSelected = GizmoSideSelected::NONE;

    return gizmoSideSelected;
}