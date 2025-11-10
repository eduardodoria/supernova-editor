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

    importedEntities = project->importSharedEntity(sceneProject, &sceneProject->entities, filepath, parent, needSaveScene, extendNode);

    if (importedEntities.empty()){
        return false;
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
    if (!sceneProject) {
        return;
    }

    // Get entity info to recover same ids
    extendNode = Stream::encodeEntity(importedEntities[0], sceneProject->scene, project, sceneProject);

    // Unimport the shared entity
    project->unimportSharedEntity(sceneId, filepath, importedEntities);

    // Restore previous selection
    if (!lastSelected.empty()) {
        project->replaceSelectedEntities(sceneId, lastSelected);
    }

    // Restore modified state
    sceneProject->isModified = wasModified;
}

bool Editor::ImportSharedEntityCmd::mergeWith(Editor::Command* otherCommand){

    return false;
}

std::vector<Entity> Editor::ImportSharedEntityCmd::getImportedEntities() const{
    return importedEntities;
}