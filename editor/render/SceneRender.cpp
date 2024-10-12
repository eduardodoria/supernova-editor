#include "SceneRender.h"

#include "sky/Daylight_Box_Back_png.h"
#include "sky/Daylight_Box_Bottom_png.h"
#include "sky/Daylight_Box_Front_png.h"
#include "sky/Daylight_Box_Left_png.h"
#include "sky/Daylight_Box_Right_png.h"
#include "sky/Daylight_Box_Top_png.h"

using namespace Supernova;

float Editor::SceneRender::gizmoSize = 40;

Editor::SceneRender::SceneRender(Scene* scene){
    this->scene = scene;
    mouseClicked = false;

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
    camera->setView(0, 0, 0);

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

            toolslayer.updateGizmo(transform->worldPosition, scale, mouseRay, mouseClicked);
        }
    }
    toolslayer.getGizmo()->setVisible(gizmoVisibility);
}

void Editor::SceneRender::mouseHoverEvent(float x, float y){
    mouseRay = camera->screenToRay(x, y);
}

void Editor::SceneRender::mouseClickEvent(float x, float y, Entity entity){
    mouseClicked = true;

    Transform* transform = scene->findComponent<Transform>(entity);

    if (transform){
        Vector3 viewDir = camera->getWorldPosition() - camera->getWorldView();

        float dotX = viewDir.dotProduct(Vector3(1,0,0));
        float dotY = viewDir.dotProduct(Vector3(0,1,0));
        float dotZ = viewDir.dotProduct(Vector3(0,0,1));

        if (toolslayer.getGizmoSelected() == GizmoSelected::XYZ){
            cursorPlane = Plane(Vector3(dotX, dotY, dotZ).normalize(), transform->worldPosition);
        }else if (toolslayer.getGizmoSelected() == GizmoSelected::X){
            cursorPlane = Plane(Vector3(0, dotY, dotZ).normalize(), transform->worldPosition);
        }else if (toolslayer.getGizmoSelected() == GizmoSelected::Y){
            cursorPlane = Plane(Vector3(dotX, 0, dotZ).normalize(), transform->worldPosition);
        }else if (toolslayer.getGizmoSelected() == GizmoSelected::Z){
            cursorPlane = Plane(Vector3(dotX, dotY, 0).normalize(), transform->worldPosition);
        }

        RayReturn rretrun = mouseRay.intersects(cursorPlane);
        if (rretrun){
            objectOffset = transform->worldPosition - rretrun.point;
        }
    }
}

void Editor::SceneRender::mouseReleaseEvent(float x, float y){
    mouseClicked = false;
}

void Editor::SceneRender::mouseDragEvent(float x, float y, Entity entity){
    Transform* transform = scene->findComponent<Transform>(entity);

    if (transform){
        RayReturn rretrun = mouseRay.intersects(cursorPlane);

        if (rretrun){
            if (toolslayer.getGizmoSelected() == GizmoSelected::XYZ){
                transform->position = rretrun.point + objectOffset;
            }else if (toolslayer.getGizmoSelected() == GizmoSelected::X){
                transform->position.x = rretrun.point.x + objectOffset.x;
            }else if (toolslayer.getGizmoSelected() == GizmoSelected::Y){
                transform->position.y = rretrun.point.y + objectOffset.y;
            }else if (toolslayer.getGizmoSelected() == GizmoSelected::Z){
                transform->position.z = rretrun.point.z + objectOffset.z;
            }
            transform->needUpdate = true;
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

bool Editor::SceneRender::isGizmoSelected() const{
    return (toolslayer.getGizmoSelected() != Editor::GizmoSelected::NONE);
}