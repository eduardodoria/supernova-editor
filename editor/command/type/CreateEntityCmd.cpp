#include "CreateEntityCmd.h"

#include "editor/Out.h"
#include "util/ShapeParameters.h"
#include "Stream.h"
#include "command/type/DeleteEntityCmd.h"

using namespace Supernova;

Editor::CreateEntityCmd::CreateEntityCmd(Project* project, uint32_t sceneId, std::string entityName, bool addToShared){
    this->project = project;
    this->sceneId = sceneId;
    this->entityName = entityName;
    this->entity = NULL_ENTITY;
    this->parent = NULL_ENTITY;
    this->type = EntityCreationType::EMPTY;
    this->addToShared = addToShared;
    this->updateFlags = 0;
}

Editor::CreateEntityCmd::CreateEntityCmd(Project* project, uint32_t sceneId, std::string entityName, EntityCreationType type, bool addToShared){
    this->project = project;
    this->sceneId = sceneId;
    this->entityName = entityName;
    this->entity = NULL_ENTITY;
    this->parent = NULL_ENTITY;
    this->type = type;
    this->addToShared = addToShared;
    this->updateFlags = 0;
}

Editor::CreateEntityCmd::CreateEntityCmd(Project* project, uint32_t sceneId, std::string entityName, EntityCreationType type, Entity parent, bool addToShared){
    this->project = project;
    this->sceneId = sceneId;
    this->entityName = entityName;
    this->entity = NULL_ENTITY;
    this->parent = parent;
    this->type = type;
    this->addToShared = addToShared;
    this->updateFlags = 0;
}

