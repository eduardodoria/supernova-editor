#include "SceneRender.h"

using namespace Supernova;

Editor::SceneRender::SceneRender(Scene* scene){
    this->scene = scene;
    camera = new Camera(scene);

    scene->setCamera(camera);

    cursorSelected = CursorSelected::POINTER;
}

Editor::SceneRender::~SceneRender(){
    framebuffer.destroy();

    delete camera;
}

AABB Editor::SceneRender::getFamilyAABB(Entity entity, float sizePercent){
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

        Signature signature = scene->getSignature(entity);
        if (signature.test(scene->getComponentId<MeshComponent>())){
            MeshComponent& mesh = scene->getComponent<MeshComponent>(entity);
            aabb.merge(transform.modelMatrix * Matrix4::scaleMatrix(Vector3(sizePercent)) * mesh.aabb);
        }else if (signature.test(scene->getComponentId<UIComponent>())){
            UIComponent& ui = scene->getComponent<UIComponent>(entity);
            aabb.merge(transform.modelMatrix * Matrix4::scaleMatrix(Vector3(sizePercent)) * ui.aabb);
            //aabb = Matrix4::translateMatrix(Vector3(-30, -30, 0)) * aabb;
        }
    }

    return aabb;
}

void Editor::SceneRender::activate(){
    Engine::setFramebuffer(&framebuffer);
    Engine::setScene(scene);

    Engine::removeAllSceneLayers();
}

void Editor::SceneRender::updateSize(int width, int height){

}

void Editor::SceneRender::updateRenderSystem(){
    // UIs is created in update, without this can affect worldA
    scene->getSystem<UISystem>()->update(0);
    // to avoid gizmos delays
    scene->getSystem<RenderSystem>()->update(0);
}

TextureRender& Editor::SceneRender::getTexture(){
    //return camera->getFramebuffer()->getRender().getColorTexture();
    return framebuffer.getRender().getColorTexture();
}

Camera* Editor::SceneRender::getCamera(){
    return camera;
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