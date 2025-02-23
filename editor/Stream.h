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
        static YAML::Node encodeVector2(const Vector2& vec);
        static YAML::Node encodeVector3(const Vector3& vec);
        static YAML::Node encodeVector4(const Vector4& vec);
        static YAML::Node encodeQuaternion(const Quaternion& quat);
        static YAML::Node encodeRect(const Rect& rect);
        static YAML::Node encodeMatrix4(const Matrix4& mat);
        static YAML::Node encodeTransform(const Transform& transform);
        static YAML::Node encodeTexture(const Texture& texture);
        static YAML::Node encodeMaterial(const Material& material);
        static YAML::Node encodeSubmesh(const Submesh& submesh);
        static YAML::Node encodeAABB(const AABB& aabb);
        static YAML::Node encodeMeshComponent(const MeshComponent& mesh);

    public:
        static YAML::Node encodeProject(Project* project);
        static YAML::Node encodeSceneProject(const SceneProject* sceneProject);
        static YAML::Node encodeScene(Scene* scene);
        static YAML::Node encodeEntity(const Entity entity, const Scene* scene);

        static Vector2 decodeVector2(const YAML::Node& node);
        static Vector3 decodeVector3(const YAML::Node& node);
        static Vector4 decodeVector4(const YAML::Node& node);
        static Quaternion decodeQuaternion(const YAML::Node& node);
        static Rect decodeRect(const YAML::Node& node);
        static Matrix4 decodeMatrix4(const YAML::Node& node);
        static Transform decodeTransform(const YAML::Node& node);
        static Texture decodeTexture(const YAML::Node& node);
        static Material decodeMaterial(const YAML::Node& node);
        static Submesh decodeSubmesh(const YAML::Node& node);
        static AABB decodeAABB(const YAML::Node& node);
        static MeshComponent decodeMeshComponent(const YAML::Node& node);
        static void decodeEntity(Scene* scene, Entity entity, const YAML::Node& entityNode);
        static void decodeSceneProject(SceneProject* sceneProject, const YAML::Node& node);
    };
}