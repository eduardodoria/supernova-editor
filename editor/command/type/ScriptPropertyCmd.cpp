#include "ScriptPropertyCmd.h"

using namespace Supernova;

Editor::ScriptPropertyCmd::ScriptPropertyCmd(Project* project, uint32_t sceneId, Entity entity, 
                                             const std::string& scriptClassName, const std::string& propertyName,
                                             size_t scriptIndex, size_t propertyIndex,
                                             const ScriptPropertyValue& newValue) {
    this->project = project;
    this->sceneId = sceneId;
    this->scriptClassName = scriptClassName;
    this->propertyName = propertyName;

    this->values[entity].scriptIndex = scriptIndex;
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

        if (value.scriptIndex >= scriptComp->scripts.size()) {
            return false;
        }

        ScriptEntry& scriptEntry = scriptComp->scripts[value.scriptIndex];

        if (value.propertyIndex >= scriptEntry.properties.size()) {
            return false;
        }

        ScriptProperty& prop = scriptEntry.properties[value.propertyIndex];
        value.oldValue = prop.value;
        prop.value = value.newValue;
        prop.syncToMember();

        if (project->isEntityShared(sceneId, entity)) {
            project->sharedGroupPropertyChanged(sceneId, entity, ComponentType::ScriptComponent, {"scripts"});
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

        if (value.scriptIndex >= scriptComp->scripts.size()) {
            continue;
        }

        ScriptEntry& scriptEntry = scriptComp->scripts[value.scriptIndex];

        if (value.propertyIndex >= scriptEntry.properties.size()) {
            continue;
        }

        ScriptProperty& prop = scriptEntry.properties[value.propertyIndex];
        prop.value = value.oldValue;
        prop.syncToMember();

        if (project->isEntityShared(sceneId, entity)) {
            project->sharedGroupPropertyChanged(sceneId, entity, ComponentType::ScriptComponent, {"scripts"});
        }
    }

    sceneProject->isModified = wasModified;
}

bool Editor::ScriptPropertyCmd::mergeWith(Editor::Command* otherCommand) {
    ScriptPropertyCmd* otherCmd = dynamic_cast<ScriptPropertyCmd*>(otherCommand);
    if (otherCmd != nullptr) {
        if (sceneId == otherCmd->sceneId && 
            scriptClassName == otherCmd->scriptClassName &&
            propertyName == otherCmd->propertyName) {
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