bool Editor::CreateEntityCmd::execute(){
    SceneProject* sceneProject = project->getScene(sceneId);

    if (!sceneProject){
        return false;
    }

    Scene* scene = sceneProject->scene;

    if (entity == NULL_ENTITY){
        entity = scene->createEntity();
    }else{
        if (!scene->recreateEntity(entity)){
            entity = scene->createEntity();
        }
    }

    unsigned int nameCount = 2;
    std::string baseName = entityName;
    bool foundName = true;
    while (foundName){
        foundName = false;
        for (auto& entity : sceneProject->entities) {
            std::string usedName = scene->getEntityName(entity);
            if (usedName == entityName){
                entityName = baseName + " " + std::to_string(nameCount);
                nameCount++;
                foundName = true;
            }
        }
    }

    if (type == EntityCreationType::OBJECT){

        scene->addComponent<Transform>(entity, {});

    }else if (type == EntityCreationType::BOX){

        scene->addComponent<Transform>(entity, {});
        scene->addComponent<MeshComponent>(entity, {});

        ShapeParameters shapeDefs;
        MeshComponent& mesh = scene->getComponent<MeshComponent>(entity);
        scene->getSystem<MeshSystem>()->createBox(mesh, shapeDefs.boxWidth, shapeDefs.boxHeight, shapeDefs.boxDepth);

    }else if (type == EntityCreationType::PLANE){

        scene->addComponent<Transform>(entity, {});
        scene->addComponent<MeshComponent>(entity, {});

        ShapeParameters shapeDefs;
        MeshComponent& mesh = scene->getComponent<MeshComponent>(entity);
        scene->getSystem<MeshSystem>()->createPlane(mesh, shapeDefs.planeWidth, shapeDefs.planeDepth);

    }else if (type == EntityCreationType::SPHERE){

        scene->addComponent<Transform>(entity, {});
        scene->addComponent<MeshComponent>(entity, {});

        ShapeParameters shapeDefs;
        MeshComponent& mesh = scene->getComponent<MeshComponent>(entity);
        scene->getSystem<MeshSystem>()->createSphere(mesh, shapeDefs.sphereRadius, shapeDefs.sphereSlices, shapeDefs.sphereStacks);

    }else if (type == EntityCreationType::CYLINDER){

        scene->addComponent<Transform>(entity, {});
        scene->addComponent<MeshComponent>(entity, {});

        ShapeParameters shapeDefs;
        MeshComponent& mesh = scene->getComponent<MeshComponent>(entity);
        scene->getSystem<MeshSystem>()->createCylinder(mesh, shapeDefs.cylinderBaseRadius, shapeDefs.cylinderTopRadius, shapeDefs.cylinderHeight, shapeDefs.cylinderSlices, shapeDefs.cylinderStacks);

    }else if (type == EntityCreationType::CAPSULE){

        scene->addComponent<Transform>(entity, {});
        scene->addComponent<MeshComponent>(entity, {});

        ShapeParameters shapeDefs;
        MeshComponent& mesh = scene->getComponent<MeshComponent>(entity);
        scene->getSystem<MeshSystem>()->createCapsule(mesh, shapeDefs.capsuleBaseRadius, shapeDefs.capsuleTopRadius, shapeDefs.capsuleHeight, shapeDefs.capsuleSlices, shapeDefs.capsuleStacks);

    }else if (type == EntityCreationType::TORUS){

        scene->addComponent<Transform>(entity, {});
        scene->addComponent<MeshComponent>(entity, {});

        ShapeParameters shapeDefs;
        MeshComponent& mesh = scene->getComponent<MeshComponent>(entity);
        scene->getSystem<MeshSystem>()->createTorus(mesh, shapeDefs.torusRadius, shapeDefs.torusRingRadius, shapeDefs.torusSides, shapeDefs.torusRings);

    }else if (type == EntityCreationType::IMAGE){

        scene->addComponent<Transform>(entity, {});
        scene->addComponent<UILayoutComponent>(entity, {});
        scene->addComponent<UIComponent>(entity, {});
        scene->addComponent<ImageComponent>(entity, {});

        UILayoutComponent& layout = scene->getComponent<UILayoutComponent>(entity);
        layout.width = 100;
        layout.height = 100;

    }else if (type == EntityCreationType::SPRITE){

        scene->addComponent<Transform>(entity, {});
        scene->addComponent<MeshComponent>(entity, {});
        scene->addComponent<SpriteComponent>(entity, {});

        SpriteComponent& sprite = scene->getComponent<SpriteComponent>(entity);
        sprite.width = 100;
        sprite.height = 100;
    }else if (type == EntityCreationType::DIRECTIONAL_LIGHT){

        scene->addComponent<Transform>(entity, {});
        scene->addComponent<LightComponent>(entity, {});

        LightComponent& light = scene->getComponent<LightComponent>(entity);
        light.type = LightType::DIRECTIONAL;
        light.direction = Vector3(0.0f, -1.0f, 0.0f);
        light.intensity = 4.0f;

    }else if (type == EntityCreationType::POINT_LIGHT){

        scene->addComponent<Transform>(entity, {});
        scene->addComponent<LightComponent>(entity, {});

        LightComponent& light = scene->getComponent<LightComponent>(entity);
        light.type = LightType::POINT;
        light.range = 10.0f;
        light.intensity = 30.0f;

    }else if (type == EntityCreationType::SPOT_LIGHT){

        scene->addComponent<Transform>(entity, {});
        scene->addComponent<LightComponent>(entity, {});

        LightComponent& light = scene->getComponent<LightComponent>(entity);
        light.type = LightType::SPOT;
        light.direction = Vector3(0.0f, -1.0f, 0.0f);
        light.range = 10.0f;
        light.intensity = 30.0f;

    }

    scene->setEntityName(entity, entityName);

    if (parent != NULL_ENTITY){
        scene->addEntityChild(parent, entity, false);
    }

    // Apply all property setters
    for (const auto& [componentType, properties] : propertySetters) {
        for (const auto& [propertyName, propertySetter] : properties) {
            propertySetter(entity);
        }
    }

    Catalog::updateEntity(scene, entity, updateFlags);

    sceneProject->entities.push_back(entity);

    lastSelected = project->getSelectedEntities(sceneId);
    project->setSelectedEntity(sceneId, entity);

    sceneProject->isModified = true;

    if (addToShared){
        project->addEntityToSharedGroup(sceneId, entity, parent, false);
    }

    ImGui::SetWindowFocus(("###Scene" + std::to_string(sceneId)).c_str());

    Editor::Out::info("Created entity '%s' at scene '%s'", entityName.c_str(), sceneProject->name.c_str());

    return true;
}

void Editor::CreateEntityCmd::undo(){
    SceneProject* sceneProject = project->getScene(sceneId);

    if (sceneProject){
        if (addToShared){
            project->removeEntityFromSharedGroup(sceneId, entity, false);
        }

        DeleteEntityCmd::destroyEntity(sceneProject->scene, entity, sceneProject->entities, project, sceneId);

        sceneProject->isModified = true;

    }
}

bool Editor::CreateEntityCmd::mergeWith(Editor::Command* otherCommand){
    return false;
}

Entity Editor::CreateEntityCmd::getEntity(){
    return entity;
}