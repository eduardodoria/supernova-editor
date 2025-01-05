#include "Backend.h"

using namespace Supernova;

// Main code
int main(int argc, char* argv[]){
    return Editor::Backend::init(argc, argv);
}

// for SDL
int SDL_main(int argc, char* argv[]){
    return Editor::Backend::init(argc, argv);
}