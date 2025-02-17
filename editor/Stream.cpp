#include "Stream.h"

using namespace Supernova;

YAML::Node Editor::Stream::encodeProject(Project* project) {
    YAML::Node root;

    // Save scenes data
    YAML::Node scenesNode;
    for (const auto& scene : project->getScenes()) {
        YAML::Node sceneNode;
        sceneNode["id"] = scene.id;
        sceneNode["name"] = scene.name;

        YAML::Node entitiesNode;
        for (const auto& entity : scene.entities) {
            entitiesNode.push_back(encodeEntity(entity, scene.scene));
        }
        sceneNode["entities"] = entitiesNode;

        scenesNode.push_back(sceneNode);
    }
    root["scenes"] = scenesNode;

    // Save selected scene and last activated scene
    root["selectedScene"] = project->getSelectedSceneId();
    root["lastActivatedScene"] = project->getLastActivatedSceneId();

    return root;
}

YAML::Node Editor::Stream::encodeScene(Scene* scene) {
    YAML::Node sceneNode;
    // Add scene serialization logic here
    return sceneNode;
}

YAML::Node Editor::Stream::encodeTransform(const Transform& transform) {
    YAML::Node transformNode;

    transformNode["position"] = encodeVector3(transform.position);
    transformNode["rotation"] = encodeQuaternion(transform.rotation);
    transformNode["scale"] = encodeVector3(transform.scale);
    transformNode["worldPosition"] = encodeVector3(transform.worldPosition);
    transformNode["worldRotation"] = encodeQuaternion(transform.worldRotation);
    transformNode["worldScale"] = encodeVector3(transform.worldScale);
    transformNode["localMatrix"] = encodeMatrix4(transform.localMatrix);
    transformNode["modelMatrix"] = encodeMatrix4(transform.modelMatrix);
    transformNode["normalMatrix"] = encodeMatrix4(transform.normalMatrix);
    transformNode["modelViewProjectionMatrix"] = encodeMatrix4(transform.modelViewProjectionMatrix);
    transformNode["visible"] = transform.visible;
    transformNode["parent"] = transform.parent;
    transformNode["distanceToCamera"] = transform.distanceToCamera;
    transformNode["billboardRotation"] = encodeQuaternion(transform.billboardRotation);
    transformNode["billboard"] = transform.billboard;
    transformNode["fakeBillboard"] = transform.fakeBillboard;
    transformNode["cylindricalBillboard"] = transform.cylindricalBillboard;
    transformNode["needUpdateChildVisibility"] = transform.needUpdateChildVisibility;
    transformNode["needUpdate"] = transform.needUpdate;

    return transformNode;
}

YAML::Node Editor::Stream::encodeEntity(Entity entity, Scene* scene) {
    YAML::Node entityNode;
    entityNode["entity"] = entity;
    entityNode["name"] = scene->getEntityName(entity);

    Signature signature = scene->getSignature(entity);

    if (signature.test(scene->getComponentId<Transform>())) {
        Transform transform = scene->getComponent<Transform>(entity);
        entityNode["transform"] = encodeTransform(transform);
    }

    if (signature.test(scene->getComponentId<MeshComponent>())) {
        MeshComponent mesh = scene->getComponent<MeshComponent>(entity);
        entityNode["mesh"] = encodeMeshComponent(mesh);
    }

    return entityNode;
}

YAML::Node Editor::Stream::encodeVector3(const Vector3& vec) {
    YAML::Node node;
    node.SetStyle(YAML::EmitterStyle::Flow);
    node.push_back(vec.x);
    node.push_back(vec.y);
    node.push_back(vec.z);
    return node;
}

YAML::Node Editor::Stream::encodeQuaternion(const Quaternion& quat) {
    YAML::Node node;
    node.SetStyle(YAML::EmitterStyle::Flow);
    node.push_back(quat.x);
    node.push_back(quat.y);
    node.push_back(quat.z);
    node.push_back(quat.w);
    return node;
}

YAML::Node Editor::Stream::encodeRect(const Rect& rect) {
    YAML::Node node;
    node.SetStyle(YAML::EmitterStyle::Flow);
    node.push_back(rect.getX());
    node.push_back(rect.getY());
    node.push_back(rect.getWidth());
    node.push_back(rect.getHeight());
    return node;
}

