#include "SceneRender.h"

#include "command/CommandHandle.h"
#include "command/type/ObjectTransformCmd.h"
#include "command/type/PropertyCmd.h"
#include "command/type/MultiPropertyCmd.h"

using namespace Supernova;

Editor::SceneRender::SceneRender(Scene* scene, bool use2DGizmos, bool enableViewGizmo, float gizmoScale, float selectionOffset): toolslayer(use2DGizmos), uilayer(enableViewGizmo){
    this->mouseClicked = false;
    this->lastCommand = nullptr;
    this->useGlobalTransform = true;

    this->gizmoScale = gizmoScale;
    this->selectionOffset = selectionOffset;

    this->scene = scene;
    this->camera = new Camera(scene);

    this->multipleEntitiesSelected = false;

    this->zoom = 1.0f;

    selLines = new Lines(scene);
    selLines->addLine(Vector3::ZERO, Vector3::ZERO, Vector4::ZERO);
    selLines->setVisible(false);

    scene->setCamera(camera);

    cursorSelected = CursorSelected::POINTER;
}

Editor::SceneRender::~SceneRender(){
    framebuffer.destroy();

    delete camera;
}

AABB Editor::SceneRender::getAABB(Entity entity, bool local){
    Signature signature = scene->getSignature(entity);
    if (signature.test(scene->getComponentId<MeshComponent>())){
        MeshComponent& mesh = scene->getComponent<MeshComponent>(entity);
        if (local){
            return mesh.aabb;
        }else{
            return mesh.worldAABB;
        }
    }else if (signature.test(scene->getComponentId<UIComponent>())){
        UIComponent& ui = scene->getComponent<UIComponent>(entity);
        if (local){
            return ui.aabb;
        }else{
            return ui.worldAABB;
        }
    }

    return AABB();
}

AABB Editor::SceneRender::getFamilyAABB(Entity entity, float offset){
    auto transforms = scene->getComponentArray<Transform>();
    size_t index = transforms->getIndex(entity);

    AABB aabb;
    std::vector<Entity> parentList;
    for (int i = index; i < transforms->size(); i++){
        Transform& transform = transforms->getComponentFromIndex(i);

        // Finding childs
        if (i > index){
            if (std::find(parentList.begin(), parentList.end(), transform.parent) == parentList.end()){
                break;
            }
        }

        entity = transforms->getEntity(i);
        parentList.push_back(entity);

        AABB entityAABB = getAABB(entity, false);

        if (!entityAABB.isNull() && !entityAABB.isInfinite()){
            Vector3 min = entityAABB.getMinimum() - Vector3(offset);
            Vector3 max = entityAABB.getMaximum() + Vector3(offset);
            entityAABB.setExtents(min, max);

            aabb.merge(entityAABB);
        }
    }

    return aabb;
}

OBB Editor::SceneRender::getOBB(Entity entity, bool local){
    Signature signature = scene->getSignature(entity);

    Matrix4 modelMatrix;
    if (signature.test(scene->getComponentId<Transform>())){
        Transform& transform = scene->getComponent<Transform>(entity);
        modelMatrix = transform.modelMatrix;
    }

    if (signature.test(scene->getComponentId<MeshComponent>())){
        MeshComponent& mesh = scene->getComponent<MeshComponent>(entity);
        if (local){
            return mesh.aabb.getOBB();
        }else{
            return modelMatrix * mesh.aabb.getOBB();
        }
    }else if (signature.test(scene->getComponentId<UIComponent>())){
        UIComponent& ui = scene->getComponent<UIComponent>(entity);
        if (local){
            return ui.aabb.getOBB();
        }else{
            return modelMatrix * ui.aabb.getOBB();
        }
    }

    return OBB();
}

OBB Editor::SceneRender::getFamilyOBB(Entity entity, float offset){
    auto transforms = scene->getComponentArray<Transform>();
    size_t index = transforms->getIndex(entity);

    OBB obb;
    std::vector<Entity> parentList;
    for (int i = index; i < transforms->size(); i++){
        Transform& transform = transforms->getComponentFromIndex(i);

        // Finding childs
        if (i > index){
            if (std::find(parentList.begin(), parentList.end(), transform.parent) == parentList.end()){
                break;
            }
        }

        entity = transforms->getEntity(i);
        parentList.push_back(entity);

        OBB entityOBB = getOBB(entity, false);

        if (entityOBB != OBB::ZERO){
            entityOBB.setHalfExtents(entityOBB.getHalfExtents() + Vector3(offset));

            obb.enclose(entityOBB);
        }
    }

    return obb;
}

