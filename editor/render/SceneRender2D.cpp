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
    for (auto& pair : containerLines) {
        delete pair.second;
    }
    containerLines.clear();
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
    for (auto& pair : containerLines) {
        pair.second->setVisible(false);
    }
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

    std::set<Entity> currentContainers;
    for (Entity& entity: entities){
        Signature signature = scene->getSignature(entity);
        if (signature.test(scene->getComponentId<UIContainerComponent>()) && 
            signature.test(scene->getComponentId<Transform>()) && 
            signature.test(scene->getComponentId<UILayoutComponent>())){

            currentContainers.insert(entity);

            if (containerLines.find(entity) == containerLines.end()){
                ScopedDefaultEntityPool sys(*scene, EntityPool::System);
                containerLines[entity] = new Lines(scene);
            }

            Lines* containerLinesObj = containerLines[entity];
            containerLinesObj->clearLines();

            Transform& transform = scene->getComponent<Transform>(entity);
            UILayoutComponent& layout = scene->getComponent<UILayoutComponent>(entity);
            UIContainerComponent& container = scene->getComponent<UIContainerComponent>(entity);

            if (transform.visible){
                containerLinesObj->setVisible(true);

                Vector4 borderColor(0.55f, 0.35f, 0.85f, 1.0f); // Purple color for container
                Vector4 divColor(0.55f, 0.35f, 0.85f, 0.5f); // Semi-transparent purple for divisions

                Matrix4 modelMatrix = transform.modelMatrix;

                // Container borders
                Vector3 p1 = modelMatrix * Vector3(0, 0, 0);
                Vector3 p2 = modelMatrix * Vector3(layout.width, 0, 0);
                Vector3 p3 = modelMatrix * Vector3(layout.width, layout.height, 0);
                Vector3 p4 = modelMatrix * Vector3(0, layout.height, 0);

                containerLinesObj->addLine(p1, p2, borderColor);
                containerLinesObj->addLine(p2, p3, borderColor);
                containerLinesObj->addLine(p3, p4, borderColor);
                containerLinesObj->addLine(p4, p1, borderColor);

                // Divisions
                for (int b = 0; b < container.numBoxes; b++){
                    if (container.boxes[b].layout != NULL_ENTITY){
                        Rect rect = container.boxes[b].rect;

                        Vector3 bp1 = modelMatrix * Vector3(rect.getX(), rect.getY(), 0);
                        Vector3 bp2 = modelMatrix * Vector3(rect.getX() + rect.getWidth(), rect.getY(), 0);
                        Vector3 bp3 = modelMatrix * Vector3(rect.getX() + rect.getWidth(), rect.getY() + rect.getHeight(), 0);
                        Vector3 bp4 = modelMatrix * Vector3(rect.getX(), rect.getY() + rect.getHeight(), 0);

                        containerLinesObj->addLine(bp1, bp2, divColor);
                        containerLinesObj->addLine(bp2, bp3, divColor);
                        containerLinesObj->addLine(bp3, bp4, divColor);
                        containerLinesObj->addLine(bp4, bp1, divColor);
                    }
                }
            }else{
                containerLinesObj->setVisible(false);
            }
        }
    }

    auto it = containerLines.begin();
    while (it != containerLines.end()) {
        if (currentContainers.find(it->first) == currentContainers.end()) {
            delete it->second;
            it = containerLines.erase(it);
        } else {
            ++it;
        }
    }
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