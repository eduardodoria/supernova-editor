#pragma once

#include "command/Command.h"
#include "PropertyCmd.h"
#include <vector>
#include <memory>

namespace Supernova::Editor {

    class MultiPropertyCmd : public Command {
    private:
        std::vector<std::unique_ptr<Command>> commands;

    public:
        MultiPropertyCmd() {
        }

        template<typename T>
        void addPropertyCmd(Scene* scene, Entity entity, ComponentType type, 
                           std::string propertyName, int updateFlags, T newValue) {
            commands.push_back(std::make_unique<PropertyCmd<T>>(
                scene, entity, type, propertyName, updateFlags, newValue));
        }

        void addCommand(std::unique_ptr<Command> command) {
            commands.push_back(std::move(command));
        }

        void execute() override {
            for (auto& cmd : commands) {
                cmd->execute();
            }
        }

        void undo() override {
            // Undo in reverse order
            for (auto it = commands.rbegin(); it != commands.rend(); ++it) {
                (*it)->undo();
            }
        }

        bool mergeWith(Command* otherCommand) override {
            MultiPropertyCmd* otherMultiCmd = dynamic_cast<MultiPropertyCmd*>(otherCommand);
            if (otherMultiCmd) {
                bool anyMerged = false;
                
                for (size_t i = 0; i < otherMultiCmd->commands.size(); i++) {
                    bool merged = false;
                    
                    for (size_t j = 0; j < commands.size(); j++) {
                        if (commands[j]->mergeWith(otherMultiCmd->commands[i].get())) {
                            merged = true;
                            anyMerged = true;
                            break;
                        }
                    }

                    if (!merged) {
                        commands.push_back(std::move(otherMultiCmd->commands[i]));
                    }
                }
                
                return anyMerged;
            }
            
            return false;
        }
    };

}