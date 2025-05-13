#pragma once

#include "command/Command.h"
#include "Scene.h"
#include "Catalog.h"

namespace Supernova::Editor {

    template <typename T>
    struct ScenePropertyCmdValue {
        T oldValue;
        T newValue;
    };

    template <typename T>
    class ScenePropertyCmd : public Command {

    private:
        Scene* scene;
        std::string propertyName;
        ScenePropertyCmdValue<T> value;

    public:
        ScenePropertyCmd(Scene* scene, const std::string& propertyName, const T& newValue) {
            this->scene = scene;
            this->propertyName = propertyName;
            this->value.newValue = newValue;
        }

        bool execute() override {
            // Store the old value before changing it
            value.oldValue = Catalog::getSceneProperty<T>(scene, propertyName);
            Catalog::setSceneProperty<T>(scene, propertyName, value.newValue);
            return true;
        }

        void undo() override {
            Catalog::setSceneProperty<T>(scene, propertyName, value.oldValue);
        }

        bool mergeWith(Editor::Command* otherCommand) override {
            ScenePropertyCmd* otherCmd = dynamic_cast<ScenePropertyCmd*>(otherCommand);
            if (otherCmd != nullptr) {
                if (scene == otherCmd->scene && propertyName == otherCmd->propertyName) {
                    // Update oldValue from the earliest command
                    value.oldValue = otherCmd->value.oldValue;
                    value.newValue = otherCmd->value.newValue;
                    return true;
                }
            }
            return false;
        }
    };

}