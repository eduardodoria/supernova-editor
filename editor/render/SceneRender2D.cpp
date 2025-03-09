#include "SceneRender2D.h"

#include "Project.h"


using namespace Supernova;

Editor::SceneRender2D::SceneRender2D(Scene* scene): SceneRender(scene){
    scene->setCamera(camera);
    scene->setBackgroundColor(Vector4(0.25, 0.45, 0.65, 1.0));

    Engine::setScalingMode(Scaling::NATIVE);
    Engine::setFixedTimeSceneUpdate(false);
}

Editor::SceneRender2D::~SceneRender2D(){

}

void Editor::SceneRender2D::activate(){
    SceneRender::activate();
}

void Editor::SceneRender2D::update(std::vector<Entity> selEntities){

}

void Editor::SceneRender2D::mouseHoverEvent(float x, float y){

}

void Editor::SceneRender2D::mouseClickEvent(float x, float y, std::vector<Entity> selEntities){

}

void Editor::SceneRender2D::mouseReleaseEvent(float x, float y){

}

void Editor::SceneRender2D::mouseDragEvent(float x, float y, float origX, float origY, size_t sceneId, SceneProject* sceneProject, std::vector<Entity> selEntities){

}

bool Editor::SceneRender2D::isAnyGizmoSideSelected() const{
    return false;
}