#include "Gizmos.h"

using namespace Supernova;

Editor::Gizmos::Gizmos(){
    scene = new Scene();
    //scene->setBackgroundColor(0.0, 0.0, 0.0, 0.0);

    camera = new Camera(scene);
    camera->setType(CameraType::CAMERA_ORTHO);
    //camera->setPosition(0, 0, 1);
    //camera->setView(0, 0, 0);

    //camera->setRenderToTexture(true);
    //camera->setUseFramebufferSizes(false);
    
    gizmo = new Shape(scene);
    gizmo->createSphere(10);
    gizmo->setColor(1.0, 0.5, 0.2, 1.0);

    gimbalImage = new Image(scene);
    gimbalImage->setSize(100, 100);

    scene->setCamera(camera);
}

void Editor::Gizmos::setGimbalTexture(Framebuffer* framebuffer){
    gimbalImage->setTexture(framebuffer);
}

void Editor::Gizmos::updateSize(int width, int height){
    //camera->setFramebufferSize(width, height);
    camera->setOrtho(0, width, 0, height, DEFAULT_ORTHO_NEAR, DEFAULT_ORTHO_FAR);

    gimbalImage->setPosition(width - gimbalImage->getWidth(), height - gimbalImage->getHeight());
}

Framebuffer* Editor::Gizmos::getFramebuffer(){
    return camera->getFramebuffer();
}

TextureRender& Editor::Gizmos::getTexture(){
    return getFramebuffer()->getRender().getColorTexture();
}

Camera* Editor::Gizmos::getCamera(){
    return camera;
}

Scene* Editor::Gizmos::getScene(){
    return scene;
}

Shape* Editor::Gizmos::getGizmo(){
    return gizmo;
}