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

    scene->setCamera(camera);
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