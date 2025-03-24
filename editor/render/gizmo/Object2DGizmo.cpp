#include "Object2DGizmo.h"

using namespace Supernova;

Editor::Object2DGizmo::Object2DGizmo(Scene* scene): Object(scene){
    float rectSize = 10;
    float width = 100;
    float height = 100;

    for (int i = 0; i < 8; i++){
        rects[i] = new Polygon(scene);

        rects[i]->addVertex(0, 0);
        rects[i]->addVertex(rectSize, 0);
        rects[i]->addVertex(0, rectSize);
        rects[i]->addVertex(rectSize, rectSize);
        rects[i]->setColor(1.0, 0.3, 0.8, 1.0);

        this->addChild(rects[i]);
    }

    float halfRect = rectSize / 2.0;
    float halfWidth = width / 2.0;
    float halfHeight = height / 2.0;

    rects[0]->setPosition(-halfWidth-halfRect, -halfHeight-halfRect, 0);
    rects[1]->setPosition(-halfWidth-halfRect, -halfRect, 0);
    rects[2]->setPosition(-halfWidth-halfRect, halfHeight-halfRect, 0);
    rects[3]->setPosition(-halfRect, halfHeight-halfRect, 0);
    rects[4]->setPosition(halfWidth-halfRect, halfHeight-halfRect, 0);
    rects[5]->setPosition(halfWidth-halfRect, -halfRect, 0);
    rects[6]->setPosition(halfWidth-halfRect, -halfHeight-halfRect, 0);
    rects[7]->setPosition(-halfRect, -halfHeight-halfRect, 0);
}

Editor::Object2DGizmo::~Object2DGizmo(){
    for (int i = 0; i < 8; i++){
        delete rects[i];
    }
}