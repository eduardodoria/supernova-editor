#include "UILayer.h"

using namespace Supernova;

Editor::UILayer::UILayer(){
    scene = new Scene();
    camera = new Camera(scene);
    viewGizmoImage = new Image(scene);
    
    camera->setType(CameraType::CAMERA_ORTHO);

    viewGizmoImage->setSize(100, 100);

    scene->setCamera(camera);
}

void Editor::UILayer::setViewportGizmoTexture(Framebuffer* framebuffer){
    viewGizmoImage->setTexture(framebuffer);
}

void Editor::UILayer::updateSize(int width, int height){
    //camera->setFramebufferSize(width, height);
    // not needed because setScalingMode(Scaling::NATIVE)
    //camera->setOrtho(0, width, 0, height, DEFAULT_ORTHO_NEAR, DEFAULT_ORTHO_FAR);

    viewGizmoImage->setPosition(width - viewGizmoImage->getWidth(), height - viewGizmoImage->getHeight());
}

Framebuffer* Editor::UILayer::getFramebuffer(){
    return camera->getFramebuffer();
}

TextureRender& Editor::UILayer::getTexture(){
    return getFramebuffer()->getRender().getColorTexture();
}

Camera* Editor::UILayer::getCamera(){
    return camera;
}

Scene* Editor::UILayer::getScene(){
    return scene;
}