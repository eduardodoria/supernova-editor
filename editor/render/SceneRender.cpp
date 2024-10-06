#include "SceneRender.h"

#include "sky/Daylight_Box_Back_png.h"
#include "sky/Daylight_Box_Bottom_png.h"
#include "sky/Daylight_Box_Front_png.h"
#include "sky/Daylight_Box_Left_png.h"
#include "sky/Daylight_Box_Right_png.h"
#include "sky/Daylight_Box_Top_png.h"

using namespace Supernova;

Editor::SceneRender::SceneRender(Scene* scene){
    this->scene = scene;

    Lines* lines = new Lines(scene);
    Light* sun = new Light(scene);
    SkyBox* sky = new SkyBox(scene);
    cube  = new Shape(scene);

    cube->createBox(1,1,1);

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

    camera->setRenderToTexture(true);
    camera->setUseFramebufferSizes(false);

    sun->setType(LightType::DIRECTIONAL);
    sun->setDirection(-0.2, -0.5, 0.3);
    sun->setIntensity(4.0);
    sun->setShadows(true);

    scene->setAmbientLight(0.4);
    scene->setCamera(camera);
    scene->setBackgroundColor(Vector4(0.25, 0.45, 0.65, 1.0));

    Engine::setFixedTimeSceneUpdate(false);
}

void Editor::SceneRender::activate(){
    Engine::setScene(scene);
    Engine::removeAllSceneLayers();
    Engine::addSceneLayer(gizmos.getScene());
    Engine::addSceneLayer(gimbal.getScene());
}

void Editor::SceneRender::updateSize(int width, int height){
    if (width > 0 && height > 0){
        camera->setFramebufferSize(width, height);
        gizmos.getCamera()->setFramebufferSize(width, height);

        //gizmos.getCamera()->setOrtho(0, width, 0, height, -100, 100);
    }
}

void Editor::SceneRender::update(){
    gimbal.applyRotation(camera);

    float width = gizmos.getCamera()->getFramebuffer()->getWidth();
    float height = gizmos.getCamera()->getFramebuffer()->getHeight();

    //float width = gizmos.getCamera()->getRight();
    //float height = gizmos.getCamera()->getTop();

    Vector3 gpos = cube->getModelViewProjectionMatrix() * cube->getPosition();
    gpos = Vector3((gpos.x + 1.0) / 2.0 * width, (gpos.y + 1.0) / 2.0 * height, 0);
    gizmos.getGizmo()->setPosition(gpos);
}

TextureRender& Editor::SceneRender::getTexture(){
    //return Engine::framebuffer.getRender().getColorTexture();
    return camera->getFramebuffer()->getRender().getColorTexture();
}

Camera* Editor::SceneRender::getCamera(){
    return camera;
}

Editor::Gimbal* Editor::SceneRender::getGimbal(){
    return &gimbal;
}

Editor::Gizmos* Editor::SceneRender::getGizmos(){
    return &gizmos;
}