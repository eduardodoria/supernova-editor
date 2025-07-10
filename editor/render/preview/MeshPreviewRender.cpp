#include "MeshPreviewRender.h"

#include "Configs.h"
#include "Stream.h"

using namespace Supernova;

Editor::MeshPreviewRender::MeshPreviewRender(){
    scene = new Scene();
    camera = new Camera(scene);
    light = new Light(scene);
    
    mesh = new Mesh(scene);

    scene->setBackgroundColor(0.0, 0.0, 0.0, 0.0);
    scene->setCamera(camera);

    light->setDirection(-0.4, -0.5, -0.5);
    light->setIntensity(6.0);
    light->setType(LightType::DIRECTIONAL);

    camera->setTarget(0, 0, 0);
    camera->setType(CameraType::CAMERA_PERSPECTIVE);
    camera->setFramebufferSize(THUMBNAIL_SIZE, THUMBNAIL_SIZE);
    camera->setRenderToTexture(true);
}

Editor::MeshPreviewRender::~MeshPreviewRender(){
    delete camera;
    delete light;
    delete mesh;
    delete scene;
}

void Editor::MeshPreviewRender::applyMesh(YAML::Node meshData, bool updateCamera, bool removeMaterial){
    mesh->getComponent<MeshComponent>() = Stream::decodeMeshComponent(meshData);

    if (removeMaterial){
        mesh->setMaterial(Material());
    }
    
    if (updateCamera){
        positionCameraForMesh();
    }
}

void Editor::MeshPreviewRender::setBackground(Vector4 color){
    scene->setBackgroundColor(color);
}

void Editor::MeshPreviewRender::positionCameraForMesh(){
    AABB aabb = mesh->getAABB();
    Vector3 center = aabb.getCenter();
    Vector3 size = aabb.getSize();
    
    // Calculate the maximum dimension to determine camera distance
    float maxDimension = std::max({size.x, size.y, size.z});
    
    // Calculate camera distance based on field of view
    // Assuming a 45-degree FOV, we need distance = (maxDimension / 2) / tan(22.5°)
    float distance = (maxDimension * 1.0f) / tan(22.5f * M_PI / 180.0f);
    
    // Ensure minimum distance
    distance = std::max(distance, maxDimension * 1.6f);
    
    // Position camera at an angle for better 3D visualization
    Vector3 cameraOffset = Vector3(0.5f, 0.3f, 0.6f);
    cameraOffset.normalize();
    cameraOffset = cameraOffset * distance;
    
    Vector3 cameraPosition = center + cameraOffset;
    
    camera->setPosition(cameraPosition.x, cameraPosition.y, cameraPosition.z);
    camera->setTarget(center.x, center.y, center.z);
}

Framebuffer* Editor::MeshPreviewRender::getFramebuffer(){
    return camera->getFramebuffer();
}

Texture Editor::MeshPreviewRender::getTexture(){
    return Texture(getFramebuffer());
}

Entity Editor::MeshPreviewRender::getMeshEntity(){
    return mesh->getEntity();
}

MeshComponent& Editor::MeshPreviewRender::getMeshComponent(){
    return mesh->getComponent<MeshComponent>();
}

Scene* Editor::MeshPreviewRender::getScene(){
    return scene;
}

Object* Editor::MeshPreviewRender::getObject(){
    return mesh;
}