void Editor::SceneRender::activate(){
    Engine::setFramebuffer(&framebuffer);
    Engine::setScene(scene);

    Engine::removeAllSceneLayers();

    Engine::addSceneLayer(toolslayer.getScene());
    Engine::addSceneLayer(uilayer.getScene());
}

void Editor::SceneRender::updateSize(int width, int height){

}

void Editor::SceneRender::updateRenderSystem(){
    // Meshes and UIs are created in update, without this can affect worldAABB
    scene->getSystem<MeshSystem>()->update(0);
    scene->getSystem<UISystem>()->update(0);
    // to avoid gizmos delays
    scene->getSystem<RenderSystem>()->update(0);
}

void Editor::SceneRender::update(std::vector<Entity> selEntities){
    Entity cameraEntity = camera->getEntity();
    CameraComponent& cameracomp = scene->getComponent<CameraComponent>(cameraEntity);
    Transform& cameratransform = scene->getComponent<Transform>(cameraEntity);

    // TODO: avoid get gizmo position and rotation every frame
    Vector3 gizmoPosition;
    Quaternion gizmoRotation;

    bool sameRotation = true;
    Quaternion firstRotation;
    bool firstEntity = true;

    size_t numTEntities = 0;
    std::vector<OBB> selBB;
    OBB totalSelBB;

    for (Entity& entity: selEntities){
        if (Transform* transform = scene->findComponent<Transform>(entity)){
            numTEntities++;

            gizmoPosition += transform->worldPosition;

            if (firstEntity) {
                firstRotation = transform->worldRotation;
                firstEntity = false;
            } else if (transform->worldRotation != firstRotation) {
                sameRotation = false;
            }

            if ((!useGlobalTransform && selEntities.size() == 1) || toolslayer.getGizmoSelected() == GizmoSelected::OBJECT2D){
                gizmoRotation = transform->worldRotation;
            }

            selBB.push_back(getFamilyOBB(entity, selectionOffset));

            totalSelBB.enclose(getOBB(entity, false));
        }
    }

    multipleEntitiesSelected = selEntities.size() > 1;

    bool selectionVisibility = false;
    if (numTEntities > 0){
        gizmoPosition /= numTEntities;

        selectionVisibility = true;

        float scale = gizmoScale * zoom;
        if (cameracomp.type == CameraType::CAMERA_PERSPECTIVE){
            float dist = (gizmoPosition - camera->getWorldPosition()).length();
            scale = std::tan(cameracomp.yfov) * dist * (gizmoScale / (float)framebuffer.getHeight());
        }

        toolslayer.updateGizmo(camera, gizmoPosition, gizmoRotation, scale, totalSelBB, mouseRay, mouseClicked);

        if (selBB.size() == 0){
            selLines->setVisible(false);
        }else{
            selLines->setVisible(true);

            updateSelLines(selBB);
        }
    }
    toolslayer.updateCamera(cameracomp, cameratransform);
    selLines->setVisible(selectionVisibility);

    if (toolslayer.getGizmoSelected() == GizmoSelected::OBJECT2D && !sameRotation){
        toolslayer.setGizmoVisible(false);
    }else{
        toolslayer.setGizmoVisible(selectionVisibility);
    }
}

void Editor::SceneRender::mouseHoverEvent(float x, float y){
    mouseRay = camera->screenToRay(x, y);
}

