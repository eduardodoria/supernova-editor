#include "SceneRender.h"

#include "command/CommandHandle.h"
#include "command/type/ObjectTransformCmd.h"
#include "command/type/PropertyCmd.h"

using namespace Supernova;

Editor::SceneRender::SceneRender(Scene* scene, bool use2DGizmos, bool enableViewGizmo, float gizmoScale, float selectionOffset): toolslayer(use2DGizmos), uilayer(enableViewGizmo){
    this->mouseClicked = false;
    this->lastCommand = nullptr;
    this->useGlobalTransform = true;

    this->gizmoScale = gizmoScale;
    this->selectionOffset = selectionOffset;

    this->scene = scene;
    this->camera = new Camera(scene);

    this->zoom = 1.0f;

    scene->setCamera(camera);

    cursorSelected = CursorSelected::POINTER;
}

Editor::SceneRender::~SceneRender(){
    framebuffer.destroy();

    delete camera;
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

        AABB entityAABB;
        Signature signature = scene->getSignature(entity);
        if (signature.test(scene->getComponentId<MeshComponent>())){
            MeshComponent& mesh = scene->getComponent<MeshComponent>(entity);
            entityAABB = mesh.worldAABB;
        }else if (signature.test(scene->getComponentId<UIComponent>())){
            UIComponent& ui = scene->getComponent<UIComponent>(entity);
            entityAABB = ui.worldAABB;
        }

        if (!entityAABB.isNull() && !entityAABB.isInfinite()){
            Vector3 min = entityAABB.getMinimum() - Vector3(offset);
            Vector3 max = entityAABB.getMaximum() + Vector3(offset);
            entityAABB.setExtents(min, max);

            aabb.merge(entityAABB);
        }
    }

    return aabb;
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
    // UIs is created in update, without this can affect worldA
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

    size_t numTEntities = 0;
    AABB selAABB;

    for (Entity& entity: selEntities){
        if (Transform* transform = scene->findComponent<Transform>(entity)){
            numTEntities++;

            gizmoPosition += transform->worldPosition;

            if (!useGlobalTransform && selEntities.size() == 1){
                gizmoRotation = transform->worldRotation;
            }

            selAABB.merge(getFamilyAABB(entity, selectionOffset));
        }
    }

    bool gizmoVisibility = false;
    if (numTEntities > 0){
        gizmoPosition /= numTEntities;

        gizmoVisibility = true;

        float scale = gizmoScale * zoom;
        if (cameracomp.type == CameraType::CAMERA_PERSPECTIVE){
            float dist = (gizmoPosition - camera->getWorldPosition()).length();
            scale = std::tan(cameracomp.yfov) * dist * (gizmoScale / (float)framebuffer.getHeight());
        }

        toolslayer.updateGizmo(camera, gizmoPosition, gizmoRotation, scale, selAABB, mouseRay, mouseClicked);

        if (selAABB.isNull() || selAABB.isInfinite()){
            selLines->setVisible(false);
        }else{
            selLines->setVisible(true);

            updateSelLines(selAABB);
        }
    }
    toolslayer.updateCamera(cameracomp, cameratransform);
    toolslayer.setGizmoVisible(gizmoVisibility);
    selLines->setVisible(gizmoVisibility);
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
        cursorStartOffset = gizmoPosition - rretrun.point;
        rotationStartOffset = gizmoRotation;
        scaleStartOffset = Vector3(1,1,1);

        for (Entity& entity: selEntities){
            Signature signature = scene->getSignature(entity);
            if (signature.test(scene->getComponentId<Transform>())){
                Transform& transform = scene->getComponent<Transform>(entity);
                objectMatrixOffset[entity] = gizmoMatrix.inverse() * transform.modelMatrix;
            }else if (signature.test(scene->getComponentId<SpriteComponent>())){
                SpriteComponent& sprite = scene->getComponent<SpriteComponent>(entity);
                objectSizeOffset[entity] = Vector2(sprite.width, sprite.height);
            }else if (signature.test(scene->getComponentId<UILayoutComponent>())){
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
                    Vector3 newPos = gizmoRMatrix.inverse() * ((rretrun.point + cursorStartOffset) - gizmoPosition);
                    Vector2 newSize;
                    if (toolslayer.getGizmo2DSideSelected() == Gizmo2DSideSelected::CENTER){
                        newPos = gizmoPosition + (gizmoRMatrix * newPos);
                    }else if (toolslayer.getGizmo2DSideSelected() == Gizmo2DSideSelected::NX){
                        newPos = gizmoPosition + (gizmoRMatrix * Vector3(newPos.x, 0, 0));
                    }else if (toolslayer.getGizmo2DSideSelected() == Gizmo2DSideSelected::NY){
                        newPos = gizmoPosition + (gizmoRMatrix * Vector3(0, 0, 0));
                    }else if (toolslayer.getGizmo2DSideSelected() == Gizmo2DSideSelected::PX){
                        newSize = objectSizeOffset[entity] + Vector2(newPos.x, 0);
                        printf("objectSizeOffset: %f, %f\n", objectSizeOffset[entity].x, objectSizeOffset[entity].y);
                        printf("newPos: %f, %f\n", newPos.x, newPos.y);
                        printf("newSize: %f, %f\n", newSize.x, newSize.y);
                        newPos = gizmoPosition + (gizmoRMatrix * Vector3(0, 0, 0));
                    }else if (toolslayer.getGizmo2DSideSelected() == Gizmo2DSideSelected::PY){
                        newPos = gizmoPosition + (gizmoRMatrix * Vector3(0, newPos.y, 0));
                    }else if (toolslayer.getGizmo2DSideSelected() == Gizmo2DSideSelected::NX_NY){
                        newPos = gizmoPosition + (gizmoRMatrix * Vector3(newPos.x, 0, 0));
                    }else if (toolslayer.getGizmo2DSideSelected() == Gizmo2DSideSelected::NX_PY){
                        newPos = gizmoPosition + (gizmoRMatrix * Vector3(newPos.x, newPos.y, 0));
                    }else if (toolslayer.getGizmo2DSideSelected() == Gizmo2DSideSelected::PX_NY){
                        newPos = gizmoPosition + (gizmoRMatrix * Vector3(0, 0, 0));
                    }else if (toolslayer.getGizmo2DSideSelected() == Gizmo2DSideSelected::PX_PY){
                        newPos = gizmoPosition + (gizmoRMatrix * Vector3(0, newPos.y, 0));
                    }

                    Matrix4 gizmoMatrix = Matrix4::translateMatrix(newPos) * gizmoRMatrix * Matrix4::scaleMatrix(Vector3(1,1,1));
                    Matrix4 objMatrix = gizmoMatrix * objectMatrixOffset[entity];

                    if (transformParent){
                        objMatrix = transformParent->modelMatrix.inverse() * objMatrix;
                    }

                    if (toolslayer.getGizmo2DSideSelected() != Gizmo2DSideSelected::NONE){
                        Vector3 pos = Vector3(objMatrix[3][0], objMatrix[3][1], objMatrix[3][2]);
                        lastCommand = new PropertyCmd<int>(sceneProject->scene, entity, ComponentType::UILayoutComponent, "width", UpdateFlags_Layout_Sizes, static_cast<int>(newSize.x));
                        lastCommand = new PropertyCmd<int>(sceneProject->scene, entity, ComponentType::UILayoutComponent, "height", UpdateFlags_Layout_Sizes, static_cast<int>(newSize.y));
                        lastCommand = new PropertyCmd<Vector3>(sceneProject->scene, entity, ComponentType::Transform, "position", UpdateFlags_Transform, pos);
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

Editor::CursorSelected Editor::SceneRender::getCursorSelected(){
    return cursorSelected;
}