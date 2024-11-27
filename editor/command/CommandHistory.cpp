#include "CommandHistory.h"
#include <stdio.h>

using namespace Supernova;

Editor::CommandHistory::~CommandHistory(){
    for (int i = 0; i < list.size(); i++){
        delete list[i];
    }
}

void Editor::CommandHistory::addCommand(Editor::Command* cmd){
    cmd->execute();

    if (index < list.size()){
        for (auto it = list.begin() + index; it != list.end(); ++it) {
            delete *it;
        }

        list.erase(list.begin() + index, list.end());
    }

    if (list.size() > 0 && list.back()->canMerge() && cmd->canMerge()){
        if (cmd->mergeWith(list.back())){
            list.pop_back();
        }
    }

    list.push_back(cmd);
    index = list.size();
}

void Editor::CommandHistory::addCommandNoMerge(Command* cmd){
    cmd->setNoMerge();
    addCommand(cmd);
}

void Editor::CommandHistory::undo(){
    if (index > 0){
        list[index-1]->undo();
        index--;

        #ifdef _DEBUG
        printf("DEBUG: undo (%lu from %lu)\n", index, list.size());
        #endif
    }
}

void Editor::CommandHistory::redo(){
    if (index < list.size()){
        index++;
        list[index-1]->execute();

        #ifdef _DEBUG
        printf("DEBUG: redo (%lu from %lu)\n", index, list.size());
        #endif
    }
}