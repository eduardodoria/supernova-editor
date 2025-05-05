#include "MaterialRender.h"

using namespace Supernova;

Editor::MaterialRender::MaterialRender(){
    scene = new Scene();
    camera = new Camera(scene);
    light = new Light(scene);
    sphere = new Shape(scene);

    sphere->createSphere(1.0);

    scene->setBackgroundColor(0.0, 0.0, 0.0, 0.0);
    scene->setCamera(camera);

    light->setDirection(-0.4, 0.5, -0.5);
    light->setIntensity(6.0);
    light->setType(LightType::DIRECTIONAL);

    scene->setAmbientLight(0.2);

    camera->setPosition(0, 0, 5);
    camera->setTarget(0, 0, 0);
    camera->setType(CameraType::CAMERA_PERSPECTIVE);
    camera->setFramebufferSize(128, 128);
    camera->setRenderToTexture(true);
}

Editor::MaterialRender::~MaterialRender(){
    delete camera;
    delete sphere;

    delete scene;
}

void Editor::MaterialRender::applyMaterial(const Material& material){
    sphere->setMaterial(material);
}

Framebuffer* Editor::MaterialRender::getFramebuffer(){
    return camera->getFramebuffer();
}

TextureRender& Editor::MaterialRender::getTexture(){
    return getFramebuffer()->getRender().getColorTexture();
}

Scene* Editor::MaterialRender::getScene(){
    return scene;
}

Object* Editor::MaterialRender::getObject(){
    return sphere;
}