#include "UILayer.h"

using namespace Supernova;

Editor::UILayer::UILayer(){
    Vector3 rectColor = Vector3(0.3, 0.1, 0.2);

    cursorSelected = CursorSelected::POINTER;

    scene = new Scene();
    camera = new Camera(scene);

    rect = new Object(scene);
    upRect = new Polygon(scene);
    bottomRect = new Polygon(scene);
    leftRect = new Polygon(scene);
    rightRect = new Polygon(scene);
    centralRect = new Polygon(scene);

    viewGizmoImage = new Image(scene);

    viewGizmoImage->setAnchorPreset(AnchorPreset::TOP_RIGHT);
    rect->setVisible(false);

    upRect->setColor(Vector4(rectColor, 1.0));
    bottomRect->setColor(Vector4(rectColor, 1.0));
    leftRect->setColor(Vector4(rectColor, 1.0));
    rightRect->setColor(Vector4(rectColor, 1.0));
    centralRect->setColor(Vector4(rectColor, 0.2));

    rect->addChild(upRect);
    rect->addChild(bottomRect);
    rect->addChild(leftRect);
    rect->addChild(rightRect);
    rect->addChild(centralRect);
    
    camera->setType(CameraType::CAMERA_2D);

    viewGizmoImage->setSize(100, 100);

    scene->setCamera(camera);
}

Editor::UILayer::~UILayer(){
    delete camera;
    delete rect;
    delete upRect;
    delete bottomRect;
    delete leftRect;
    delete rightRect;
    delete centralRect;
    delete viewGizmoImage;

    delete scene;
}

void Editor::UILayer::setViewportGizmoTexture(Framebuffer* framebuffer){
    viewGizmoImage->setTexture(framebuffer);
}

void Editor::UILayer::enableCursorPointer(){
    cursorSelected = CursorSelected::POINTER;
}

void Editor::UILayer::enableCursorHand(){
    cursorSelected = CursorSelected::HAND;
}

void Editor::UILayer::setRectVisible(bool visible){
    rect->setVisible(visible);
}

void Editor::UILayer::updateRect(Vector2 position, Vector2 size){
    float thickness = 2;

    if (size.x < 0 && size.y < 0){
        position = position + size - Vector2(thickness);
        size = -size + Vector2(thickness*2);
    }

    rect->setPosition(Vector3(position, 0));

    upRect->clearVertices();
    leftRect->clearVertices();
    rightRect->clearVertices();
    bottomRect->clearVertices();
    centralRect->clearVertices();

    upRect->addVertex(0, 0);
    upRect->addVertex(0, thickness);
    upRect->addVertex(size.x, 0);
    upRect->addVertex(size.x, thickness);

    bottomRect->addVertex(0, size.y-thickness);
    bottomRect->addVertex(0, size.y);
    bottomRect->addVertex(size.x, size.y-thickness);
    bottomRect->addVertex(size.x, size.y);

    leftRect->addVertex(0, 0);
    leftRect->addVertex(0, size.y);
    leftRect->addVertex(thickness, 0);
    leftRect->addVertex(thickness, size.y);

    rightRect->addVertex(size.x-thickness, 0);
    rightRect->addVertex(size.x-thickness, size.y);
    rightRect->addVertex(size.x, 0);
    rightRect->addVertex(size.x, size.y);

    centralRect->addVertex(0, 0);
    centralRect->addVertex(0, size.y);
    centralRect->addVertex(size.x, 0);
    centralRect->addVertex(size.x, size.y);
}

Framebuffer* Editor::UILayer::getFramebuffer(){
    return camera->getFramebuffer();
}

TextureRender& Editor::UILayer::getTexture(){
    return getFramebuffer()->getRender().getColorTexture();
}

Camera* Editor::UILayer::getCamera(){
    return camera;
}

Scene* Editor::UILayer::getScene(){
    return scene;
}

Editor::CursorSelected Editor::UILayer::getCursorSelected(){
    return cursorSelected;
}