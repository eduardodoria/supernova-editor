#include "CommandHistory.h"

using namespace Supernova;

Editor::CommandHistory::CommandHistory(){
    index = 0;
}

void Editor::CommandHistory::addCommand(Editor::Command* cmd){
    cmd->execute();

    if (index < list.size()){
        list.erase(list.begin() + index, list.end());
    }

    list.push_back(cmd);
    index = list.size();
}

void Editor::CommandHistory::undo(){
    if (index > 0){
        list[index-1]->undo();
        index--;
    }
}

void Editor::CommandHistory::redo(){
    if (index < list.size()){
        index++;
        list[index-1]->execute();
    }
}