YAML::Node Editor::Stream::encodeMatrix4(const Matrix4& mat) {
    YAML::Node node;
    node.SetStyle(YAML::EmitterStyle::Flow);
    for (int i = 0; i < 4; i++) {
        YAML::Node row;
        row.SetStyle(YAML::EmitterStyle::Flow);
        for (int j = 0; j < 4; j++) {
            row.push_back(mat[i][j]);
        }
        node.push_back(row);
    }
    return node;
}

YAML::Node Editor::Stream::encodeMaterial(const Material& material) {
    YAML::Node node;
    // TODO: Implement material serialization
    return node;
}

YAML::Node Editor::Stream::encodeSubmesh(const Submesh& submesh) {
    YAML::Node node;

    node["material"] = encodeMaterial(submesh.material);
    // TODO: Implement attributes map serialization

    node["textureRect"] = encodeRect(submesh.textureRect);

    node["primitiveType"] = static_cast<int>(submesh.primitiveType);
    node["vertexCount"] = submesh.vertexCount;
    node["faceCulling"] = submesh.faceCulling;

    // Flags
    node["hasTexCoord1"] = submesh.hasTexCoord1;
    node["hasNormalMap"] = submesh.hasNormalMap;
    node["hasTangent"] = submesh.hasTangent;
    node["hasVertexColor4"] = submesh.hasVertexColor4;
    node["hasTextureRect"] = submesh.hasTextureRect;
    node["hasSkinning"] = submesh.hasSkinning;
    node["hasMorphTarget"] = submesh.hasMorphTarget;
    node["hasMorphNormal"] = submesh.hasMorphNormal;
    node["hasMorphTangent"] = submesh.hasMorphTangent;
    node["hasDepthTexture"] = submesh.hasDepthTexture;

    return node;
}

YAML::Node Editor::Stream::encodeAABB(const AABB& aabb) {
    YAML::Node node;
    node["min"] = encodeVector3(aabb.getMinimum());
    node["max"] = encodeVector3(aabb.getMaximum());
    return node;
}

YAML::Node Editor::Stream::encodeMeshComponent(const MeshComponent& mesh) {
    YAML::Node node;

    node["loaded"] = mesh.loaded;
    node["loadCalled"] = mesh.loadCalled;

    // Buffer data might need special handling or might not need to be serialized
    // node["buffer"] = ...
    // node["indices"] = ...
    // node["eBuffers"] = ...

    node["vertexCount"] = mesh.vertexCount;
    node["numExternalBuffers"] = mesh.numExternalBuffers;

    // Encode submeshes
    YAML::Node submeshesNode;
    for(unsigned int i = 0; i < mesh.numSubmeshes; i++) {
        submeshesNode.push_back(encodeSubmesh(mesh.submeshes[i]));
    }
    node["submeshes"] = submeshesNode;
    node["numSubmeshes"] = mesh.numSubmeshes;

    // Encode bones matrix array
    YAML::Node bonesNode;
    for(int i = 0; i < MAX_BONES; i++) {
        bonesNode.push_back(encodeMatrix4(mesh.bonesMatrix[i]));
    }
    node["bonesMatrix"] = bonesNode;

    node["normAdjustJoint"] = mesh.normAdjustJoint;
    node["normAdjustWeight"] = mesh.normAdjustWeight;

    // Encode morph weights array
    YAML::Node morphWeightsNode;
    morphWeightsNode.SetStyle(YAML::EmitterStyle::Flow);
    for(int i = 0; i < MAX_MORPHTARGETS; i++) {
        morphWeightsNode.push_back(mesh.morphWeights[i]);
    }
    node["morphWeights"] = morphWeightsNode;

    // Encode AABBs
    node["aabb"] = encodeAABB(mesh.aabb);
    node["verticesAABB"] = encodeAABB(mesh.verticesAABB);
    node["worldAABB"] = encodeAABB(mesh.worldAABB);

    node["castShadows"] = mesh.castShadows;
    node["receiveShadows"] = mesh.receiveShadows;
    node["shadowsBillboard"] = mesh.shadowsBillboard;
    node["transparent"] = mesh.transparent;

    node["cullingMode"] = static_cast<int>(mesh.cullingMode);
    node["windingOrder"] = static_cast<int>(mesh.windingOrder);

    node["needUpdateBuffer"] = mesh.needUpdateBuffer;
    node["needReload"] = mesh.needReload;

    return node;
}