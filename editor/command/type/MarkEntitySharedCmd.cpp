#include "MarkEntitySharedCmd.h"
#include "editor/Out.h"
#include "Stream.h"

using namespace Supernova;

Editor::MarkEntitySharedCmd::MarkEntitySharedCmd(Project* project, uint32_t sceneId, Entity entity, const fs::path& filepath){
    this->project = project;
    this->sceneId = sceneId;
    this->entity = entity;
    this->filepath = filepath;
    this->wasModified = project->getScene(sceneId)->isModified;
    this->wasSuccessful = false;
    this->providedEntityNode = YAML::Node(); // Empty node
}

Editor::MarkEntitySharedCmd::MarkEntitySharedCmd(Project* project, uint32_t sceneId, Entity entity, const fs::path& filepath, const YAML::Node& entityNode){
    this->project = project;
    this->sceneId = sceneId;
    this->entity = entity;
    this->filepath = filepath;
    this->wasModified = project->getScene(sceneId)->isModified;
    this->wasSuccessful = false;
    this->providedEntityNode = entityNode;
}

bool Editor::MarkEntitySharedCmd::execute(){
    SceneProject* sceneProject = project->getScene(sceneId);

    if (!sceneProject){
        Editor::Out::error("Invalid scene ID: %u", sceneId);
        return false;
    }

    if (!filepath.is_relative()) {
        Editor::Out::error("Shared entity filepath must be relative: %s", filepath.string().c_str());
        return false;
    }

    // Check if a shared group already exists at this path
    if (project->getSharedGroup(filepath)) {
        Editor::Out::error("Shared entity group already exists at %s", filepath.string().c_str());
        return false;
    }

    // Store current selection for undo
    lastSelected = project->getSelectedEntities(sceneId);

    // Use provided entity node if available, otherwise encode from the scene
    if (providedEntityNode && !providedEntityNode.IsNull()) {
        savedEntityNode = providedEntityNode;
    } else {
        savedEntityNode = Stream::encodeEntity(entity, sceneProject->scene, nullptr, sceneProject);
    }

    // Mark the entity as shared (this will also save it to disk)
    wasSuccessful = project->markEntityShared(sceneId, entity, filepath, savedEntityNode);

    if (wasSuccessful) {
        Editor::Out::info("Marked entity '%s' as shared at '%s'", sceneProject->scene->getEntityName(entity).c_str(), filepath.string().c_str());

        return true;
    }

    return false;
}

void Editor::MarkEntitySharedCmd::undo(){
    if (!wasSuccessful) {
        return;
    }

    SceneProject* sceneProject = project->getScene(sceneId);
    if (!sceneProject) {
        return;
    }

    // Verify the entity still exists
    if (! sceneProject->scene->isEntityCreated(entity)) {
        Editor::Out::warning("Cannot undo - entity no longer exists");
        return;
    }

    // Delete the shared entity file from disk if it exists
    try {
        fs::path fullPath = project->getProjectPath() / filepath;
        if (fs::exists(fullPath)) {
            fs::remove(fullPath);
            Editor::Out::info("Removed shared entity file: %s", fullPath.string().c_str());
        }
    } catch (const std::exception& e) {
        Editor::Out::warning("Failed to remove shared entity file: %s", e.what());
    }

    // Remove the shared group entirely (this handles all entity cleanup)
    project->removeSharedGroup(filepath);

    // Restore previous selection
    if (!lastSelected.empty()) {
        project->replaceSelectedEntities(sceneId, lastSelected);
    }

    // Restore modified state
    sceneProject->isModified = wasModified;

    Editor::Out::info("Unmarked entity '%s' from shared group at '%s'", sceneProject->scene->getEntityName(entity).c_str(), filepath.string().c_str());
}

bool Editor::MarkEntitySharedCmd::mergeWith(Editor::Command* otherCommand){
    // Mark entity shared commands typically don't merge with each other
    return false;
}