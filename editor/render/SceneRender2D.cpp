#include "SceneRender2D.h"

#include "Project.h"


using namespace Supernova;

Editor::SceneRender2D::SceneRender2D(Scene* scene, unsigned int width, unsigned int height): SceneRender(scene, false, 40, 1.05){
    camera->setType(CameraType::CAMERA_ORTHO);

    camera->slide(-50);
    camera->slideUp(-50);

    lines = new Lines(scene);

    createLines(width, height);

    selLines = new Lines(scene);
    for (int i = 0; i < 4; i++){
        selLines->addLine(Vector3::ZERO, Vector3::ZERO, Vector4(1.0, 0.6, 0.0, 1.0));
    }
    selLines->setVisible(false);

    scene->setBackgroundColor(Vector4(0.231, 0.298, 0.475, 1.0));

    Engine::setScalingMode(Scaling::NATIVE);
    Engine::setFixedTimeSceneUpdate(false);
}

Editor::SceneRender2D::~SceneRender2D(){

}

void Editor::SceneRender2D::createLines(unsigned int width, unsigned int height){
    lines->clearLines();

    lines->addLine(Vector3(0, -1000000, 0), Vector3(0, 1000000, 0), Vector4(0.2, 0.8, 0.4, 1.0));
    lines->addLine(Vector3(-1000000, 0, 0), Vector3(1000000, 0, 0), Vector4(0.8, 0.2, 0.4, 1.0));

    lines->addLine(Vector3(0, height, 0), Vector3(width, height, 0), Vector4(0.8, 0.8, 0.8, 1.0));
    lines->addLine(Vector3(width, height, 0), Vector3(width, 0, 0), Vector4(0.8, 0.8, 0.8, 1.0));
}

void Editor::SceneRender2D::activate(){
    SceneRender::activate();
}

void Editor::SceneRender2D::updateSize(int width, int height){
    SceneRender::updateSize(width, height);

    float newWidth = width * zoom;
    float newHeight = height * zoom;

    float left = camera->getLeftClip();
    float bottom = camera->getBottomClip();

    float right = left + newWidth;
    float top = bottom + newHeight;

    camera->setLeftClip(left);
    camera->setRightClip(right);
    camera->setBottomClip(bottom);
    camera->setTopClip(top);

    Entity cameraEntity = camera->getEntity();
    CameraComponent& cameracomp = scene->getComponent<CameraComponent>(cameraEntity);
    Transform& cameratransform = scene->getComponent<Transform>(cameraEntity);

    toolslayer.updateCamera(cameracomp, cameratransform);
}

void Editor::SceneRender2D::updateSelLines(AABB aabb){
    selLines->updateLine(0, aabb.getCorner(AABB::NEAR_LEFT_BOTTOM), aabb.getCorner(AABB::NEAR_LEFT_TOP));
    selLines->updateLine(1, aabb.getCorner(AABB::NEAR_LEFT_TOP), aabb.getCorner(AABB::NEAR_RIGHT_TOP));
    selLines->updateLine(2, aabb.getCorner(AABB::NEAR_RIGHT_TOP), aabb.getCorner(AABB::NEAR_RIGHT_BOTTOM));
    selLines->updateLine(3, aabb.getCorner(AABB::NEAR_RIGHT_BOTTOM), aabb.getCorner(AABB::NEAR_LEFT_BOTTOM));
}

void Editor::SceneRender2D::update(std::vector<Entity> selEntities){
    SceneRender::update(selEntities);
}

void Editor::SceneRender2D::mouseHoverEvent(float x, float y){
    SceneRender::mouseHoverEvent(x, y);
}

void Editor::SceneRender2D::mouseClickEvent(float x, float y, std::vector<Entity> selEntities){
    SceneRender::mouseClickEvent(x, y, selEntities);
}

void Editor::SceneRender2D::mouseReleaseEvent(float x, float y){
    SceneRender::mouseReleaseEvent(x, y);
}

void Editor::SceneRender2D::mouseDragEvent(float x, float y, float origX, float origY, size_t sceneId, SceneProject* sceneProject, std::vector<Entity> selEntities, bool disableSelection){
    SceneRender::mouseDragEvent(x, y, origX, origY, sceneId, sceneProject, selEntities, disableSelection);
}

void Editor::SceneRender2D::zoomAtPosition(float width, float height, Vector2 pos, float zoomFactor){
    float left = camera->getLeftClip();
    float right = camera->getRightClip();
    float bottom = camera->getBottomClip();
    float top = camera->getTopClip();

    float worldX = left + (pos.x / width) * (right - left);
    float worldY = bottom + ((height - pos.y) / height) * (top - bottom);

    float currentWidth = right - left;
    float currentZoom = currentWidth / width; // units per pixel

    float newZoom = currentZoom * zoomFactor;

    float newWidth = width * newZoom;
    float newHeight = height * newZoom;

    float newLeft = worldX - (pos.x / width) * newWidth;
    float newRight = newLeft + newWidth;
    float newBottom = worldY - ((height - pos.y) / height) * newHeight;
    float newTop = newBottom + newHeight;

    camera->setLeftClip(newLeft);
    camera->setRightClip(newRight);
    camera->setBottomClip(newBottom);
    camera->setTopClip(newTop);

    camera->setNearClip(-10 * newZoom);
    camera->setFarClip(10 * newZoom);

    if (zoom != newZoom) {
        Entity cameraEntity = camera->getEntity();
        CameraComponent& cameracomp = scene->getComponent<CameraComponent>(cameraEntity);
        Transform& cameratransform = scene->getComponent<Transform>(cameraEntity);

        toolslayer.updateCamera(cameracomp, cameratransform);

        zoom = newZoom;
    }
}

float Editor::SceneRender2D::getZoom() const {
    return zoom;
}