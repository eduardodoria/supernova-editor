#include "ScriptPropertyCmd.h"

using namespace Supernova;

Editor::ScriptPropertyCmd::ScriptPropertyCmd(Project* project, uint32_t sceneId, Entity entity, 
                                             const std::string& propertyName, size_t propertyIndex,
                                             const ScriptPropertyValue& newValue) {
    this->project = project;
    this->sceneId = sceneId;
    this->propertyName = propertyName;

    this->values[entity].propertyIndex = propertyIndex;
    this->values[entity].newValue = newValue;
    this->wasModified = project->getScene(sceneId)->isModified;
}

bool Editor::ScriptPropertyCmd::execute() {
    SceneProject* sceneProject = project->getScene(sceneId);
    if (!sceneProject) {
        return false;
    }

    for (auto& [entity, value] : values) {
        ScriptComponent* scriptComp = sceneProject->scene->findComponent<ScriptComponent>(entity);
        if (!scriptComp) {
            return false;
        }

        if (value.propertyIndex >= scriptComp->properties.size()) {
            return false;
        }

        ScriptProperty& prop = scriptComp->properties[value.propertyIndex];
        value.oldValue = prop.value;
        prop.value = value.newValue;
        prop.syncToMember();

        if (project->isEntityShared(sceneId, entity)) {
            project->sharedGroupPropertyChanged(sceneId, entity, ComponentType::ScriptComponent, {"properties"});
        }
    }

    sceneProject->isModified = true;

    return true;
}

void Editor::ScriptPropertyCmd::undo() {
    SceneProject* sceneProject = project->getScene(sceneId);
    if (!sceneProject) {
        return;
    }

    for (auto const& [entity, value] : values) {
        ScriptComponent* scriptComp = sceneProject->scene->findComponent<ScriptComponent>(entity);
        if (!scriptComp) {
            continue;
        }

        if (value.propertyIndex >= scriptComp->properties.size()) {
            continue;
        }

        ScriptProperty& prop = scriptComp->properties[value.propertyIndex];
        prop.value = value.oldValue;
        prop.syncToMember();

        if (project->isEntityShared(sceneId, entity)) {
            project->sharedGroupPropertyChanged(sceneId, entity, ComponentType::ScriptComponent, {"properties"});
        }
    }

    sceneProject->isModified = wasModified;
}

bool Editor::ScriptPropertyCmd::mergeWith(Editor::Command* otherCommand) {
    ScriptPropertyCmd* otherCmd = dynamic_cast<ScriptPropertyCmd*>(otherCommand);
    if (otherCmd != nullptr) {
        if (sceneId == otherCmd->sceneId && propertyName == otherCmd->propertyName) {
            for (auto const& [otherEntity, otherValue] : otherCmd->values) {
                if (values.find(otherEntity) != values.end()) {
                    values[otherEntity].oldValue = otherValue.oldValue;
                } else {
                    values[otherEntity] = otherValue;
                }
            }
            wasModified = wasModified && otherCmd->wasModified;
            return true;
        }
    }

    return false;
}