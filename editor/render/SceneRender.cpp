#include "SceneRender.h"

using namespace Supernova;

Editor::SceneRender::SceneRender(Scene* scene){
    this->scene = scene;

    Lines* lines = new Lines(scene);
    camera = new Camera(scene);

    int gridHeight = 0;

    for (int i = -1000; i <= 1000; i++){
        if (i == 0){
            lines->addLine(Vector3(i, gridHeight, -1000), Vector3(i, gridHeight, 1000), Vector4(0.5, 0.5, 1.0, 1.0));
        }else{
            if (i % 10 == 0){
                lines->addLine(Vector3(i, gridHeight, -1000), Vector3(i, gridHeight, 1000), Vector4(0.5, 0.5, 0.5, 1.0));
            }else{
                lines->addLine(Vector3(i, gridHeight, -1000), Vector3(i, gridHeight, 1000), Vector4(0.5, 0.5, 0.5, 0.5));
            }
        }
    }
    for (int i = -1000; i <= 1000; i++){
        if (i == 0){
            lines->addLine(Vector3(-1000, gridHeight, i), Vector3(1000, gridHeight, i), Vector4(1.0, 0.5, 0.5, 1.0));
        }else{
            if (i % 10 == 0){
                lines->addLine(Vector3(-1000, gridHeight, i), Vector3(1000, gridHeight, i), Vector4(0.5, 0.5, 0.5, 1.0));
            }else{
                lines->addLine(Vector3(-1000, gridHeight, i), Vector3(1000, gridHeight, i), Vector4(0.5, 0.5, 0.5, 0.5));
            }
        }
    }
    lines->addLine(Vector3(0, -1000, 0), Vector3(0, 1000, 0), Vector4(0.5, 1.0, 0.5, 1.0));

    //camera->setType(CameraType::CAMERA_2D);
    camera->setPosition(10, 4, 10);
    camera->setView(0, 0, 0);

    camera->setRenderToTexture(true);
    camera->setUseFramebufferSizes(false);

    scene->setCamera(camera);
    scene->setBackgroundColor(Vector4(0.25, 0.45, 0.65, 1.0));

    Engine::setFixedTimeSceneUpdate(false);
}

void Editor::SceneRender::activate(){
    Engine::setScene(scene);
    Engine::removeAllSceneLayers();
    Engine::addSceneLayer(gimbal.getScene());
}

void Editor::SceneRender::update(int width, int height){
    if (width != 0 && height != 0){
        camera->setFramebufferSize(width, height);
    }

    gimbal.applyRotation(camera);
}

TextureRender& Editor::SceneRender::getTexture(){
    return camera->getFramebuffer()->getRender().getColorTexture();
}

Camera* Editor::SceneRender::getCamera(){
    return camera;
}

Editor::Gimbal* Editor::SceneRender::getGimbal(){
    return &gimbal;
}