#include "CommandHistory.h"

using namespace Supernova;

Editor::CommandHistory::CommandHistory(){
    index = 0;
}

void Editor::CommandHistory::addCommand(Editor::Command* cmd){
    cmd->execute();

    if ((index + 1) < list.size()){
        list.erase(list.begin() + (index + 1), list.end());
    }

    list.push_back(cmd);
    index = list.size() - 1;
}

void Editor::CommandHistory::undo(){
    if (list.size() > 0 && index >= 0){
        list[index]->undo();

        if ((index - 1) >= 0){
            index--;
        }
    }
}

void Editor::CommandHistory::redo(){

}