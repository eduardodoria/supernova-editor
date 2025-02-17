#include "Stream.h"

using namespace Supernova;

void Editor::Stream::serializeEntity(YAML::Emitter& out, Entity entity, Scene* scene){
    out << YAML::BeginMap;
    out << YAML::Key << "entity" << YAML::Value << entity;
    out << YAML::Key << "name" << YAML::Value << scene->getEntityName(entity);
    out << YAML::EndMap;
}