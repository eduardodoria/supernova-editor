#include "SceneRender2D.h"

#include "Project.h"


using namespace Supernova;

Editor::SceneRender2D::SceneRender2D(Scene* scene, unsigned int width, unsigned int height, bool isUI): SceneRender(scene, true, false, 40, 2){
    this->isUI = isUI;

    if (isUI){
        camera->setType(CameraType::CAMERA_UI);
    }else{
        camera->setType(CameraType::CAMERA_ORTHO);
    }

    camera->slide(-50);
    camera->slideUp(-50);

    scene->setDefaultEntityPool(EntityPool::System);
    lines = new Lines(scene);
    scene->setDefaultEntityPool(EntityPool::User);

    createLines(width, height);

    scene->setLightState(LightState::OFF);

    if (isUI){
        scene->setBackgroundColor(Vector4(0.525, 0.525, 0.525, 1.0));
    }else{
        scene->setBackgroundColor(Vector4(0.231, 0.298, 0.475, 1.0));
    }

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

void Editor::SceneRender2D::hideAllGizmos(){
    SceneRender::hideAllGizmos();

    lines->setVisible(false);
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

void Editor::SceneRender2D::updateSelLines(std::vector<OBB> obbs){
    Vector4 color = Vector4(1.0, 0.6, 0.0, 1.0);

    if (selLines->getNumLines() != obbs.size() * 4){
        selLines->clearLines();
        for (OBB& obb : obbs){
            selLines->addLine(obb.getCorner(OBB::NEAR_LEFT_BOTTOM), obb.getCorner(OBB::NEAR_LEFT_TOP), color);
            selLines->addLine(obb.getCorner(OBB::NEAR_LEFT_TOP), obb.getCorner(OBB::NEAR_RIGHT_TOP), color);
            selLines->addLine(obb.getCorner(OBB::NEAR_RIGHT_TOP), obb.getCorner(OBB::NEAR_RIGHT_BOTTOM), color);
            selLines->addLine(obb.getCorner(OBB::NEAR_RIGHT_BOTTOM), obb.getCorner(OBB::NEAR_LEFT_BOTTOM), color);
        }
    }else{
        int i = 0;
        for (OBB& obb : obbs){
            selLines->updateLine(i * 4 + 0, obb.getCorner(OBB::NEAR_LEFT_BOTTOM), obb.getCorner(OBB::NEAR_LEFT_TOP));
            selLines->updateLine(i * 4 + 1, obb.getCorner(OBB::NEAR_LEFT_TOP), obb.getCorner(OBB::NEAR_RIGHT_TOP));
            selLines->updateLine(i * 4 + 2, obb.getCorner(OBB::NEAR_RIGHT_TOP), obb.getCorner(OBB::NEAR_RIGHT_BOTTOM));
            selLines->updateLine(i * 4 + 3, obb.getCorner(OBB::NEAR_RIGHT_BOTTOM), obb.getCorner(OBB::NEAR_LEFT_BOTTOM));
            i++;
        }
    }
}

void Editor::SceneRender2D::update(std::vector<Entity> selEntities, std::vector<Entity> entities, Entity mainCamera){
    SceneRender::update(selEntities, entities, mainCamera);

    if (isPlaying){
        return;
    }

    lines->setVisible(true);
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

void Editor::SceneRender2D::mouseDragEvent(float x, float y, float origX, float origY, Project* project, size_t sceneId, std::vector<Entity> selEntities, bool disableSelection){
    SceneRender::mouseDragEvent(x, y, origX, origY, project, sceneId, selEntities, disableSelection);
}

void Editor::SceneRender2D::zoomAtPosition(float width, float height, Vector2 pos, float zoomFactor){
    float left = camera->getLeftClip();
    float right = camera->getRightClip();
    float bottom = camera->getBottomClip();
    float top = camera->getTopClip();

    float worldX = left + (pos.x / width) * (right - left);
    float worldY = bottom + (pos.y / height) * (top - bottom);

    float currentWidth = right - left;
    float currentZoom = currentWidth / width; // units per pixel

    float newZoom = currentZoom * zoomFactor;

    float newWidth = width * newZoom;
    float newHeight = height * newZoom;

    float newLeft = worldX - (pos.x / width) * newWidth;
    float newRight = newLeft + newWidth;
    float newBottom = worldY - (pos.y / height) * newHeight;
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