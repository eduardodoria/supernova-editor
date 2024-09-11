#include "App.h"

#include "Engine.h"
#include "Supernova.h"

using namespace Supernova;

Editor::Frame* Editor::App::frame;

bool Editor::App::OnInit(){
    frame = new Editor::Frame("Supernova Editor", wxDefaultPosition, wxSize(1280, 800));
    frame->Show(true);

    Engine::systemInit(argc, argv);



    Supernova::Scene* scene = new Supernova::Scene();
    Supernova::Polygon* triangle = new Supernova::Polygon(scene);

    triangle->addVertex(0, -100);
    triangle->addVertex(-50, 50);
    triangle->addVertex(50, 50);

    triangle->setPosition(Supernova::Vector3(300,300,0));
    triangle->setColor(0.6, 0.2, 0.6, 1);

    //Engine::setCanvasSize(1000,480);
    Supernova::Engine::setFixedTimeSceneUpdate(false);
    Supernova::Engine::setScene(scene);


    return true;
}

Editor::Frame* Editor::App::getFrame(){
    return frame;
}