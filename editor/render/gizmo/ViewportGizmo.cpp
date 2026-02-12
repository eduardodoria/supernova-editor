#include "ViewportGizmo.h"

using namespace Supernova;

Editor::ViewportGizmo::ViewportGizmo(){
    scene = new Scene(EntityPool::System);
    camera = new Camera(scene);
    mainObject = new Object(scene);
    cube = new Shape(scene);
    xaxis = new Shape(scene);
    yaxis = new Shape(scene);
    zaxis = new Shape(scene);
    xarrow = new Shape(scene);
    yarrow = new Shape(scene);
    zarrow = new Shape(scene);

    scene->setBackgroundColor(0.0, 0.0, 0.0, 0.0);
    scene->setCamera(camera);

    cube->createBox(0.6,0.6,0.6);
    cube->setColor(0.5, 0.5, 0.5, 1.0);

    xaxis->createCylinder(0.12, 2);
    yaxis->createCylinder(0.12, 2);
    zaxis->createCylinder(0.12, 2);

    xaxis->setColor(0.7, 0.2, 0.2, 1.0);
    yaxis->setColor(0.2, 0.7, 0.2, 1.0);
    zaxis->setColor(0.2, 0.2, 0.7, 1.0);

    xaxis->setRotation(0,0,90);
    zaxis->setRotation(90,0,0);

    xarrow->createCylinder(0.3, 0.0, 0.6);
    yarrow->createCylinder(0.3, 0.0, 0.6);
    zarrow->createCylinder(0.3, 0.0, 0.6);

    xarrow->setPosition(1.2, 0, 0);
    yarrow->setPosition(0, 1.2, 0);
    zarrow->setPosition(0, 0, 1.2);

    xarrow->setRotation(0,0,-90);
    zarrow->setRotation(90,0,0);

    xarrow->setColor(0.7, 0.2, 0.2, 1.0);
    yarrow->setColor(0.2, 0.7, 0.2, 1.0);
    zarrow->setColor(0.2, 0.2, 0.7, 1.0);

    mainObject->addChild(cube);
    mainObject->addChild(xaxis);
    mainObject->addChild(yaxis);
    mainObject->addChild(zaxis);
    mainObject->addChild(xarrow);
    mainObject->addChild(yarrow);
    mainObject->addChild(zarrow);

    camera->setPosition(0, 0, 5);
    camera->setTarget(0, 0, 0);
    camera->setFramebufferSize(128, 128);
    camera->setRenderToTexture(true);
}

Editor::ViewportGizmo::~ViewportGizmo(){
    delete camera;
    delete mainObject;
    delete cube;
    delete xaxis;
    delete yaxis;
    delete zaxis;
    delete xarrow;
    delete yarrow;
    delete zarrow;

    delete scene;
}

void Editor::ViewportGizmo::applyRotation(Camera* sceneCam){
    Vector3 direction = sceneCam->getWorldDirection();
    Vector3 right = sceneCam->getWorldRight();
    Vector3 up = direction.crossProduct(right);

    mainObject->setRotation(Quaternion(right, up, direction).inverse());
}

Framebuffer* Editor::ViewportGizmo::getFramebuffer(){
    return camera->getFramebuffer();
}

TextureRender& Editor::ViewportGizmo::getTexture(){
    return getFramebuffer()->getRender().getColorTexture();
}

Scene* Editor::ViewportGizmo::getScene(){
    return scene;
}

Object* Editor::ViewportGizmo::getObject(){
    return mainObject;
}