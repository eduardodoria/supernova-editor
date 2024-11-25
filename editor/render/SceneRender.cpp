#include "SceneRender.h"

#include "sky/Daylight_Box_Back_png.h"
#include "sky/Daylight_Box_Bottom_png.h"
#include "sky/Daylight_Box_Front_png.h"
#include "sky/Daylight_Box_Left_png.h"
#include "sky/Daylight_Box_Right_png.h"
#include "sky/Daylight_Box_Top_png.h"

#include "command/CommandHistory.h"
#include "command/type/ChangePropertyCmd.h"
#include "command/type/ChangeObjTransfCmd.h"

using namespace Supernova;

float Editor::SceneRender::gizmoSize = 40;

Editor::SceneRender::SceneRender(Scene* scene){
    this->scene = scene;
    mouseClicked = false;
    lastCommand = nullptr;
    useGlobalTransform = true;

    lines = new Lines(scene);
    sun = new Light(scene);
    sky = new SkyBox(scene);

    selAABBLines = new Lines(scene);
    for (int i = 0; i < 12; i++){
        selAABBLines->addLine(Vector3::ZERO, Vector3::ZERO, Vector4(1.0, 0.6, 0.0, 1.0));
    }
    selAABBLines->setVisible(false);

    camera = new Camera(scene);

    TextureData skyBack;
    TextureData skyBottom;
    TextureData skyFront;
    TextureData skyLeft;
    TextureData skyRight;
    TextureData skyTop;

    skyBack.loadTextureFromMemory(Daylight_Box_Back_png, Daylight_Box_Back_png_len);
    skyBottom.loadTextureFromMemory(Daylight_Box_Bottom_png, Daylight_Box_Bottom_png_len);
    skyFront.loadTextureFromMemory(Daylight_Box_Front_png, Daylight_Box_Front_png_len);
    skyLeft.loadTextureFromMemory(Daylight_Box_Left_png, Daylight_Box_Left_png_len);
    skyRight.loadTextureFromMemory(Daylight_Box_Right_png, Daylight_Box_Right_png_len);
    skyTop.loadTextureFromMemory(Daylight_Box_Top_png, Daylight_Box_Top_png_len);

    sky->setTextures("default_editor_sky", skyBack, skyFront, skyLeft, skyRight, skyTop, skyBottom);

    int gridHeight = 0;

    for (int i = -1000; i <= 1000; i++){
        if (i == 0){
            lines->addLine(Vector3(i, gridHeight, -1000), Vector3(i, gridHeight, 1000), Vector4(0.5, 0.5, 1.0, 1.0));
        }else{
            if (i % 10 == 0){
                lines->addLine(Vector3(i, gridHeight, -1000), Vector3(i, gridHeight, 1000), Vector4(0.5, 0.5, 0.5, 1.0));
            }else{
                lines->addLine(Vector3(i, gridHeight, -1000), Vector3(i, gridHeight, 1000), Vector4(0.5, 0.5, 0.5, 0.5));
            }
        }
    }
    for (int i = -1000; i <= 1000; i++){
        if (i == 0){
            lines->addLine(Vector3(-1000, gridHeight, i), Vector3(1000, gridHeight, i), Vector4(1.0, 0.5, 0.5, 1.0));
        }else{
            if (i % 10 == 0){
                lines->addLine(Vector3(-1000, gridHeight, i), Vector3(1000, gridHeight, i), Vector4(0.5, 0.5, 0.5, 1.0));
            }else{
                lines->addLine(Vector3(-1000, gridHeight, i), Vector3(1000, gridHeight, i), Vector4(0.5, 0.5, 0.5, 0.5));
            }
        }
    }
    lines->addLine(Vector3(0, -1000, 0), Vector3(0, 1000, 0), Vector4(0.5, 1.0, 0.5, 1.0));

    //camera->setType(CameraType::CAMERA_2D);
    camera->setPosition(10, 4, 10);
    camera->setTarget(0, 0, 0);

    //camera->setRenderToTexture(true);
    //camera->setUseFramebufferSizes(false);

    sun->setType(LightType::DIRECTIONAL);
    sun->setDirection(-0.2, -0.5, 0.3);
    sun->setIntensity(4.0);
    sun->setShadows(true);

    scene->setAmbientLight(0.4);
    scene->setCamera(camera);
    scene->setBackgroundColor(Vector4(0.25, 0.45, 0.65, 1.0));

    uilayer.setViewportGizmoTexture(viewgizmo.getFramebuffer());

    Engine::setScalingMode(Scaling::NATIVE);
    Engine::setFixedTimeSceneUpdate(false);
}

