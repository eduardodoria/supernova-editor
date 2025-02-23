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
        static Vector2 decodeVector2(const YAML::Node& node);

        static YAML::Node encodeVector3(const Vector3& vec);
        static Vector3 decodeVector3(const YAML::Node& node);

        static YAML::Node encodeVector4(const Vector4& vec);
        static Vector4 decodeVector4(const YAML::Node& node);

        static YAML::Node encodeQuaternion(const Quaternion& quat);
        static Quaternion decodeQuaternion(const YAML::Node& node);

        static YAML::Node encodeRect(const Rect& rect);
        static Rect decodeRect(const YAML::Node& node);

        static YAML::Node encodeMatrix4(const Matrix4& mat);
        static Matrix4 decodeMatrix4(const YAML::Node& node);

        static YAML::Node encodeTransform(const Transform& transform);
        static Transform decodeTransform(const YAML::Node& node);

        static YAML::Node encodeTexture(const Texture& texture);
        static Texture decodeTexture(const YAML::Node& node);

        static YAML::Node encodeMaterial(const Material& material);
        static Material decodeMaterial(const YAML::Node& node);

        static YAML::Node encodeSubmesh(const Submesh& submesh);
        static Submesh decodeSubmesh(const YAML::Node& node);

        static YAML::Node encodeAABB(const AABB& aabb);
        static AABB decodeAABB(const YAML::Node& node);

        static YAML::Node encodeMeshComponent(const MeshComponent& mesh);
        static MeshComponent decodeMeshComponent(const YAML::Node& node);

    public:
        static YAML::Node encodeProject(Project* project);

        static YAML::Node encodeSceneProject(const SceneProject* sceneProject);
        static void decodeSceneProject(SceneProject* sceneProject, const YAML::Node& node);

        static YAML::Node encodeScene(Scene* scene);

        static YAML::Node encodeEntity(const Entity entity, const Scene* scene);
        static void decodeEntity(Scene* scene, Entity entity, const YAML::Node& entityNode);
    };
}