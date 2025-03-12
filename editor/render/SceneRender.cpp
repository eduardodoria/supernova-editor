#include "SceneRender.h"

using namespace Supernova;

Editor::SceneRender::SceneRender(Scene* scene){
    this->scene = scene;
    camera = new Camera(scene);

    scene->setCamera(camera);
}

Editor::SceneRender::~SceneRender(){
    framebuffer.destroy();

    delete camera;
}

void Editor::SceneRender::activate(){
    Engine::setFramebuffer(&framebuffer);
    Engine::setScene(scene);

    Engine::removeAllSceneLayers();
}

void Editor::SceneRender::updateSize(int width, int height){

}

void Editor::SceneRender::updateRenderSystem(){
    scene->getSystem<RenderSystem>()->update(0);
}

TextureRender& Editor::SceneRender::getTexture(){
    //return camera->getFramebuffer()->getRender().getColorTexture();
    return framebuffer.getRender().getColorTexture();
}

Camera* Editor::SceneRender::getCamera(){
    return camera;
}

bool Editor::SceneRender::isUseGlobalTransform() const{
    return useGlobalTransform;
}

void Editor::SceneRender::setUseGlobalTransform(bool useGlobalTransform){
    this->useGlobalTransform = useGlobalTransform;
}

void Editor::SceneRender::changeUseGlobalTransform(){
    this->useGlobalTransform = !this->useGlobalTransform;
}