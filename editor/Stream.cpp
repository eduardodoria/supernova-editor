#include "Stream.h"

using namespace Supernova;

YAML::Node Editor::Stream::encodeProject(Project* project) {
    YAML::Node root;

    root["name"] = "Supernova Project";
    root["selectedScene"] = project->getSelectedSceneId();

    return root;
}

YAML::Node Editor::Stream::encodeSceneProject(const SceneProject* sceneProject) {
    YAML::Node root;

    root["id"] = sceneProject->id;
    root["name"] = sceneProject->name;

    YAML::Node entitiesNode;
    for (const auto& entity : sceneProject->entities) {
        entitiesNode.push_back(encodeEntity(entity, sceneProject->scene));
    }
    root["entities"] = entitiesNode;

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

YAML::Node Editor::Stream::encodeEntity(const Entity entity, const Scene* scene) {
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

YAML::Node Editor::Stream::encodeVector2(const Vector2& vec){
    YAML::Node node;
    node.SetStyle(YAML::EmitterStyle::Flow);
    node.push_back(vec.x);
    node.push_back(vec.y);
    return node;
}

YAML::Node Editor::Stream::encodeVector3(const Vector3& vec) {
    YAML::Node node;
    node.SetStyle(YAML::EmitterStyle::Flow);
    node.push_back(vec.x);
    node.push_back(vec.y);
    node.push_back(vec.z);
    return node;
}

YAML::Node Editor::Stream::encodeVector4(const Vector4& vec){
    YAML::Node node;
    node.SetStyle(YAML::EmitterStyle::Flow);
    node.push_back(vec.x);
    node.push_back(vec.y);
    node.push_back(vec.z);
    node.push_back(vec.w);
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

YAML::Node Editor::Stream::encodeTexture(const Texture& texture) {
    YAML::Node node;
    if (!texture.empty()) {
        node["id"] = texture.getId();
        node["path"] = texture.getPath();
        node["minFilter"] = static_cast<int>(texture.getMinFilter());
        node["magFilter"] = static_cast<int>(texture.getMagFilter());
        node["wrapU"] = static_cast<int>(texture.getWrapU());
        node["wrapV"] = static_cast<int>(texture.getWrapV());

        if (texture.isFramebuffer()) {
            node["isFramebuffer"] = true;
        }

        // Add texture dimensions if available
        if (texture.getWidth() > 0 && texture.getHeight() > 0) {
            node["width"] = texture.getWidth();
            node["height"] = texture.getHeight();
        }

        node["transparent"] = texture.isTransparent();
        node["releaseDataAfterLoad"] = texture.isReleaseDataAfterLoad();
    }
    return node;
}

YAML::Node Editor::Stream::encodeMaterial(const Material& material) {
    YAML::Node node;

    // Encode shader part properties
    node["baseColorFactor"] = encodeVector4(material.baseColorFactor);
    node["metallicFactor"] = material.metallicFactor;
    node["roughnessFactor"] = material.roughnessFactor;
    node["emissiveFactor"] = encodeVector3(material.emissiveFactor);
    node["ambientLight"] = encodeVector3(material.ambientLight);
    node["ambientIntensity"] = material.ambientIntensity;

    // Encode textures using the helper method
    if (!material.baseColorTexture.empty()) {
        node["baseColorTexture"] = encodeTexture(material.baseColorTexture);
    }

    if (!material.emissiveTexture.empty()) {
        node["emissiveTexture"] = encodeTexture(material.emissiveTexture);
    }

    if (!material.metallicRoughnessTexture.empty()) {
        node["metallicRoughnessTexture"] = encodeTexture(material.metallicRoughnessTexture);
    }

    if (!material.occlusionTexture.empty()) {
        node["occlusionTexture"] = encodeTexture(material.occlusionTexture);
    }

    if (!material.normalTexture.empty()) {
        node["normalTexture"] = encodeTexture(material.normalTexture);
    }

    // Encode material name
    node["name"] = material.name;

    return node;
}

YAML::Node Editor::Stream::encodeSubmesh(const Submesh& submesh) {
    YAML::Node node;

    node["material"] = encodeMaterial(submesh.material);

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

Vector2 Editor::Stream::decodeVector2(const YAML::Node& node) {
    return Vector2(node[0].as<float>(), node[1].as<float>());
}

Vector3 Editor::Stream::decodeVector3(const YAML::Node& node) {
    return Vector3(node[0].as<float>(), node[1].as<float>(), node[2].as<float>());
}

Vector4 Editor::Stream::decodeVector4(const YAML::Node& node) {
    return Vector4(node[0].as<float>(), node[1].as<float>(), node[2].as<float>(), node[3].as<float>());
}

Quaternion Editor::Stream::decodeQuaternion(const YAML::Node& node) {
    return Quaternion(node[0].as<float>(), node[1].as<float>(), node[2].as<float>(), node[3].as<float>());
}

Rect Editor::Stream::decodeRect(const YAML::Node& node) {
    return Rect(node[0].as<float>(), node[1].as<float>(), node[2].as<float>(), node[3].as<float>());
}

Matrix4 Editor::Stream::decodeMatrix4(const YAML::Node& node) {
    Matrix4 mat;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            mat[i][j] = node[i][j].as<float>();
        }
    }
    return mat;
}

Transform Editor::Stream::decodeTransform(const YAML::Node& node) {
    Transform transform;

    transform.position = decodeVector3(node["position"]);
    transform.rotation = decodeQuaternion(node["rotation"]);
    transform.scale = decodeVector3(node["scale"]);
    transform.worldPosition = decodeVector3(node["worldPosition"]);
    transform.worldRotation = decodeQuaternion(node["worldRotation"]);
    transform.worldScale = decodeVector3(node["worldScale"]);
    transform.localMatrix = decodeMatrix4(node["localMatrix"]);
    transform.modelMatrix = decodeMatrix4(node["modelMatrix"]);
    transform.normalMatrix = decodeMatrix4(node["normalMatrix"]);
    transform.modelViewProjectionMatrix = decodeMatrix4(node["modelViewProjectionMatrix"]);
    transform.visible = node["visible"].as<bool>();
    transform.parent = node["parent"].as<Entity>();
    transform.distanceToCamera = node["distanceToCamera"].as<float>();
    transform.billboardRotation = decodeQuaternion(node["billboardRotation"]);
    transform.billboard = node["billboard"].as<bool>();
    transform.fakeBillboard = node["fakeBillboard"].as<bool>();
    transform.cylindricalBillboard = node["cylindricalBillboard"].as<bool>();
    transform.needUpdateChildVisibility = node["needUpdateChildVisibility"].as<bool>();
    transform.needUpdate = node["needUpdate"].as<bool>();

    return transform;
}

Texture Editor::Stream::decodeTexture(const YAML::Node& node) {
    Texture texture;
    if (node.IsMap()) { // Check if node has data
        texture.setId(node["id"].as<std::string>());
        texture.setPath(node["path"].as<std::string>());
        texture.setMinFilter(static_cast<TextureFilter>(node["minFilter"].as<int>()));
        texture.setMagFilter(static_cast<TextureFilter>(node["magFilter"].as<int>()));
        texture.setWrapU(static_cast<TextureWrap>(node["wrapU"].as<int>()));
        texture.setWrapV(static_cast<TextureWrap>(node["wrapV"].as<int>()));

        //if (node["isFramebuffer"] && node["isFramebuffer"].as<bool>()) {
        //    texture.setIsFramebuffer(true);
        //}

        //if (node["width"] && node["height"]) {
        //    texture.setWidth(node["width"].as<int>());
        //    texture.setHeight(node["height"].as<int>());
        //}

        //texture.setTransparent(node["transparent"].as<bool>());
        texture.setReleaseDataAfterLoad(node["releaseDataAfterLoad"].as<bool>());
    }
    return texture;
}

Material Editor::Stream::decodeMaterial(const YAML::Node& node) {
    Material material;

    material.baseColorFactor = decodeVector4(node["baseColorFactor"]);
    material.metallicFactor = node["metallicFactor"].as<float>();
    material.roughnessFactor = node["roughnessFactor"].as<float>();
    material.emissiveFactor = decodeVector3(node["emissiveFactor"]);
    material.ambientLight = decodeVector3(node["ambientLight"]);
    material.ambientIntensity = node["ambientIntensity"].as<float>();

    if (node["baseColorTexture"]) {
        material.baseColorTexture = decodeTexture(node["baseColorTexture"]);
    }

    if (node["emissiveTexture"]) {
        material.emissiveTexture = decodeTexture(node["emissiveTexture"]);
    }

    if (node["metallicRoughnessTexture"]) {
        material.metallicRoughnessTexture = decodeTexture(node["metallicRoughnessTexture"]);
    }

    if (node["occlusionTexture"]) {
        material.occlusionTexture = decodeTexture(node["occlusionTexture"]);
    }

    if (node["normalTexture"]) {
        material.normalTexture = decodeTexture(node["normalTexture"]);
    }

    material.name = node["name"].as<std::string>();

    return material;
}

Submesh Editor::Stream::decodeSubmesh(const YAML::Node& node) {
    Submesh submesh;

    submesh.material = decodeMaterial(node["material"]);
    submesh.textureRect = decodeRect(node["textureRect"]);
    submesh.primitiveType = static_cast<PrimitiveType>(node["primitiveType"].as<int>());
    submesh.vertexCount = node["vertexCount"].as<uint32_t>();
    submesh.faceCulling = node["faceCulling"].as<bool>();

    // Flags
    submesh.hasTexCoord1 = node["hasTexCoord1"].as<bool>();
    submesh.hasNormalMap = node["hasNormalMap"].as<bool>();
    submesh.hasTangent = node["hasTangent"].as<bool>();
    submesh.hasVertexColor4 = node["hasVertexColor4"].as<bool>();
    submesh.hasTextureRect = node["hasTextureRect"].as<bool>();
    submesh.hasSkinning = node["hasSkinning"].as<bool>();
    submesh.hasMorphTarget = node["hasMorphTarget"].as<bool>();
    submesh.hasMorphNormal = node["hasMorphNormal"].as<bool>();
    submesh.hasMorphTangent = node["hasMorphTangent"].as<bool>();
    submesh.hasDepthTexture = node["hasDepthTexture"].as<bool>();

    return submesh;
}

AABB Editor::Stream::decodeAABB(const YAML::Node& node) {
    Vector3 min = decodeVector3(node["min"]);
    Vector3 max = decodeVector3(node["max"]);
    return AABB(min, max);
}

MeshComponent Editor::Stream::decodeMeshComponent(const YAML::Node& node) {
    MeshComponent mesh;

    //mesh.loaded = node["loaded"].as<bool>();
    //mesh.loadCalled = node["loadCalled"].as<bool>();
    mesh.vertexCount = node["vertexCount"].as<uint32_t>();
    mesh.numExternalBuffers = node["numExternalBuffers"].as<uint32_t>();
    mesh.numSubmeshes = node["numSubmeshes"].as<uint32_t>();

    // Decode submeshes
    auto submeshesNode = node["submeshes"];
    for(unsigned int i = 0; i < mesh.numSubmeshes; i++) {
        mesh.submeshes[i] = decodeSubmesh(submeshesNode[i]);
    }

    // Decode bones matrix
    auto bonesNode = node["bonesMatrix"];
    for(int i = 0; i < MAX_BONES; i++) {
        mesh.bonesMatrix[i] = decodeMatrix4(bonesNode[i]);
    }

    mesh.normAdjustJoint = node["normAdjustJoint"].as<int>();
    mesh.normAdjustWeight = node["normAdjustWeight"].as<float>();

    // Decode morph weights
    auto morphWeightsNode = node["morphWeights"];
    for(int i = 0; i < MAX_MORPHTARGETS; i++) {
        mesh.morphWeights[i] = morphWeightsNode[i].as<float>();
    }

    mesh.aabb = decodeAABB(node["aabb"]);
    mesh.verticesAABB = decodeAABB(node["verticesAABB"]);
    mesh.worldAABB = decodeAABB(node["worldAABB"]);

    mesh.castShadows = node["castShadows"].as<bool>();
    mesh.receiveShadows = node["receiveShadows"].as<bool>();
    mesh.shadowsBillboard = node["shadowsBillboard"].as<bool>();
    mesh.transparent = node["transparent"].as<bool>();

    mesh.cullingMode = static_cast<CullingMode>(node["cullingMode"].as<int>());
    mesh.windingOrder = static_cast<WindingOrder>(node["windingOrder"].as<int>());

    //mesh.needUpdateBuffer = node["needUpdateBuffer"].as<bool>();
    //mesh.needReload = node["needReload"].as<bool>();

    return mesh;
}

void Editor::Stream::decodeEntity(Scene* scene, Entity entity, const YAML::Node& entityNode) {
    //Entity entity = entityNode["entity"].as<Entity>();
    std::string name = entityNode["name"].as<std::string>();

    scene->setEntityName(entity, name);

    if (entityNode["transform"]) {
        Transform transform = decodeTransform(entityNode["transform"]);
        scene->addComponent<Transform>(entity, transform);
    }

    if (entityNode["mesh"]) {
        MeshComponent mesh = decodeMeshComponent(entityNode["mesh"]);
        scene->addComponent<MeshComponent>(entity, mesh);
    }
}

void Editor::Stream::decodeSceneProject(SceneProject* sceneProject, const YAML::Node& node) {
    //sceneProject->id = node["id"].as<uint32_t>();
    sceneProject->name = node["name"].as<std::string>();

    auto entitiesNode = node["entities"];
    for (const auto& entityNode : entitiesNode) {
        Entity entity = sceneProject->scene->createEntity();
        sceneProject->entities.push_back(entity);
        decodeEntity(sceneProject->scene, entity, entityNode);
    }
}