#include "Object2DGizmo.h"

using namespace Supernova;

Editor::Object2DGizmo::Object2DGizmo(Scene* scene): Object(scene){
    Polygon* polygon = new Polygon(scene);

    polygon->addVertex(0, 0);
    polygon->addVertex(10, 0);
    polygon->addVertex(0, 10);
    polygon->addVertex(10, 10);
    polygon->setColor(1.0, 0.3, 0.8, 1.0);
    polygon->setPosition(50, 50, 0);

    this->addChild(polygon);
}

Editor::Object2DGizmo::~Object2DGizmo(){
    delete polygon;
}