void Editor::SceneRender::mouseClickEvent(float x, float y, std::vector<Entity> selEntities){
    mouseClicked = true;

    Vector3 viewDir = camera->getWorldDirection();

    Vector3 gizmoPosition = toolslayer.getGizmoPosition();
    Quaternion gizmoRotation = toolslayer.getGizmoRotation();
    Matrix4 gizmoRMatrix = gizmoRotation.getRotationMatrix();
    Matrix4 gizmoMatrix = Matrix4::translateMatrix(gizmoPosition) * gizmoRMatrix * Matrix4::scaleMatrix(Vector3(1,1,1));

    float dotX = viewDir.dotProduct(gizmoRMatrix * Vector3(1,0,0));
    float dotY = viewDir.dotProduct(gizmoRMatrix * Vector3(0,1,0));
    float dotZ = viewDir.dotProduct(gizmoRMatrix * Vector3(0,0,1));

    if (toolslayer.getGizmoSelected() == GizmoSelected::TRANSLATE || toolslayer.getGizmoSelected() == GizmoSelected::SCALE){
        if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::XYZ){
            cursorPlane = Plane((gizmoRMatrix * Vector3(dotX, dotY, dotZ).normalize()), gizmoPosition);
        }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::X){
            cursorPlane = Plane((gizmoRMatrix * Vector3(0, dotY, dotZ).normalize()), gizmoPosition);
        }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::Y){
            cursorPlane = Plane((gizmoRMatrix * Vector3(dotX, 0, dotZ).normalize()), gizmoPosition);
        }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::Z){
            cursorPlane = Plane((gizmoRMatrix * Vector3(dotX, dotY, 0).normalize()), gizmoPosition);
        }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::XY){
            cursorPlane = Plane((gizmoRMatrix * Vector3(0, 0, dotZ).normalize()), gizmoPosition);
        }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::XZ){
            cursorPlane = Plane((gizmoRMatrix * Vector3(0, dotY, 0).normalize()), gizmoPosition);
        }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::YZ){
            cursorPlane = Plane((gizmoRMatrix * Vector3(dotX, 0, 0).normalize()), gizmoPosition);
        }
    }

    if (toolslayer.getGizmoSelected() == GizmoSelected::ROTATE){
        cursorPlane = Plane((gizmoRMatrix * Vector3(dotX, dotY, dotZ).normalize()), gizmoPosition);

        if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::X){
            rotationAxis = gizmoRMatrix * Vector3(dotX, 0.0, 0.0).normalize();
        }else if(toolslayer.getGizmoSideSelected() == GizmoSideSelected::Y){
            rotationAxis = gizmoRMatrix * Vector3(0.0, dotY, 0.0).normalize();
        }else if(toolslayer.getGizmoSideSelected() == GizmoSideSelected::Z){
            rotationAxis = gizmoRMatrix * Vector3(0.0, 0.0, dotZ).normalize();
        }else{
            rotationAxis = cursorPlane.normal;
        }
    }

    if (toolslayer.getGizmoSelected() == GizmoSelected::OBJECT2D){
        cursorPlane = Plane(Vector3(0, 0, 1), gizmoPosition);
    }

    RayReturn rretrun = mouseRay.intersects(cursorPlane);
    objectMatrixOffset.clear();
    if (rretrun){
        gizmoStartPosition = gizmoPosition;
        cursorStartOffset = gizmoPosition - rretrun.point;
        rotationStartOffset = gizmoRotation;
        scaleStartOffset = Vector3(1,1,1);

        for (Entity& entity: selEntities){
            Signature signature = scene->getSignature(entity);
            if (signature.test(scene->getComponentId<Transform>())){
                Transform& transform = scene->getComponent<Transform>(entity);
                objectMatrixOffset[entity] = gizmoMatrix.inverse() * transform.modelMatrix;
            }
            if (signature.test(scene->getComponentId<SpriteComponent>())){
                SpriteComponent& sprite = scene->getComponent<SpriteComponent>(entity);
                objectSizeOffset[entity] = Vector2(sprite.width, sprite.height);
            }
            if (signature.test(scene->getComponentId<UILayoutComponent>())){
                UILayoutComponent& layout = scene->getComponent<UILayoutComponent>(entity);
                objectSizeOffset[entity] = Vector2(layout.width, layout.height);
            }
        }
    }

}

void Editor::SceneRender::mouseReleaseEvent(float x, float y){
    uilayer.setRectVisible(false);

    mouseClicked = false;

    toolslayer.mouseRelease();

    if (lastCommand){
        lastCommand->setNoMerge();
        lastCommand = nullptr;
    }
}

