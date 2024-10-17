
#include "ChangeVec3Cmd.h"

using namespace Supernova;

Editor::ChangeVec3Cmd::ChangeVec3Cmd(Vector3& originalVector, Vector3& newVector): vector(originalVector){
    //this->vector = originalVector;
    this->oldVector = Vector3();
    this->newVector = newVector;
}

void Editor::ChangeVec3Cmd::execute(){
    oldVector = Vector3(vector);
    vector = newVector;
}

void Editor::ChangeVec3Cmd::undo(){
    vector = oldVector;
}