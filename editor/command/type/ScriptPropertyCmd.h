#pragma once

#include "command/Command.h"
#include "Project.h"
#include "Catalog.h"
#include "util/ScriptProperty.h"

namespace Supernova::Editor {

    struct ScriptPropertyCmdValue {
        size_t propertyIndex;
        ScriptPropertyValue oldValue;
        ScriptPropertyValue newValue;
    };

    class ScriptPropertyCmd : public Command {
    private:
        Project* project;
        uint32_t sceneId;
        std::string propertyName;
        bool wasModified;

        std::map<Entity, ScriptPropertyCmdValue> values;

    public:
        ScriptPropertyCmd(Project* project, uint32_t sceneId, Entity entity, 
                         const std::string& propertyName, size_t propertyIndex, 
                         const ScriptPropertyValue& newValue);

        bool execute() override;
        void undo() override;

        bool mergeWith(Command* otherCommand) override;
    };

}