AABB Editor::SceneRender::getFamilyAABB(Entity entity){
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

        if (MeshComponent* mesh = scene->findComponent<MeshComponent>(entity)){
            aabb.merge(transform.modelMatrix * Matrix4::scaleMatrix(Vector3(1.01)) * mesh->aabb);
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
    Engine::addSceneLayer(viewgizmo.getScene());
}

void Editor::SceneRender::updateRenderSystem(){
    scene->getSystem<RenderSystem>()->update(0);
}

void Editor::SceneRender::updateSize(int width, int height){
    //if (width > 0 && height > 0){
        //camera->setFramebufferSize(width, height);
    //}
}

void Editor::SceneRender::update(std::vector<Entity> selEntities){
    viewgizmo.applyRotation(camera);

    Entity cameraEntity = camera->getEntity();
    CameraComponent& cameracomp = scene->getComponent<CameraComponent>(cameraEntity);
    Transform& cameratransform = scene->getComponent<Transform>(cameraEntity);

    toolslayer.updateCamera(cameracomp, cameratransform);

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

            selAABB.merge(getFamilyAABB(entity));
        }
    }

    bool gizmoVisibility = false;
    if (numTEntities > 0){
        gizmoPosition /= numTEntities;

        gizmoVisibility = true;

        float dist = (gizmoPosition - camera->getWorldPosition()).length();
        float scale = std::tan(cameracomp.yfov) * dist * (gizmoSize / (float)framebuffer.getHeight());

        toolslayer.updateGizmo(camera, gizmoPosition, gizmoRotation, scale, mouseRay, mouseClicked);

        selAABBLines->updateLine(0, selAABB.getCorner(AABB::FAR_LEFT_BOTTOM), selAABB.getCorner(AABB::FAR_LEFT_TOP));
        selAABBLines->updateLine(1, selAABB.getCorner(AABB::FAR_LEFT_TOP), selAABB.getCorner(AABB::FAR_RIGHT_TOP));
        selAABBLines->updateLine(2, selAABB.getCorner(AABB::FAR_RIGHT_TOP), selAABB.getCorner(AABB::FAR_RIGHT_BOTTOM));
        selAABBLines->updateLine(3, selAABB.getCorner(AABB::FAR_RIGHT_BOTTOM), selAABB.getCorner(AABB::FAR_LEFT_BOTTOM));

        selAABBLines->updateLine(4, selAABB.getCorner(AABB::NEAR_LEFT_BOTTOM), selAABB.getCorner(AABB::NEAR_LEFT_TOP));
        selAABBLines->updateLine(5, selAABB.getCorner(AABB::NEAR_LEFT_TOP), selAABB.getCorner(AABB::NEAR_RIGHT_TOP));
        selAABBLines->updateLine(6, selAABB.getCorner(AABB::NEAR_RIGHT_TOP), selAABB.getCorner(AABB::NEAR_RIGHT_BOTTOM));
        selAABBLines->updateLine(7, selAABB.getCorner(AABB::NEAR_RIGHT_BOTTOM), selAABB.getCorner(AABB::NEAR_LEFT_BOTTOM));

        selAABBLines->updateLine(8, selAABB.getCorner(AABB::FAR_LEFT_BOTTOM), selAABB.getCorner(AABB::NEAR_LEFT_BOTTOM));
        selAABBLines->updateLine(9, selAABB.getCorner(AABB::FAR_LEFT_TOP), selAABB.getCorner(AABB::NEAR_LEFT_TOP));
        selAABBLines->updateLine(10, selAABB.getCorner(AABB::FAR_RIGHT_TOP), selAABB.getCorner(AABB::NEAR_RIGHT_TOP));
        selAABBLines->updateLine(11, selAABB.getCorner(AABB::FAR_RIGHT_BOTTOM), selAABB.getCorner(AABB::NEAR_RIGHT_BOTTOM));
    }
    toolslayer.setGizmoVisible(gizmoVisibility);
    selAABBLines->setVisible(gizmoVisibility);
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

    RayReturn rretrun = mouseRay.intersects(cursorPlane);
    objectMatrixOffset.clear();
    if (rretrun){
        cursorStartOffset = gizmoPosition - rretrun.point;
        rotationStartOffset = gizmoRotation;
        scaleStartOffset = Vector3(1,1,1);

        for (Entity& entity: selEntities){
            Transform* transform = scene->findComponent<Transform>(entity);
            if (transform){
                objectMatrixOffset[entity] = gizmoMatrix.inverse() * transform->modelMatrix;
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

void Editor::SceneRender::mouseDragEvent(float x, float y, float origX, float origY, std::vector<Entity> selEntities){
    if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::NONE){
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
                        lastCommand = new ChangeObjTransfCmd(scene, entity, objMatrix);
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
                        lastCommand = new ChangeObjTransfCmd(scene, entity, objMatrix);
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
                        lastCommand = new ChangeObjTransfCmd(scene, entity, objMatrix);
                    }

                }

                if (lastCommand){
                    CommandHistory::addCommand(lastCommand);
                }
            }
        }
    }
}

TextureRender& Editor::SceneRender::getTexture(){
    //return camera->getFramebuffer()->getRender().getColorTexture();
    return framebuffer.getRender().getColorTexture();
}

Camera* Editor::SceneRender::getCamera(){
    return camera;
}

Editor::ViewportGizmo* Editor::SceneRender::getViewportGizmo(){
    return &viewgizmo;
}

Editor::ToolsLayer* Editor::SceneRender::getToolsLayer(){
    return &toolslayer;
}

Editor::UILayer* Editor::SceneRender::getUILayer(){
    return &uilayer;
}

bool Editor::SceneRender::isGizmoSideSelected() const{
    return (toolslayer.getGizmoSideSelected() != Editor::GizmoSideSelected::NONE);
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