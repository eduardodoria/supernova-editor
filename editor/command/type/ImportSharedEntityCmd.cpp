#include "ImportSharedEntityCmd.h"

#include "editor/Out.h"
#include "Stream.h"

using namespace Supernova;

Editor::ImportSharedEntityCmd::ImportSharedEntityCmd(Project* project, uint32_t sceneId, const fs::path& filepath, Entity parent, bool needSaveScene){
    this->project = project;
    this->sceneId = sceneId;
    this->filepath = filepath;
    this->parent = parent;
    this->needSaveScene = needSaveScene;
    this->extendNode = YAML::Node();
    this->wasModified = project->getScene(sceneId)->isModified;
}

Editor::ImportSharedEntityCmd::ImportSharedEntityCmd(Project* project, uint32_t sceneId, const fs::path& filepath, Entity parent, bool needSaveScene, YAML::Node extendNode){
    this->project = project;
    this->sceneId = sceneId;
    this->filepath = filepath;
    this->parent = parent;
    this->needSaveScene = needSaveScene;
    this->extendNode = extendNode;
    this->wasModified = project->getScene(sceneId)->isModified;
}

bool Editor::ImportSharedEntityCmd::execute(){
    SceneProject* sceneProject = project->getScene(sceneId);

    if (!sceneProject){
        return false;
    }

    lastSelected = project->getSelectedEntities(sceneId);

    importedEntities = project->importSharedEntity(sceneProject, filepath, parent, needSaveScene, extendNode);

    if (importedEntities.empty()){
        return false;
    }

    // Add imported entities to scene's entity list
    for (Entity entity : importedEntities) {
        sceneProject->entities.push_back(entity);
    }

    // Select the root imported entity
    if (!importedEntities.empty()) {
        project->setSelectedEntity(sceneId, importedEntities[0]);
    }

    ImGui::SetWindowFocus(("###Scene" + std::to_string(sceneId)).c_str());

    Editor::Out::info("Imported shared entity from '%s' to scene '%s'", filepath.string().c_str(), sceneProject->name.c_str());

    return true;
}

void Editor::ImportSharedEntityCmd::undo(){
    SceneProject* sceneProject = project->getScene(sceneId);

    if (sceneProject){
        Scene* scene = sceneProject->scene;

        // Remove entities from shared group
        SharedGroup* group = project->getSharedGroup(filepath);
        if (group) {
            group->members.erase(sceneId);
            
            // If this was the last scene using this shared group, clean it up
            if (group->members.empty()) {
                // Note: We don't actually remove the shared group here as it might be needed
                // for redo operations. The group will be cleaned up when appropriate.
            }
        }

        // Destroy all imported entities
        for (Entity entity : importedEntities) {
            scene->destroyEntity(entity);

            // Remove from scene's entity list
            auto it = std::find(sceneProject->entities.begin(), sceneProject->entities.end(), entity);
            if (it != sceneProject->entities.end()) {
                sceneProject->entities.erase(it);
            }

            // Clear selection if this entity was selected
            if (project->isSelectedEntity(sceneId, entity)){
                project->clearSelectedEntities(sceneId);
            }
        }

        // Restore previous selection
        if (lastSelected.size() > 0){
            project->replaceSelectedEntities(sceneId, lastSelected);
        }

        sceneProject->isModified = wasModified;
    }
}

bool Editor::ImportSharedEntityCmd::mergeWith(Editor::Command* otherCommand){
    // Import commands typically don't merge with each other
    return false;
}

std::vector<Entity> Editor::ImportSharedEntityCmd::getImportedEntities() const{
    return importedEntities;
}