void Editor::SceneRender::mouseDragEvent(float x, float y, float origX, float origY, size_t sceneId, SceneProject* sceneProject, std::vector<Entity> selEntities, bool disableSelection){
    if (!disableSelection){
        uilayer.setRectVisible(true);
        uilayer.updateRect(Vector2(origX, origY), Vector2(x, y) - Vector2(origX, origY));
    }

    Vector3 gizmoPosition = toolslayer.getGizmoPosition();
    Matrix4 gizmoRMatrix = toolslayer.getGizmoRotation().getRotationMatrix();

    for (Entity& entity: selEntities){
        Transform* transform = scene->findComponent<Transform>(entity);
        if (transform){
            RayReturn rretrun = mouseRay.intersects(cursorPlane);

            if (rretrun){

                toolslayer.mouseDrag(rretrun.point);

                Transform* transformParent = scene->findComponent<Transform>(transform->parent);

                if (toolslayer.getGizmoSelected() == GizmoSelected::TRANSLATE){
                    Vector3 newPos = gizmoRMatrix.inverse() * ((rretrun.point + cursorStartOffset) - gizmoPosition);
                    if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::XYZ){
                        newPos = gizmoPosition + (gizmoRMatrix * newPos);
                    }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::X){
                        newPos = gizmoPosition + (gizmoRMatrix * Vector3(newPos.x, 0, 0));
                    }else if(toolslayer.getGizmoSideSelected() == GizmoSideSelected::Y){
                        newPos = gizmoPosition + (gizmoRMatrix * Vector3(0, newPos.y, 0));
                    }else if(toolslayer.getGizmoSideSelected() == GizmoSideSelected::Z){
                        newPos = gizmoPosition + (gizmoRMatrix * Vector3(0, 0, newPos.z));
                    }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::XY){
                        newPos = gizmoPosition + (gizmoRMatrix * Vector3(newPos.x, newPos.y, 0));
                    }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::XZ){
                        newPos = gizmoPosition + (gizmoRMatrix * Vector3(newPos.x, 0, newPos.z));
                    }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::YZ){
                        newPos = gizmoPosition + (gizmoRMatrix * Vector3(0, newPos.y, newPos.z));
                    }

                    Matrix4 gizmoMatrix = Matrix4::translateMatrix(newPos) * gizmoRMatrix * Matrix4::scaleMatrix(Vector3(1,1,1));
                    Matrix4 objMatrix = gizmoMatrix * objectMatrixOffset[entity];

                    if (transformParent){
                        objMatrix = transformParent->modelMatrix.inverse() * objMatrix;
                    }

                    if (toolslayer.getGizmoSideSelected() != GizmoSideSelected::NONE){
                        lastCommand = new ObjectTransformCmd(sceneProject, entity, objMatrix);
                    }
                }

                if (toolslayer.getGizmoSelected() == GizmoSelected::ROTATE){
                    Vector3 lastPoint = gizmoPosition - rretrun.point;

                    float dot = cursorStartOffset.dotProduct(lastPoint);
                    float slength = cursorStartOffset.length() * lastPoint.length();
                    float cosine = (slength != 0) ? dot / slength : 0;
                    cosine = std::fmax(-1.0, std::fmin(1.0, cosine));
                    float orig_angle = acos(cosine);

                    Vector3 cross = cursorStartOffset.crossProduct(lastPoint);
                    float sign = cross.dotProduct(cursorPlane.normal);

                    float angle = (sign < 0) ? -orig_angle : orig_angle;

                    Quaternion newRot = Quaternion(Angle::radToDefault(angle), rotationAxis) * rotationStartOffset;

                    Matrix4 gizmoMatrix = Matrix4::translateMatrix(gizmoPosition) * newRot.getRotationMatrix() * Matrix4::scaleMatrix(Vector3(1,1,1));
                    Matrix4 objMatrix = gizmoMatrix * objectMatrixOffset[entity];

                    if (transformParent){
                        objMatrix = transformParent->modelMatrix.inverse() * objMatrix;
                    }

                    if (toolslayer.getGizmoSideSelected() != GizmoSideSelected::NONE){
                        lastCommand = new ObjectTransformCmd(sceneProject, entity, objMatrix);
                    }
                }

                if (toolslayer.getGizmoSelected() == GizmoSelected::SCALE){
                    Vector3 lastPoint = gizmoPosition - rretrun.point;

                    Vector3 startRotPoint = gizmoRMatrix.inverse() * cursorStartOffset;
                    Vector3 lastRotPoint = gizmoRMatrix.inverse() * lastPoint;

                    float radioX = (startRotPoint.x != 0) ? (lastRotPoint.x / startRotPoint.x) : 1;
                    float radioY = (startRotPoint.y != 0) ? (lastRotPoint.y / startRotPoint.y) : 1;
                    float radioZ = (startRotPoint.z != 0) ? (lastRotPoint.z / startRotPoint.z) : 1;

                    Vector3 newScale;
                    if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::XYZ){
                        newScale = Vector3((lastPoint.length() / cursorStartOffset.length()));
                    }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::X){
                        newScale = Vector3(radioX, 1, 1);
                    }else if(toolslayer.getGizmoSideSelected() == GizmoSideSelected::Y){
                        newScale = Vector3(1, radioY, 1);
                    }else if(toolslayer.getGizmoSideSelected() == GizmoSideSelected::Z){
                        newScale = Vector3(1, 1, radioZ);
                    }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::XY){
                        newScale = Vector3(radioX, radioY, 1);
                    }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::XZ){
                        newScale = Vector3(radioX, 1, radioZ);
                    }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::YZ){
                        newScale = Vector3(1, radioY, radioZ);
                    }

                    newScale = newScale * scaleStartOffset;
                    newScale = Vector3(abs(newScale.x), abs(newScale.y), abs(newScale.z));

                    Matrix4 gizmoMatrix = Matrix4::translateMatrix(gizmoPosition) * gizmoRMatrix * Matrix4::scaleMatrix(newScale);
                    Matrix4 objMatrix = gizmoMatrix * objectMatrixOffset[entity];

                    if (transformParent){
                        objMatrix = transformParent->modelMatrix.inverse() * objMatrix;
                    }

                    if (toolslayer.getGizmoSideSelected() != GizmoSideSelected::NONE){
                        lastCommand = new ObjectTransformCmd(sceneProject, entity, objMatrix);
                    }
                }

                if (toolslayer.getGizmoSelected() == GizmoSelected::OBJECT2D){
                    bool isSprite = scene->getComponentArray<SpriteComponent>()->hasEntity(entity);
                    bool isLayout = scene->getComponentArray<UILayoutComponent>()->hasEntity(entity);

                    Vector3 newPos = gizmoRMatrix.inverse() * ((rretrun.point + cursorStartOffset) - gizmoPosition);

                    Vector3 newSize = gizmoRMatrix.inverse() * -(gizmoStartPosition - rretrun.point - cursorStartOffset);

                    if (toolslayer.getGizmo2DSideSelected() == Gizmo2DSideSelected::CENTER){
                        newPos = gizmoPosition + (gizmoRMatrix * newPos);
                        newSize = Vector3(0, 0, 0);
                    }else if (toolslayer.getGizmo2DSideSelected() == Gizmo2DSideSelected::NX){
                        newPos = gizmoPosition + (gizmoRMatrix * Vector3(newPos.x, 0, 0));
                        newSize = Vector3(-newSize.x, 0, 0);
                    }else if (toolslayer.getGizmo2DSideSelected() == Gizmo2DSideSelected::NY){
                        newPos = gizmoPosition + (gizmoRMatrix * Vector3(0, newPos.y, 0));
                        newSize = Vector3(0, -newSize.y, 0);
                    }else if (toolslayer.getGizmo2DSideSelected() == Gizmo2DSideSelected::PX){
                        newPos = gizmoPosition + (gizmoRMatrix * Vector3(0, 0, 0));
                        newSize = Vector3(newSize.x, 0, 0);
                    }else if (toolslayer.getGizmo2DSideSelected() == Gizmo2DSideSelected::PY){
                        newPos = gizmoPosition + (gizmoRMatrix * Vector3(0, 0, 0));
                        newSize = Vector3(0, newSize.y, 0);
                    }else if (toolslayer.getGizmo2DSideSelected() == Gizmo2DSideSelected::NX_NY){
                        newPos = gizmoPosition + (gizmoRMatrix * Vector3(newPos.x, newPos.y, 0));
                        newSize = Vector3(-newSize.x, -newSize.y, 0);
                    }else if (toolslayer.getGizmo2DSideSelected() == Gizmo2DSideSelected::NX_PY){
                        newPos = gizmoPosition + (gizmoRMatrix * Vector3(newPos.x, 0, 0));
                        newSize = Vector3(-newSize.x, newSize.y, 0);
                    }else if (toolslayer.getGizmo2DSideSelected() == Gizmo2DSideSelected::PX_NY){
                        newPos = gizmoPosition + (gizmoRMatrix * Vector3(0, newPos.y, 0));
                        newSize = Vector3(newSize.x, -newSize.y, 0);
                    }else if (toolslayer.getGizmo2DSideSelected() == Gizmo2DSideSelected::PX_PY){
                        newPos = gizmoPosition + (gizmoRMatrix * Vector3(0, 0, 0));
                        newSize = Vector3(newSize.x, newSize.y, 0);
                    }

                    Matrix4 gizmoMatrix = Matrix4::translateMatrix(newPos) * gizmoRMatrix * Matrix4::scaleMatrix(Vector3(1,1,1));
                    Matrix4 objMatrix = gizmoMatrix * objectMatrixOffset[entity];

                    if (transformParent){
                        objMatrix = transformParent->modelMatrix.inverse() * objMatrix;
                    }

                    Vector3 oScale = Vector3(1.0f, 1.0f, 1.0f);
                    oScale.x = Vector3(objMatrix[0][0], objMatrix[0][1], objMatrix[0][2]).length();
                    oScale.y = Vector3(objMatrix[1][0], objMatrix[1][1], objMatrix[1][2]).length();
                    oScale.z = Vector3(objMatrix[2][0], objMatrix[2][1], objMatrix[2][2]).length();

                    Vector2 size = objectSizeOffset[entity] + Vector2(newSize.x / oScale.x, newSize.y / oScale.y);
                    Vector3 pos = Vector3(objMatrix[3][0], objMatrix[3][1], objMatrix[3][2]);

                    if (size.x < 0) size.x = 0;
                    if (size.y < 0) size.y = 0;

                    if (toolslayer.getGizmo2DSideSelected() != Gizmo2DSideSelected::NONE){
                        MultiPropertyCmd* multiCmd = new MultiPropertyCmd();
                        if (isLayout){
                            multiCmd->addPropertyCmd<unsigned int>(sceneProject, entity, ComponentType::UILayoutComponent, "width", UpdateFlags_Layout_Sizes, static_cast<unsigned int>(size.x));
                            multiCmd->addPropertyCmd<unsigned int>(sceneProject, entity, ComponentType::UILayoutComponent, "height", UpdateFlags_Layout_Sizes, static_cast<unsigned int>(size.y));
                        }else if (isSprite){
                            multiCmd->addPropertyCmd<unsigned int>(sceneProject, entity, ComponentType::SpriteComponent, "width", UpdateFlags_Sprite, static_cast<unsigned int>(size.x));
                            multiCmd->addPropertyCmd<unsigned int>(sceneProject, entity, ComponentType::SpriteComponent, "height", UpdateFlags_Sprite, static_cast<unsigned int>(size.y));
                        }
                        multiCmd->addPropertyCmd<Vector3>(sceneProject, entity, ComponentType::Transform, "position", UpdateFlags_Transform, pos);
                        lastCommand = multiCmd;
                    }
                }

                if (lastCommand){
                    CommandHandle::get(sceneId)->addCommand(lastCommand);
                }
            }
        }
    }
}

bool Editor::SceneRender::isAnyGizmoSideSelected() const{
    return (toolslayer.getGizmoSideSelected() != Editor::GizmoSideSelected::NONE || toolslayer.getGizmo2DSideSelected() != Gizmo2DSideSelected::NONE);
}

TextureRender& Editor::SceneRender::getTexture(){
    //return camera->getFramebuffer()->getRender().getColorTexture();
    return framebuffer.getRender().getColorTexture();
}

Camera* Editor::SceneRender::getCamera(){
    return camera;
}

Editor::ToolsLayer* Editor::SceneRender::getToolsLayer(){
    return &toolslayer;
}

Editor::UILayer* Editor::SceneRender::getUILayer(){
    return &uilayer;
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

Editor::CursorSelected Editor::SceneRender::getCursorSelected() const{
    return cursorSelected;
}

bool Editor::SceneRender::isMultipleEntitesSelected() const{
    return multipleEntitiesSelected;
}