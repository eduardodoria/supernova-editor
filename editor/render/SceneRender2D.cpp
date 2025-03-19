#include "SceneRender2D.h"

#include "Project.h"


using namespace Supernova;

Editor::SceneRender2D::SceneRender2D(Scene* scene, unsigned int width, unsigned int height): uilayer(false), SceneRender(scene){
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

    zoom = 1.0f;

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

    Engine::addSceneLayer(uilayer.getScene());
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
}

void Editor::SceneRender2D::update(std::vector<Entity> selEntities){
    size_t numTEntities = 0;
    AABB selAABB;

    for (Entity& entity: selEntities){
        if (Transform* transform = scene->findComponent<Transform>(entity)){
            numTEntities++;

            //gizmoPosition += transform->worldPosition;

            //if (!useGlobalTransform && selEntities.size() == 1){
            //    gizmoRotation = transform->worldRotation;
            //}

            selAABB.merge(getFamilyAABB(entity, 1.05));
        }
    }

    bool gizmoVisibility = false;
    if (numTEntities > 0){
        //gizmoPosition /= numTEntities;

        gizmoVisibility = true;

        //float dist = (gizmoPosition - camera->getWorldPosition()).length();
        //float scale = std::tan(cameracomp.yfov) * dist * (gizmoSize / (float)framebuffer.getHeight());

        //toolslayer.updateGizmo(camera, gizmoPosition, gizmoRotation, scale, mouseRay, mouseClicked);

        if (selAABB.isNull() || selAABB.isInfinite()){
            selLines->setVisible(false);
        }else{
            selLines->setVisible(true);

            selLines->updateLine(0, selAABB.getCorner(AABB::NEAR_LEFT_BOTTOM), selAABB.getCorner(AABB::NEAR_LEFT_TOP));
            selLines->updateLine(1, selAABB.getCorner(AABB::NEAR_LEFT_TOP), selAABB.getCorner(AABB::NEAR_RIGHT_TOP));
            selLines->updateLine(2, selAABB.getCorner(AABB::NEAR_RIGHT_TOP), selAABB.getCorner(AABB::NEAR_RIGHT_BOTTOM));
            selLines->updateLine(3, selAABB.getCorner(AABB::NEAR_RIGHT_BOTTOM), selAABB.getCorner(AABB::NEAR_LEFT_BOTTOM));
        }
    }

    selLines->setVisible(gizmoVisibility);
}

void Editor::SceneRender2D::mouseHoverEvent(float x, float y){

}

void Editor::SceneRender2D::mouseClickEvent(float x, float y, std::vector<Entity> selEntities){

}

void Editor::SceneRender2D::mouseReleaseEvent(float x, float y){
    uilayer.setRectVisible(false);
}

void Editor::SceneRender2D::mouseDragEvent(float x, float y, float origX, float origY, size_t sceneId, SceneProject* sceneProject, std::vector<Entity> selEntities, bool disableSelection){
    if (!disableSelection){
        uilayer.setRectVisible(true);
        uilayer.updateRect(Vector2(origX, origY), Vector2(x, y) - Vector2(origX, origY));
    }
}

bool Editor::SceneRender2D::isAnyGizmoSideSelected() const{
    return false;
}

void Editor::SceneRender2D::setZoom(float newZoom) {
    zoom = newZoom;
}

float Editor::SceneRender2D::getZoom() const {
    return zoom;
}