#include "MaterialRender.h"

#include "Configs.h"

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

    camera->setPosition(0, 0, 3);
    camera->setTarget(0, 0, 0);
    camera->setType(CameraType::CAMERA_PERSPECTIVE);
    camera->setFramebufferSize(THUMBNAIL_SIZE, THUMBNAIL_SIZE);
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

const Material Editor::MaterialRender::getMaterial(){
    return sphere->getMaterial();
}

Framebuffer* Editor::MaterialRender::getFramebuffer(){
    return camera->getFramebuffer();
}

Texture Editor::MaterialRender::getTexture(){
    return Texture(getFramebuffer());
}

Scene* Editor::MaterialRender::getScene(){
    return scene;
}

Object* Editor::MaterialRender::getObject(){
    return sphere;
}