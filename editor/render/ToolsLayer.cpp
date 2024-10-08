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

    gimbalImage = new Image(scene);

    //scene->setBackgroundColor(0.0, 0.0, 0.0, 0.0);
    
    camera->setType(CameraType::CAMERA_ORTHO);
    camera->setNear(-200);
    camera->setFar(200);
    //camera->setPosition(0, 0, 1);
    //camera->setView(0, 0, 0);

    //camera->setRenderToTexture(true);
    //camera->setUseFramebufferSizes(false);
    
    sphere->createSphere(5);
    sphere->setColor(1.0, 0.5, 0.2, 1.0);

    xaxis->createCylinder(2, 80);
    yaxis->createCylinder(2, 80);
    zaxis->createCylinder(2, 80);

    xaxis->setColor(0.7, 0.2, 0.2, 1.0);
    yaxis->setColor(0.2, 0.7, 0.2, 1.0);
    zaxis->setColor(0.2, 0.2, 0.7, 1.0);

    xaxis->setPosition(40, 0, 0);
    yaxis->setPosition(0, 40, 0);
    zaxis->setPosition(0, 0, 40);

    xaxis->setRotation(0,0,90);
    zaxis->setRotation(90,0,0);

    xarrow->createCylinder(4, 0.0, 10);
    yarrow->createCylinder(4, 0.0, 10);
    zarrow->createCylinder(4, 0.0, 10);

    xarrow->setPosition(80, 0, 0);
    yarrow->setPosition(0, 80, 0);
    zarrow->setPosition(0, 0, 80);

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


    gimbalImage->setSize(100, 100);

    scene->setCamera(camera);
}

void Editor::ToolsLayer::setGimbalTexture(Framebuffer* framebuffer){
    gimbalImage->setTexture(framebuffer);
}

void Editor::ToolsLayer::updateSize(int width, int height){
    //camera->setFramebufferSize(width, height);
    // not needed because setScalingMode(Scaling::NATIVE)
    //camera->setOrtho(0, width, 0, height, DEFAULT_ORTHO_NEAR, DEFAULT_ORTHO_FAR);

    gimbalImage->setPosition(width - gimbalImage->getWidth(), height - gimbalImage->getHeight());
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