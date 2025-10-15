#pragma once

#include "command/Command.h"
#include "Project.h"
#include "Catalog.h"
#include "script/ScriptProperty.h"

namespace Supernova::Editor {

    struct ScriptPropertyCmdValue {
        size_t scriptIndex;      // Index of the script in the scripts vector
        size_t propertyIndex;    // Index of the property in that script's properties
        ScriptPropertyValue oldValue;
        ScriptPropertyValue newValue;
    };

    class ScriptPropertyCmd : public Command {
    private:
        Project* project;
        uint32_t sceneId;
        std::string propertyName;
        std::string scriptClassName; // To identify which script the property belongs to
        bool wasModified;

        std::map<Entity, ScriptPropertyCmdValue> values;

    public:
        ScriptPropertyCmd(Project* project, uint32_t sceneId, Entity entity, 
                         const std::string& scriptClassName, const std::string& propertyName,
                         size_t scriptIndex, size_t propertyIndex, 
                         const ScriptPropertyValue& newValue);

        bool execute() override;
        void undo() override;

        bool mergeWith(Command* otherCommand) override;
    };

}