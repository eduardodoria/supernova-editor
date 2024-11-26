#include "CommandHandle.h"

using namespace Supernova;

std::map<size_t, Editor::CommandHistory*>  Editor::CommandHandle::historys;


Editor::CommandHistory* Editor::CommandHandle::get(size_t sceneId){
    if (!historys.count(sceneId)){
        historys[sceneId] = new CommandHistory();
    }
    return historys[sceneId];
}

void Editor::CommandHandle::remove(size_t sceneId){
    if (historys.count(sceneId)){
        delete historys[sceneId];
        historys.erase(sceneId);
    }
}