#include "SceneRender.h"

#include "sky/Daylight_Box_Back_png.h"
#include "sky/Daylight_Box_Bottom_png.h"
#include "sky/Daylight_Box_Front_png.h"
#include "sky/Daylight_Box_Left_png.h"
#include "sky/Daylight_Box_Right_png.h"
#include "sky/Daylight_Box_Top_png.h"

#include "command/CommandHistory.h"
#include "command/type/ChangePropertyCmd.h"

using namespace Supernova;

float Editor::SceneRender::gizmoSize = 40;

Editor::SceneRender::SceneRender(Scene* scene){
    this->scene = scene;
    mouseClicked = false;
    lastCommand = nullptr;
    useGlobalTransform = true;

    Lines* lines = new Lines(scene);
    Light* sun = new Light(scene);
    SkyBox* sky = new SkyBox(scene);

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

void Editor::SceneRender::update(Entity selectedEntity){
    viewgizmo.applyRotation(camera);

    Entity cameraEntity = camera->getEntity();
    CameraComponent& cameracomp = scene->getComponent<CameraComponent>(cameraEntity);
    Transform& cameratransform = scene->getComponent<Transform>(cameraEntity);

    toolslayer.updateCamera(cameracomp, cameratransform);

    bool gizmoVisibility = false;
    if (selectedEntity != NULL_ENTITY){
        Transform* transform = scene->findComponent<Transform>(selectedEntity);

        if (transform){
            gizmoVisibility = true;

            float dist = (transform->worldPosition - camera->getWorldPosition()).length();
            float scale = std::tan(cameracomp.yfov) * dist * (gizmoSize / (float)framebuffer.getHeight());

            Quaternion gizmoRotation;
            if (!useGlobalTransform){
                gizmoRotation = transform->worldRotation;
            }

            toolslayer.updateGizmo(camera, transform->worldPosition, gizmoRotation, scale, mouseRay, mouseClicked);
        }
    }
    toolslayer.setGizmoVisible(gizmoVisibility);
}

void Editor::SceneRender::mouseHoverEvent(float x, float y){
    mouseRay = camera->screenToRay(x, y);
}

void Editor::SceneRender::mouseClickEvent(float x, float y, Entity selEntity){
    mouseClicked = true;
    mouseStartPosition = Vector2(x, y);

    Transform* transform = scene->findComponent<Transform>(selEntity);

    if (transform){
        Vector3 viewDir = camera->getWorldDirection();

        Quaternion gizmoRotation;
        if (!useGlobalTransform){
            gizmoRotation = transform->worldRotation;
        }
        Matrix4 gizmoRMatrix = gizmoRotation.getRotationMatrix();

        float dotX = viewDir.dotProduct(gizmoRMatrix * Vector3(1,0,0));
        float dotY = viewDir.dotProduct(gizmoRMatrix * Vector3(0,1,0));
        float dotZ = viewDir.dotProduct(gizmoRMatrix * Vector3(0,0,1));

        if (toolslayer.getGizmoSelected() == GizmoSelected::TRANSLATE || toolslayer.getGizmoSelected() == GizmoSelected::SCALE){
            if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::XYZ){
                cursorPlane = Plane((gizmoRMatrix * Vector3(dotX, dotY, dotZ).normalize()), transform->worldPosition);
            }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::X){
                cursorPlane = Plane((gizmoRMatrix * Vector3(0, dotY, dotZ).normalize()), transform->worldPosition);
            }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::Y){
                cursorPlane = Plane((gizmoRMatrix * Vector3(dotX, 0, dotZ).normalize()), transform->worldPosition);
            }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::Z){
                cursorPlane = Plane((gizmoRMatrix * Vector3(dotX, dotY, 0).normalize()), transform->worldPosition);
            }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::XY){
                cursorPlane = Plane((gizmoRMatrix * Vector3(0, 0, dotZ).normalize()), transform->worldPosition);
            }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::XZ){
                cursorPlane = Plane((gizmoRMatrix * Vector3(0, dotY, 0).normalize()), transform->worldPosition);
            }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::YZ){
                cursorPlane = Plane((gizmoRMatrix * Vector3(dotX, 0, 0).normalize()), transform->worldPosition);
            }
        }

        if (toolslayer.getGizmoSelected() == GizmoSelected::ROTATE){
            cursorPlane = Plane((gizmoRMatrix * Vector3(dotX, dotY, dotZ).normalize()), transform->worldPosition);

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
        if (rretrun){
            cursorStartOffset = transform->worldPosition - rretrun.point;
            rotationStartOffset = transform->worldRotation;
            scaleStartOffset = transform->worldScale;
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

void Editor::SceneRender::mouseDragEvent(float x, float y, Entity selEntity){
    if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::NONE){
        uilayer.setRectVisible(true);
        uilayer.updateRect(mouseStartPosition, Vector2(x, y) - mouseStartPosition);
    }

    Transform* transform = scene->findComponent<Transform>(selEntity);

    if (transform){
        RayReturn rretrun = mouseRay.intersects(cursorPlane);

        Quaternion gizmoRotation;
        if (!useGlobalTransform){
            gizmoRotation = transform->worldRotation;
        }
        Matrix4 gizmoRMatrix = gizmoRotation.getRotationMatrix();

        if (rretrun){

            toolslayer.mouseDrag(rretrun.point);

            Transform* transformParent = scene->findComponent<Transform>(transform->parent);

            if (toolslayer.getGizmoSelected() == GizmoSelected::TRANSLATE){
                Vector3 newPos = gizmoRMatrix.inverse() * ((rretrun.point + cursorStartOffset) - transform->worldPosition);
                if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::XYZ){
                    newPos = transform->worldPosition + (gizmoRMatrix * newPos);
                }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::X){
                    newPos = transform->worldPosition + (gizmoRMatrix * Vector3(newPos.x, 0, 0));
                }else if(toolslayer.getGizmoSideSelected() == GizmoSideSelected::Y){
                    newPos = transform->worldPosition + (gizmoRMatrix * Vector3(0, newPos.y, 0));
                }else if(toolslayer.getGizmoSideSelected() == GizmoSideSelected::Z){
                    newPos = transform->worldPosition + (gizmoRMatrix * Vector3(0, 0, newPos.z));
                }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::XY){
                    newPos = transform->worldPosition + (gizmoRMatrix * Vector3(newPos.x, newPos.y, 0));
                }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::XZ){
                    newPos = transform->worldPosition + (gizmoRMatrix * Vector3(newPos.x, 0, newPos.z));
                }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::YZ){
                    newPos = transform->worldPosition + (gizmoRMatrix * Vector3(0, newPos.y, newPos.z));
                }

                if (transformParent){
                    newPos = transformParent->modelMatrix.inverse() * newPos;
                }

                if (toolslayer.getGizmoSideSelected() != GizmoSideSelected::NONE){
                    lastCommand = new ChangePropertyCmd<Vector3>(scene, selEntity, ComponentType::Transform, "position", newPos);
                }
            }

            if (toolslayer.getGizmoSelected() == GizmoSelected::ROTATE){
                Vector3 lastPoint = transform->worldPosition - rretrun.point;

                float dot = cursorStartOffset.dotProduct(lastPoint);
                float slength = cursorStartOffset.length() * lastPoint.length();
                float cosine = (slength != 0) ? dot / slength : 0;
                cosine = std::fmax(-1.0, std::fmin(1.0, cosine));
                float orig_angle = acos(cosine);

                Vector3 cross = cursorStartOffset.crossProduct(lastPoint);
                float sign = cross.dotProduct(cursorPlane.normal);

                float angle = (sign < 0) ? -orig_angle : orig_angle;

                Quaternion newRot = Quaternion(Angle::radToDefault(angle), rotationAxis) * rotationStartOffset;
                if (transformParent){
                    newRot = transformParent->worldRotation.inverse() * newRot;
                }

                if (toolslayer.getGizmoSideSelected() != GizmoSideSelected::NONE){
                    lastCommand = new ChangePropertyCmd<Quaternion>(scene, selEntity, ComponentType::Transform, "rotation", newRot);
                }
            }

            if (toolslayer.getGizmoSelected() == GizmoSelected::SCALE){
                Vector3 lastPoint = transform->worldPosition - rretrun.point;

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

                if (useGlobalTransform){
                    Matrix4 mRot = transform->worldRotation.getRotationMatrix();
                    newScale = (mRot.inverse() * Matrix4::scaleMatrix(newScale) * mRot) * newScale;
                }
                newScale = newScale * scaleStartOffset;
                newScale = Vector3(abs(newScale.x), abs(newScale.y), abs(newScale.z));

                if (transformParent){
                    newScale = Vector3(newScale.x / transformParent->worldScale.x, newScale.y / transformParent->worldScale.y, newScale.z / transformParent->worldScale.z);
                }

                if (toolslayer.getGizmoSideSelected() != GizmoSideSelected::NONE){
                    lastCommand = new ChangePropertyCmd<Vector3>(scene, selEntity, ComponentType::Transform, "scale", newScale);
                }

            }

            if (lastCommand){
                CommandHistory::addCommand(lastCommand);
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