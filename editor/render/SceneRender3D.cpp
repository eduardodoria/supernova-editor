#include "SceneRender3D.h"

#include "resources/sky/Daylight_Box_Back_png.h"
#include "resources/sky/Daylight_Box_Bottom_png.h"
#include "resources/sky/Daylight_Box_Front_png.h"
#include "resources/sky/Daylight_Box_Left_png.h"
#include "resources/sky/Daylight_Box_Right_png.h"
#include "resources/sky/Daylight_Box_Top_png.h"

#include "Project.h"
#include "command/CommandHandle.h"
#include "command/type/ObjectTransformCmd.h"

using namespace Supernova;

float Editor::SceneRender3D::gizmoSize = 40;

Editor::SceneRender3D::SceneRender3D(Scene* scene): SceneRender(scene){
    linesOffset = Vector2(0, 0);
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

    createLines();

    //camera->setType(CameraType::CAMERA_2D);
    camera->setPosition(10, 4, 10);
    camera->setTarget(0, 0, 0);

    //camera->setRenderToTexture(true);
    //camera->setUseFramebufferSizes(false);

    sun->setType(LightType::DIRECTIONAL);
    sun->setDirection(-0.2, -0.5, 0.3);
    sun->setIntensity(4.0);
    sun->setShadows(true);
    sun->setRange(100);

    scene->setAmbientLight(0.2);
    //scene->setSceneAmbientLightEnabled(false);
    scene->setBackgroundColor(Vector4(0.25, 0.45, 0.65, 1.0));

    uilayer.setViewportGizmoTexture(viewgizmo.getFramebuffer());

    Engine::setScalingMode(Scaling::NATIVE);
    Engine::setFixedTimeSceneUpdate(false);
}

Editor::SceneRender3D::~SceneRender3D(){
    delete lines;
    delete sun;
    delete sky;
    delete selAABBLines;
}

void Editor::SceneRender3D::createLines(){
    int gridHeight = 0;
    int gridSize = camera->getFarClip() * 2;

    int xGridStart = -gridSize + linesOffset.x;
    int xGridEnd = gridSize + linesOffset.x;

    int yGridStart = -gridSize + linesOffset.y;
    int yGridEnd = gridSize + linesOffset.y;

    lines->clearLines();

    for (int i = xGridStart; i <= xGridEnd; i++){
        if (i == 0){
            lines->addLine(Vector3(i, gridHeight, yGridStart), Vector3(i, gridHeight, yGridEnd), Vector4(0.5, 0.5, 1.0, 1.0));
        }else{
            if (i % 10 == 0){
                lines->addLine(Vector3(i, gridHeight, yGridStart), Vector3(i, gridHeight, yGridEnd), Vector4(0.5, 0.5, 0.5, 1.0));
            }else{
                lines->addLine(Vector3(i, gridHeight, yGridStart), Vector3(i, gridHeight, yGridEnd), Vector4(0.5, 0.5, 0.5, 0.5));
            }
        }
    }
    for (int i = yGridStart; i <= yGridEnd; i++){
        if (i == 0){
            lines->addLine(Vector3(xGridStart, gridHeight, i), Vector3(xGridEnd, gridHeight, i), Vector4(1.0, 0.5, 0.5, 1.0));
        }else{
            if (i % 10 == 0){
                lines->addLine(Vector3(xGridStart, gridHeight, i), Vector3(xGridEnd, gridHeight, i), Vector4(0.5, 0.5, 0.5, 1.0));
            }else{
                lines->addLine(Vector3(xGridStart, gridHeight, i), Vector3(xGridEnd, gridHeight, i), Vector4(0.5, 0.5, 0.5, 0.5));
            }
        }
    }
    lines->addLine(Vector3(0, -gridSize, 0), Vector3(0, gridSize, 0), Vector4(0.5, 1.0, 0.5, 1.0));
}

void Editor::SceneRender3D::activate(){
    SceneRender::activate();

    Engine::addSceneLayer(toolslayer.getScene());
    Engine::addSceneLayer(uilayer.getScene());
    Engine::addSceneLayer(viewgizmo.getScene());
}

void Editor::SceneRender3D::update(std::vector<Entity> selEntities){
    int linesStepChange = (int)(camera->getFarClip() / 2);
    int cameraLineStepX = (int)(camera->getWorldPosition().x / linesStepChange) * linesStepChange;
    int cameraLineStepZ = (int)(camera->getWorldPosition().z / linesStepChange) * linesStepChange;
    if (cameraLineStepX != linesOffset.x || cameraLineStepZ != linesOffset.y){
        linesOffset = Vector2(cameraLineStepX, cameraLineStepZ);

        createLines();
    }

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

            selAABB.merge(getFamilyAABB(entity, 1.01));
        }
    }

    bool gizmoVisibility = false;
    if (numTEntities > 0){
        gizmoPosition /= numTEntities;

        gizmoVisibility = true;

        float dist = (gizmoPosition - camera->getWorldPosition()).length();
        float scale = std::tan(cameracomp.yfov) * dist * (gizmoSize / (float)framebuffer.getHeight());

        toolslayer.updateGizmo(camera, gizmoPosition, gizmoRotation, scale, mouseRay, mouseClicked);

        if (selAABB.isNull() || selAABB.isInfinite()){
            selAABBLines->setVisible(false);
        }else{
            selAABBLines->setVisible(true);

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
    }
    toolslayer.setGizmoVisible(gizmoVisibility);
    selAABBLines->setVisible(gizmoVisibility);
}

void Editor::SceneRender3D::mouseHoverEvent(float x, float y){
    mouseRay = camera->screenToRay(x, y);
}

void Editor::SceneRender3D::mouseClickEvent(float x, float y, std::vector<Entity> selEntities){
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

void Editor::SceneRender3D::mouseReleaseEvent(float x, float y){
    uilayer.setRectVisible(false);

    mouseClicked = false;

    toolslayer.mouseRelease();

    if (lastCommand){
        lastCommand->setNoMerge();
        lastCommand = nullptr;
    }
}

void Editor::SceneRender3D::mouseDragEvent(float x, float y, float origX, float origY, size_t sceneId, SceneProject* sceneProject, std::vector<Entity> selEntities, bool disableSelection){
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

                if (lastCommand){
                    CommandHandle::get(sceneId)->addCommand(lastCommand);
                }
            }
        }
    }
}

bool Editor::SceneRender3D::isAnyGizmoSideSelected() const{
    return (toolslayer.getGizmoSideSelected() != Editor::GizmoSideSelected::NONE);
}

Editor::ViewportGizmo* Editor::SceneRender3D::getViewportGizmo(){
    return &viewgizmo;
}

Editor::ToolsLayer* Editor::SceneRender3D::getToolsLayer(){
    return &toolslayer;
}

Editor::UILayer* Editor::SceneRender3D::getUILayer(){
    return &uilayer;
}