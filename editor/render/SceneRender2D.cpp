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

    for (auto& pair : bodyLines) {
        delete pair.second;
    }
    bodyLines.clear();
}

bool Editor::SceneRender2D::instanciateBodyLines(Entity entity){
    if (bodyLines.find(entity) == bodyLines.end()){
        ScopedDefaultEntityPool sys(*scene, EntityPool::System);
        bodyLines[entity] = new Lines(scene);

        return true;
    }

    return false;
}

void Editor::SceneRender2D::createOrUpdateBodyLines(Entity entity, const Transform& transform, const Body2DComponent& body){
    Lines* bodyLinesObj = bodyLines[entity];

    bodyLinesObj->clearLines();
    bodyLinesObj->setVisible(transform.visible);

    if (!transform.visible || body.numShapes == 0){
        return;
    }

    const Vector4 bodyColor(0.2f, 0.95f, 0.95f, 1.0f);
    const Matrix4 modelMatrix = transform.modelMatrix;

    auto toWorld = [&](const Vector2& point){
        return modelMatrix * Vector3(point.x, point.y, 0.0f);
    };

    auto addCircle = [&](const Vector2& center, float radius, int segments){
        if (radius <= 0.0f){
            return;
        }

        for (int i = 0; i < segments; i++){
            float a0 = (2.0f * M_PI * i) / segments;
            float a1 = (2.0f * M_PI * (i + 1)) / segments;

            Vector2 p0 = center + Vector2(std::cos(a0), std::sin(a0)) * radius;
            Vector2 p1 = center + Vector2(std::cos(a1), std::sin(a1)) * radius;

            bodyLinesObj->addLine(toWorld(p0), toWorld(p1), bodyColor);
        }
    };

    auto addArc = [&](const Vector2& center, float radius, float startAngle, float endAngle, int segments){
        if (radius <= 0.0f){
            return;
        }

        for (int i = 0; i < segments; i++){
            float t0 = (float)i / (float)segments;
            float t1 = (float)(i + 1) / (float)segments;
            float a0 = startAngle + (endAngle - startAngle) * t0;
            float a1 = startAngle + (endAngle - startAngle) * t1;

            Vector2 p0 = center + Vector2(std::cos(a0), std::sin(a0)) * radius;
            Vector2 p1 = center + Vector2(std::cos(a1), std::sin(a1)) * radius;

            bodyLinesObj->addLine(toWorld(p0), toWorld(p1), bodyColor);
        }
    };

    auto addRoundedRect = [&](const Vector2& a, const Vector2& b, float radius){
        Vector2 minPt(std::min(a.x, b.x), std::min(a.y, b.y));
        Vector2 maxPt(std::max(a.x, b.x), std::max(a.y, b.y));

        const float width = maxPt.x - minPt.x;
        const float height = maxPt.y - minPt.y;
        if (width <= 0.0f || height <= 0.0f){
            return;
        }

        const float clampedRadius = std::max(0.0f, std::min(radius, std::min(width, height) * 0.5f));
        if (clampedRadius <= 0.0f){
            Vector2 p0(minPt.x, minPt.y);
            Vector2 p1(maxPt.x, minPt.y);
            Vector2 p2(maxPt.x, maxPt.y);
            Vector2 p3(minPt.x, maxPt.y);

            bodyLinesObj->addLine(toWorld(p0), toWorld(p1), bodyColor);
            bodyLinesObj->addLine(toWorld(p1), toWorld(p2), bodyColor);
            bodyLinesObj->addLine(toWorld(p2), toWorld(p3), bodyColor);
            bodyLinesObj->addLine(toWorld(p3), toWorld(p0), bodyColor);
            return;
        }

        const float x0 = minPt.x;
        const float y0 = minPt.y;
        const float x1 = maxPt.x;
        const float y1 = maxPt.y;
        const float r = clampedRadius;

        bodyLinesObj->addLine(toWorld(Vector2(x0 + r, y0)), toWorld(Vector2(x1 - r, y0)), bodyColor);
        bodyLinesObj->addLine(toWorld(Vector2(x1, y0 + r)), toWorld(Vector2(x1, y1 - r)), bodyColor);
        bodyLinesObj->addLine(toWorld(Vector2(x1 - r, y1)), toWorld(Vector2(x0 + r, y1)), bodyColor);
        bodyLinesObj->addLine(toWorld(Vector2(x0, y1 - r)), toWorld(Vector2(x0, y0 + r)), bodyColor);

        addArc(Vector2(x0 + r, y0 + r), r, M_PI, 1.5f * M_PI, 8);
        addArc(Vector2(x1 - r, y0 + r), r, 1.5f * M_PI, 2.0f * M_PI, 8);
        addArc(Vector2(x1 - r, y1 - r), r, 0.0f, 0.5f * M_PI, 8);
        addArc(Vector2(x0 + r, y1 - r), r, 0.5f * M_PI, M_PI, 8);
    };

    for (size_t i = 0; i < body.numShapes; i++){
        const Shape2D& shape = body.shapes[i];

        if (shape.type == Shape2DType::POLYGON){
            if (shape.verticesCount >= 3){
                if (shape.radius > 0.0f){
                    addRoundedRect(shape.pointA, shape.pointB, shape.radius);
                }else{
                    for (size_t j = 0; j < shape.verticesCount; j++){
                        const Vector2& p0 = shape.vertices[j];
                        const Vector2& p1 = shape.vertices[(j + 1) % shape.verticesCount];
                        bodyLinesObj->addLine(toWorld(p0), toWorld(p1), bodyColor);
                    }
                }
            }else{
                Vector2 minPt(std::min(shape.pointA.x, shape.pointB.x), std::min(shape.pointA.y, shape.pointB.y));
                Vector2 maxPt(std::max(shape.pointA.x, shape.pointB.x), std::max(shape.pointA.y, shape.pointB.y));

                Vector2 p0(minPt.x, minPt.y);
                Vector2 p1(maxPt.x, minPt.y);
                Vector2 p2(maxPt.x, maxPt.y);
                Vector2 p3(minPt.x, maxPt.y);

                bodyLinesObj->addLine(toWorld(p0), toWorld(p1), bodyColor);
                bodyLinesObj->addLine(toWorld(p1), toWorld(p2), bodyColor);
                bodyLinesObj->addLine(toWorld(p2), toWorld(p3), bodyColor);
                bodyLinesObj->addLine(toWorld(p3), toWorld(p0), bodyColor);
            }
        }else if (shape.type == Shape2DType::CIRCLE){
            addCircle(shape.pointA, shape.radius, 24);
        }else if (shape.type == Shape2DType::CAPSULE){
            Vector2 pointA = shape.pointA;
            Vector2 pointB = shape.pointB;
            float radius = shape.radius;

            if (radius <= 0.0f){
                continue;
            }

            Vector2 axis = pointB - pointA;
            if (axis.length() <= 0.0f){
                addCircle(pointA, radius, 24);
                continue;
            }

            axis.normalize();
            Vector2 perp(-axis.y, axis.x);

            Vector2 sideA1 = pointA + perp * radius;
            Vector2 sideB1 = pointB + perp * radius;
            Vector2 sideA2 = pointA - perp * radius;
            Vector2 sideB2 = pointB - perp * radius;

            bodyLinesObj->addLine(toWorld(sideA1), toWorld(sideB1), bodyColor);
            bodyLinesObj->addLine(toWorld(sideA2), toWorld(sideB2), bodyColor);

            float baseAngle = std::atan2(axis.y, axis.x);
            addArc(pointA, radius, baseAngle + 0.5f * M_PI, baseAngle + 1.5f * M_PI, 12);
            addArc(pointB, radius, baseAngle - 0.5f * M_PI, baseAngle + 0.5f * M_PI, 12);
        }else if (shape.type == Shape2DType::SEGMENT){
            bodyLinesObj->addLine(toWorld(shape.pointA), toWorld(shape.pointB), bodyColor);
        }else if (shape.type == Shape2DType::CHAIN){
            if (shape.verticesCount >= 2){
                for (size_t j = 0; j < shape.verticesCount - 1; j++){
                    bodyLinesObj->addLine(toWorld(shape.vertices[j]), toWorld(shape.vertices[j + 1]), bodyColor);
                }

                if (shape.loop){
                    bodyLinesObj->addLine(toWorld(shape.vertices[shape.verticesCount - 1]), toWorld(shape.vertices[0]), bodyColor);
                }
            }
        }
    }
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
    for (auto& pair : bodyLines) {
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
    std::set<Entity> currentBodies;
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

        if (signature.test(scene->getComponentId<Body2DComponent>()) && signature.test(scene->getComponentId<Transform>())){
            Body2DComponent& body = scene->getComponent<Body2DComponent>(entity);
            Transform& transform = scene->getComponent<Transform>(entity);

            currentBodies.insert(entity);
            instanciateBodyLines(entity);
            createOrUpdateBodyLines(entity, transform, body);
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

    auto itBody = bodyLines.begin();
    while (itBody != bodyLines.end()) {
        if (currentBodies.find(itBody->first) == currentBodies.end()) {
            delete itBody->second;
            itBody = bodyLines.erase(itBody);
        } else {
            ++itBody;
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