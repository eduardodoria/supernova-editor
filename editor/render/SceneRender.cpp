#include "SceneRender.h"

using namespace Supernova;

Editor::SceneRender::SceneRender(Scene* scene){
    this->scene = scene;
    camera = new Camera(scene);

    scene->setCamera(camera);

    cursorSelected = CursorSelected::POINTER;
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
    // UIs is created in update, without this can affect worldA
    scene->getSystem<UISystem>()->update(0);
    // to avoid gizmos delays
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

void Editor::SceneRender::enableCursorPointer(){
    cursorSelected = CursorSelected::POINTER;
}

void Editor::SceneRender::enableCursorHand(){
    cursorSelected = CursorSelected::HAND;
}

Editor::CursorSelected Editor::SceneRender::getCursorSelected(){
    return cursorSelected;
}