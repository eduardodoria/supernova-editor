#pragma once

#include "yaml-cpp/yaml.h"
#include "Scene.h"
#include "ecs/Entity.h"
#include "Project.h"
#include "math/Vector3.h"
#include "math/Quaternion.h"
#include "math/Matrix4.h"

namespace Supernova::Editor {
    class Stream {
    private:
        static YAML::Node encodeVector3(const Vector3& vec);
        static YAML::Node encodeQuaternion(const Quaternion& quat);
        static YAML::Node encodeRect(const Rect& rect);
        static YAML::Node encodeMatrix4(const Matrix4& mat);
        static YAML::Node encodeTransform(const Transform& transform);
        static YAML::Node encodeMaterial(const Material& material);
        static YAML::Node encodeSubmesh(const Submesh& submesh);
        static YAML::Node encodeAABB(const AABB& aabb);
        static YAML::Node encodeMeshComponent(const MeshComponent& mesh);

    public:
        static YAML::Node encodeProject(Project* project);
        static YAML::Node encodeScene(Scene* scene);
        static YAML::Node encodeEntity(Entity entity, Scene* scene);
    };
}