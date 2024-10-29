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

void Editor::SceneRender::updateSize(int width, int height){
    if (width > 0 && height > 0){
        //camera->setFramebufferSize(width, height);

        uilayer.updateSize(width, height);
    }
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

            toolslayer.updateGizmo(camera, transform->worldPosition, scale, mouseRay, mouseClicked);
        }
    }
    toolslayer.setGizmoVisible(gizmoVisibility);
}

void Editor::SceneRender::mouseHoverEvent(float x, float y){
    mouseRay = camera->screenToRay(x, y);
}

void Editor::SceneRender::mouseClickEvent(float x, float y, Entity entity){
    mouseClicked = true;

    Transform* transform = scene->findComponent<Transform>(entity);

    if (transform){
        Vector3 viewDir = camera->getWorldDirection();

        float dotX = viewDir.dotProduct(Vector3(1,0,0));
        float dotY = viewDir.dotProduct(Vector3(0,1,0));
        float dotZ = viewDir.dotProduct(Vector3(0,0,1));

        if (toolslayer.getGizmoSelected() == GizmoSelected::TRANSLATE){
            if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::XYZ){
                cursorPlane = Plane(Vector3(dotX, dotY, dotZ).normalize(), transform->worldPosition);
            }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::X){
                cursorPlane = Plane(Vector3(0, dotY, dotZ).normalize(), transform->worldPosition);
            }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::Y){
                cursorPlane = Plane(Vector3(dotX, 0, dotZ).normalize(), transform->worldPosition);
            }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::Z){
                cursorPlane = Plane(Vector3(dotX, dotY, 0).normalize(), transform->worldPosition);
            }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::XY){
                cursorPlane = Plane(Vector3(0, 0, dotZ).normalize(), transform->worldPosition);
            }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::XZ){
                cursorPlane = Plane(Vector3(0, dotY, 0).normalize(), transform->worldPosition);
            }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::YZ){
                cursorPlane = Plane(Vector3(dotX, 0, 0).normalize(), transform->worldPosition);
            }
        }

        if (toolslayer.getGizmoSelected() == GizmoSelected::ROTATE){
            cursorPlane = Plane(Vector3(dotX, dotY, dotZ).normalize(), transform->worldPosition);
        }

        RayReturn rretrun = mouseRay.intersects(cursorPlane);
        if (rretrun){
            cursorStartOffset = transform->worldPosition - rretrun.point;
            rotationStartOffset = transform->worldRotation;
        }
    }
}

void Editor::SceneRender::mouseReleaseEvent(float x, float y){
    mouseClicked = false;

    toolslayer.mouseRelease();

    if (lastCommand){
        lastCommand->setNoMerge();
        lastCommand = nullptr;
    }
}

void Editor::SceneRender::mouseDragEvent(float x, float y, Entity entity){
    Transform* transform = scene->findComponent<Transform>(entity);

    if (transform){
        RayReturn rretrun = mouseRay.intersects(cursorPlane);

        if (rretrun){

            toolslayer.mouseDrag(rretrun.point);

            Transform* transformParent = scene->findComponent<Transform>(transform->parent);

            if (toolslayer.getGizmoSelected() == GizmoSelected::TRANSLATE){
                Vector3 pos = (rretrun.point + cursorStartOffset);
                if (transformParent){
                    pos = transformParent->modelMatrix.inverse() * pos;
                }

                if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::XYZ){
                    lastCommand = new ChangePropertyCmd<Vector3>(scene, entity, ComponentType::Transform, "position", pos);
                }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::X){
                    Vector3 newPos = Vector3(pos.x, transform->position.y, transform->position.z);
                    lastCommand = new ChangePropertyCmd<Vector3>(scene, entity, ComponentType::Transform, "position", newPos);
                }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::Y){
                    Vector3 newPos = Vector3(transform->position.x, pos.y, transform->position.z);
                    lastCommand = new ChangePropertyCmd<Vector3>(scene, entity, ComponentType::Transform, "position", newPos);
                }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::Z){
                    Vector3 newPos = Vector3(transform->position.x, transform->position.y, pos.z);
                    lastCommand = new ChangePropertyCmd<Vector3>(scene, entity, ComponentType::Transform, "position", newPos);
                }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::XY){
                    Vector3 newPos = Vector3(pos.x, pos.y, transform->position.z);
                    lastCommand = new ChangePropertyCmd<Vector3>(scene, entity, ComponentType::Transform, "position", newPos);
                }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::XZ){
                    Vector3 newPos = Vector3(pos.x, transform->position.y, pos.z);
                    lastCommand = new ChangePropertyCmd<Vector3>(scene, entity, ComponentType::Transform, "position", newPos);
                }else if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::YZ){
                    Vector3 newPos = Vector3(transform->position.x, pos.y, pos.z);
                    lastCommand = new ChangePropertyCmd<Vector3>(scene, entity, ComponentType::Transform, "position", newPos);
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

                Vector3 rotAxis = cursorPlane.normal;
                if (toolslayer.getGizmoSideSelected() == GizmoSideSelected::X){
                    rotAxis = Vector3(cursorPlane.normal.x, 0.0, 0.0).normalize();
                }else if(toolslayer.getGizmoSideSelected() == GizmoSideSelected::Y){
                    rotAxis = Vector3(0.0, cursorPlane.normal.y, 0.0).normalize();
                }else if(toolslayer.getGizmoSideSelected() == GizmoSideSelected::Z){
                    rotAxis = Vector3(0.0, 0.0, cursorPlane.normal.z).normalize();
                }

                Quaternion newRot = Quaternion(Angle::radToDefault(angle), rotAxis) * rotationStartOffset;
                if (transformParent){
                    newRot = Quaternion(transformParent->modelMatrix.inverse() * newRot.getRotationMatrix());
                }

                lastCommand = new ChangePropertyCmd<Quaternion>(scene, entity, ComponentType::Transform, "rotation", newRot);
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