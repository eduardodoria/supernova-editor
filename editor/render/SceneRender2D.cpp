#include "SceneRender2D.h"

#include "Project.h"


using namespace Supernova;

Editor::SceneRender2D::SceneRender2D(Scene* scene): SceneRender(scene){
    camera->setType(CameraType::CAMERA_2D);

    lines = new Lines(scene);
    lines->addLine(Vector3(50, 50, 0), Vector3(500, 500, 0), Vector4(0.8, 0.8, 1.0, 1.0));

    scene->setBackgroundColor(Vector4(0.0824, 0.2980, 0.4745, 1.0));

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