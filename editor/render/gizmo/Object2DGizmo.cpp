#include "Object2DGizmo.h"

using namespace Supernova;

const float Editor::Object2DGizmo::rectSize = 0.20;
const float Editor::Object2DGizmo::sizeOffset = 0.15;

Editor::Object2DGizmo::Object2DGizmo(Scene* scene): Object(scene){
    width = 0.0;
    height = 0.0;

    for (int i = 0; i < 8; i++){
        rects[i] = new Polygon(scene);

        rects[i]->addVertex(0, 0);
        rects[i]->addVertex(rectSize, 0);
        rects[i]->addVertex(0, rectSize);
        rects[i]->addVertex(rectSize, rectSize);
        rects[i]->setColor(0.9, 0.5, 0.3, 1.0);

        this->addChild(rects[i]);
    }
}

Editor::Object2DGizmo::~Object2DGizmo(){
    for (int i = 0; i < 8; i++){
        delete rects[i];
    }
}

void Editor::Object2DGizmo::updateRects(){
    float halfRect = rectSize / 2.0;
    float halfWidth = width / 2.0;
    float halfHeight = height / 2.0;

    rects[0]->setPosition(-halfWidth-halfRect-sizeOffset, -halfHeight-halfRect-sizeOffset, 0);
    rects[1]->setPosition(-halfWidth-halfRect-sizeOffset, -halfRect, 0);
    rects[2]->setPosition(-halfWidth-halfRect-sizeOffset, halfHeight-halfRect+sizeOffset, 0);
    rects[3]->setPosition(-halfRect, halfHeight-halfRect+sizeOffset, 0);
    rects[4]->setPosition(halfWidth-halfRect+sizeOffset, halfHeight-halfRect+sizeOffset, 0);
    rects[5]->setPosition(halfWidth-halfRect+sizeOffset, -halfRect, 0);
    rects[6]->setPosition(halfWidth-halfRect+sizeOffset, -halfHeight-halfRect-sizeOffset, 0);
    rects[7]->setPosition(-halfRect, -halfHeight-halfRect-sizeOffset, 0);
}

void Editor::Object2DGizmo::setSize(float width, float height){
    if (this->width != width || this->height != height){
        this->width = width;
        this->height = height;

        updateRects();
    }
}

Editor::Gizmo2DSideSelected Editor::Object2DGizmo::checkHover(const Ray& ray, const AABB& aabb){
    Editor::Gizmo2DSideSelected gizmoSideSelected = Gizmo2DSideSelected::NONE;

    if (RayReturn rreturn = ray.intersects(aabb)){
        gizmoSideSelected = Gizmo2DSideSelected::CENTER;
    }

    for (int i = 0; i < 8; i++){
        if (RayReturn rreturn = ray.intersects(rects[i]->getWorldAABB())){
            if (i == 0){
                gizmoSideSelected = Gizmo2DSideSelected::NX_NY;
            } else if (i == 1){
                gizmoSideSelected = Gizmo2DSideSelected::NX;
            } else if (i == 2){
                gizmoSideSelected = Gizmo2DSideSelected::NX_PY;
            } else if (i == 3){
                gizmoSideSelected = Gizmo2DSideSelected::PY;
            } else if (i == 4){
                gizmoSideSelected = Gizmo2DSideSelected::PX_PY;
            } else if (i == 5){
                gizmoSideSelected = Gizmo2DSideSelected::PX;
            } else if (i == 6){
                gizmoSideSelected = Gizmo2DSideSelected::PX_NY;
            } else if (i == 7){
                gizmoSideSelected = Gizmo2DSideSelected::NY;
            }
        }
    }

    return gizmoSideSelected;
}