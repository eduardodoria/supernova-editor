#include "Stream.h"

#include "Base64.h"
#include "Out.h"

using namespace Supernova;

std::string Editor::Stream::sceneTypeToString(Editor::SceneType type){
    switch (type) {
        case SceneType::SCENE_3D: return "scene_3d";
        case SceneType::SCENE_2D: return "scene_2d";
        case SceneType::SCENE_UI: return "scene_ui";
        default: return "scene_3d";
    }
}

Editor::SceneType Editor::Stream::stringToSceneType(const std::string& str){
    if (str == "scene_3d") return SceneType::SCENE_3D;
    if (str == "scene_2d") return SceneType::SCENE_2D;
    if (str == "scene_ui") return SceneType::SCENE_UI;
    return SceneType::SCENE_3D; // Default
}

std::string Editor::Stream::primitiveTypeToString(PrimitiveType type) {
    switch (type) {
        case PrimitiveType::TRIANGLES: return "triangles";
        case PrimitiveType::TRIANGLE_STRIP: return "triangle_strip";
        case PrimitiveType::POINTS: return "points";
        case PrimitiveType::LINES: return "lines";
        default: return "triangles";
    }
}

PrimitiveType Editor::Stream::stringToPrimitiveType(const std::string& str) {
    if (str == "triangles") return PrimitiveType::TRIANGLES;
    if (str == "triangle_strip") return PrimitiveType::TRIANGLE_STRIP;
    if (str == "points") return PrimitiveType::POINTS;
    if (str == "lines") return PrimitiveType::LINES;
    return PrimitiveType::TRIANGLES; // Default
}

std::string Editor::Stream::bufferTypeToString(BufferType type) {
    switch (type) {
        case BufferType::VERTEX_BUFFER: return "vertex_buffer";
        case BufferType::INDEX_BUFFER: return "index_buffer";
        case BufferType::STORAGE_BUFFER: return "storage_buffer";
        default: return "vertex_buffer";
    }
}

BufferType Editor::Stream::stringToBufferType(const std::string& str) {
    if (str == "vertex_buffer") return BufferType::VERTEX_BUFFER;
    if (str == "index_buffer") return BufferType::INDEX_BUFFER;
    if (str == "storage_buffer") return BufferType::STORAGE_BUFFER;
    return BufferType::VERTEX_BUFFER; // Default
}

// BufferUsage enum conversion
std::string Editor::Stream::bufferUsageToString(BufferUsage usage) {
    switch (usage) {
        case BufferUsage::IMMUTABLE: return "immutable";
        case BufferUsage::DYNAMIC: return "dynamic";
        case BufferUsage::STREAM: return "stream";
        default: return "immutable"; // Default
    }
}

BufferUsage Editor::Stream::stringToBufferUsage(const std::string& str) {
    if (str == "immutable") return BufferUsage::IMMUTABLE;
    if (str == "dynamic") return BufferUsage::DYNAMIC;
    if (str == "stream") return BufferUsage::STREAM;
    return BufferUsage::IMMUTABLE; // Default
}

// AttributeType enum conversion
std::string Editor::Stream::attributeTypeToString(AttributeType type) {
    switch (type) {
        case AttributeType::INDEX: return "index";
        case AttributeType::POSITION: return "position";
        case AttributeType::TEXCOORD1: return "texcoord1";
        case AttributeType::NORMAL: return "normal";
        case AttributeType::TANGENT: return "tangent";
        case AttributeType::COLOR: return "color";
        case AttributeType::POINTSIZE: return "pointsize";
        case AttributeType::POINTROTATION: return "pointrotation";
        case AttributeType::TEXTURERECT: return "texturerect";
        case AttributeType::BONEWEIGHTS: return "boneweights";
        case AttributeType::BONEIDS: return "boneids";
        case AttributeType::MORPHTARGET0: return "morphtarget0";
        case AttributeType::MORPHTARGET1: return "morphtarget1";
        case AttributeType::MORPHTARGET2: return "morphtarget2";
        case AttributeType::MORPHTARGET3: return "morphtarget3";
        case AttributeType::MORPHTARGET4: return "morphtarget4";
        case AttributeType::MORPHTARGET5: return "morphtarget5";
        case AttributeType::MORPHTARGET6: return "morphtarget6";
        case AttributeType::MORPHTARGET7: return "morphtarget7";
        case AttributeType::MORPHNORMAL0: return "morphnormal0";
        case AttributeType::MORPHNORMAL1: return "morphnormal1";
        case AttributeType::MORPHNORMAL2: return "morphnormal2";
        case AttributeType::MORPHNORMAL3: return "morphnormal3";
        case AttributeType::MORPHTANGENT0: return "morphtangent0";
        case AttributeType::MORPHTANGENT1: return "morphtangent1";
        case AttributeType::INSTANCEMATRIXCOL1: return "instancematrixcol1";
        case AttributeType::INSTANCEMATRIXCOL2: return "instancematrixcol2";
        case AttributeType::INSTANCEMATRIXCOL3: return "instancematrixcol3";
        case AttributeType::INSTANCEMATRIXCOL4: return "instancematrixcol4";
        case AttributeType::INSTANCECOLOR: return "instancecolor";
        case AttributeType::INSTANCETEXTURERECT: return "instancetexturerect";
        case AttributeType::TERRAINNODEPOSITION: return "terrainnodeposition";
        case AttributeType::TERRAINNODESIZE: return "terrainnodesize";
        case AttributeType::TERRAINNODERANGE: return "terrainnoderange";
        case AttributeType::TERRAINNODERESOLUTION: return "terrainnoderesolution";
        default: return "position"; // Default
    }
}

AttributeType Editor::Stream::stringToAttributeType(const std::string& str) {
    if (str == "index") return AttributeType::INDEX;
    if (str == "position") return AttributeType::POSITION;
    if (str == "texcoord1") return AttributeType::TEXCOORD1;
    if (str == "normal") return AttributeType::NORMAL;
    if (str == "tangent") return AttributeType::TANGENT;
    if (str == "color") return AttributeType::COLOR;
    if (str == "pointsize") return AttributeType::POINTSIZE;
    if (str == "pointrotation") return AttributeType::POINTROTATION;
    if (str == "texturerect") return AttributeType::TEXTURERECT;
    if (str == "boneweights") return AttributeType::BONEWEIGHTS;
    if (str == "boneids") return AttributeType::BONEIDS;
    if (str == "morphtarget0") return AttributeType::MORPHTARGET0;
    if (str == "morphtarget1") return AttributeType::MORPHTARGET1;
    if (str == "morphtarget2") return AttributeType::MORPHTARGET2;
    if (str == "morphtarget3") return AttributeType::MORPHTARGET3;
    if (str == "morphtarget4") return AttributeType::MORPHTARGET4;
    if (str == "morphtarget5") return AttributeType::MORPHTARGET5;
    if (str == "morphtarget6") return AttributeType::MORPHTARGET6;
    if (str == "morphtarget7") return AttributeType::MORPHTARGET7;
    if (str == "morphnormal0") return AttributeType::MORPHNORMAL0;
    if (str == "morphnormal1") return AttributeType::MORPHNORMAL1;
    if (str == "morphnormal2") return AttributeType::MORPHNORMAL2;
    if (str == "morphnormal3") return AttributeType::MORPHNORMAL3;
    if (str == "morphtangent0") return AttributeType::MORPHTANGENT0;
    if (str == "morphtangent1") return AttributeType::MORPHTANGENT1;
    if (str == "instancematrixcol1") return AttributeType::INSTANCEMATRIXCOL1;
    if (str == "instancematrixcol2") return AttributeType::INSTANCEMATRIXCOL2;
    if (str == "instancematrixcol3") return AttributeType::INSTANCEMATRIXCOL3;
    if (str == "instancematrixcol4") return AttributeType::INSTANCEMATRIXCOL4;
    if (str == "instancecolor") return AttributeType::INSTANCECOLOR;
    if (str == "instancetexturerect") return AttributeType::INSTANCETEXTURERECT;
    if (str == "terrainnodeposition") return AttributeType::TERRAINNODEPOSITION;
    if (str == "terrainnodesize") return AttributeType::TERRAINNODESIZE;
    if (str == "terrainnoderange") return AttributeType::TERRAINNODERANGE;
    if (str == "terrainnoderesolution") return AttributeType::TERRAINNODERESOLUTION;
    return AttributeType::POSITION; // Default
}

// AttributeDataType enum conversion
std::string Editor::Stream::attributeDataTypeToString(AttributeDataType type) {
    switch (type) {
        case AttributeDataType::BYTE: return "byte";
        case AttributeDataType::UNSIGNED_BYTE: return "unsigned_byte";
        case AttributeDataType::SHORT: return "short";
        case AttributeDataType::UNSIGNED_SHORT: return "unsigned_short";
        case AttributeDataType::INT: return "int";
        case AttributeDataType::UNSIGNED_INT: return "unsigned_int";
        case AttributeDataType::FLOAT: return "float";
        default: return "float"; // Default
    }
}

AttributeDataType Editor::Stream::stringToAttributeDataType(const std::string& str) {
    if (str == "byte") return AttributeDataType::BYTE;
    if (str == "unsigned_byte") return AttributeDataType::UNSIGNED_BYTE;
    if (str == "short") return AttributeDataType::SHORT;
    if (str == "unsigned_short") return AttributeDataType::UNSIGNED_SHORT;
    if (str == "int") return AttributeDataType::INT;
    if (str == "unsigned_int") return AttributeDataType::UNSIGNED_INT;
    if (str == "float") return AttributeDataType::FLOAT;
    return AttributeDataType::FLOAT; // Default
}

// CullingMode enum conversion
std::string Editor::Stream::cullingModeToString(CullingMode mode) {
    switch (mode) {
        case CullingMode::BACK: return "back";
        case CullingMode::FRONT: return "front";
        default: return "back";
    }
}

CullingMode Editor::Stream::stringToCullingMode(const std::string& str) {
    if (str == "back") return CullingMode::BACK;
    if (str == "front") return CullingMode::FRONT;
    return CullingMode::BACK;
}

// WindingOrder enum conversion
std::string Editor::Stream::windingOrderToString(WindingOrder order) {
    switch (order) {
        case WindingOrder::CCW: return "ccw";
        case WindingOrder::CW: return "cw";
        default: return "ccw";
    }
}

WindingOrder Editor::Stream::stringToWindingOrder(const std::string& str) {
    if (str == "ccw") return WindingOrder::CCW;
    if (str == "cw") return WindingOrder::CW;
    return WindingOrder::CCW;
}

// TextureFilter enum conversion
std::string Editor::Stream::textureFilterToString(TextureFilter filter) {
    switch (filter) {
        case TextureFilter::NEAREST: return "nearest";
        case TextureFilter::LINEAR: return "linear";
        case TextureFilter::NEAREST_MIPMAP_NEAREST: return "nearest_mipmap_nearest";
        case TextureFilter::NEAREST_MIPMAP_LINEAR: return "nearest_mipmap_linear";
        case TextureFilter::LINEAR_MIPMAP_NEAREST: return "linear_mipmap_nearest";
        case TextureFilter::LINEAR_MIPMAP_LINEAR: return "linear_mipmap_linear";
        default: return "linear";
    }
}

TextureFilter Editor::Stream::stringToTextureFilter(const std::string& str) {
    if (str == "nearest") return TextureFilter::NEAREST;
    if (str == "linear") return TextureFilter::LINEAR;
    if (str == "nearest_mipmap_nearest") return TextureFilter::NEAREST_MIPMAP_NEAREST;
    if (str == "nearest_mipmap_linear") return TextureFilter::NEAREST_MIPMAP_LINEAR;
    if (str == "linear_mipmap_nearest") return TextureFilter::LINEAR_MIPMAP_NEAREST;
    if (str == "linear_mipmap_linear") return TextureFilter::LINEAR_MIPMAP_LINEAR;
    return TextureFilter::LINEAR;
}

// TextureWrap enum conversion
std::string Editor::Stream::textureWrapToString(TextureWrap wrap) {
    switch (wrap) {
        case TextureWrap::REPEAT: return "repeat";
        case TextureWrap::MIRRORED_REPEAT: return "mirrored_repeat";
        case TextureWrap::CLAMP_TO_EDGE: return "clamp_to_edge";
        case TextureWrap::CLAMP_TO_BORDER: return "clamp_to_border";
        default: return "repeat";
    }
}

TextureWrap Editor::Stream::stringToTextureWrap(const std::string& str) {
    if (str == "repeat") return TextureWrap::REPEAT;
    if (str == "mirrored_repeat") return TextureWrap::MIRRORED_REPEAT;
    if (str == "clamp_to_edge") return TextureWrap::CLAMP_TO_EDGE;
    if (str == "clamp_to_border") return TextureWrap::CLAMP_TO_BORDER;
    return TextureWrap::REPEAT;
}

std::string Editor::Stream::lightTypeToString(LightType type) {
    switch (type) {
        case LightType::DIRECTIONAL: return "directional";
        case LightType::POINT: return "point";
        case LightType::SPOT: return "spot";
        default: return "directional";
    }
}

LightType Editor::Stream::stringToLightType(const std::string& str) {
    if (str == "directional") return LightType::DIRECTIONAL;
    if (str == "point") return LightType::POINT;
    if (str == "spot") return LightType::SPOT;
    return LightType::DIRECTIONAL; // Default
}

std::string Editor::Stream::lightStateToString(LightState state) {
    switch (state) {
        case LightState::OFF: return "off";
        case LightState::ON: return "on";
        case LightState::AUTO: return "auto";
        default: return "auto";
    }
}

LightState Editor::Stream::stringToLightState(const std::string& str) {
    if (str == "off") return LightState::OFF;
    if (str == "on") return LightState::ON;
    if (str == "auto") return LightState::AUTO;
    return LightState::AUTO; // Default
}

YAML::Node Editor::Stream::encodeVector2(const Vector2& vec){
    YAML::Node node;
    node.SetStyle(YAML::EmitterStyle::Flow);
    node.push_back(vec.x);
    node.push_back(vec.y);
    return node;
}

Vector2 Editor::Stream::decodeVector2(const YAML::Node& node) {
    return Vector2(node[0].as<float>(), node[1].as<float>());
}

YAML::Node Editor::Stream::encodeVector3(const Vector3& vec) {
    YAML::Node node;
    node.SetStyle(YAML::EmitterStyle::Flow);
    node.push_back(vec.x);
    node.push_back(vec.y);
    node.push_back(vec.z);
    return node;
}

Vector3 Editor::Stream::decodeVector3(const YAML::Node& node) {
    return Vector3(node[0].as<float>(), node[1].as<float>(), node[2].as<float>());
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

Vector4 Editor::Stream::decodeVector4(const YAML::Node& node) {
    return Vector4(node[0].as<float>(), node[1].as<float>(), node[2].as<float>(), node[3].as<float>());
}

YAML::Node Editor::Stream::encodeQuaternion(const Quaternion& quat) {
    YAML::Node node;
    node.SetStyle(YAML::EmitterStyle::Flow);
    node.push_back(quat.w);
    node.push_back(quat.x);
    node.push_back(quat.y);
    node.push_back(quat.z);
    return node;
}

Quaternion Editor::Stream::decodeQuaternion(const YAML::Node& node) {
    return Quaternion(node[0].as<float>(), node[1].as<float>(), node[2].as<float>(), node[3].as<float>());
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

Rect Editor::Stream::decodeRect(const YAML::Node& node) {
    return Rect(node[0].as<float>(), node[1].as<float>(), node[2].as<float>(), node[3].as<float>());
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

Matrix4 Editor::Stream::decodeMatrix4(const YAML::Node& node) {
    Matrix4 mat;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            mat[i][j] = node[i][j].as<float>();
        }
    }
    return mat;
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
    //transformNode["parent"] = transform.parent;
    transformNode["distanceToCamera"] = transform.distanceToCamera;
    transformNode["billboardRotation"] = encodeQuaternion(transform.billboardRotation);
    transformNode["billboard"] = transform.billboard;
    transformNode["fakeBillboard"] = transform.fakeBillboard;
    transformNode["cylindricalBillboard"] = transform.cylindricalBillboard;
    //transformNode["needUpdateChildVisibility"] = transform.needUpdateChildVisibility;
    //transformNode["needUpdate"] = transform.needUpdate;

    return transformNode;
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
    //transform.parent = node["parent"].as<Entity>();
    transform.distanceToCamera = node["distanceToCamera"].as<float>();
    transform.billboardRotation = decodeQuaternion(node["billboardRotation"]);
    transform.billboard = node["billboard"].as<bool>();
    transform.fakeBillboard = node["fakeBillboard"].as<bool>();
    transform.cylindricalBillboard = node["cylindricalBillboard"].as<bool>();
    //transform.needUpdateChildVisibility = node["needUpdateChildVisibility"].as<bool>();
    //transform.needUpdate = node["needUpdate"].as<bool>();

    return transform;
}

YAML::Node Editor::Stream::encodeTexture(const Texture& texture) {
    YAML::Node node;
    if (!texture.empty()) {
        node["id"] = texture.getId();
        node["path"] = texture.getPath();
        node["minFilter"] = textureFilterToString(texture.getMinFilter());
        node["magFilter"] = textureFilterToString(texture.getMagFilter());
        node["wrapU"] = textureWrapToString(texture.getWrapU());
        node["wrapV"] = textureWrapToString(texture.getWrapV());

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

Texture Editor::Stream::decodeTexture(const YAML::Node& node) {
    Texture texture;
    if (node.IsMap()) { // Check if node has data
        texture.setId(node["id"].as<std::string>());
        texture.setPath(node["path"].as<std::string>());
        texture.setMinFilter(stringToTextureFilter(node["minFilter"].as<std::string>()));
        texture.setMagFilter(stringToTextureFilter(node["magFilter"].as<std::string>()));
        texture.setWrapU(stringToTextureWrap(node["wrapU"].as<std::string>()));
        texture.setWrapV(stringToTextureWrap(node["wrapV"].as<std::string>()));

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

YAML::Node Editor::Stream::encodeBuffer(const Buffer& buffer) {
    YAML::Node node;

    // Encode buffer properties
    node["type"] = bufferTypeToString(buffer.getType());
    node["usage"] = bufferUsageToString(buffer.getUsage());
    node["stride"] = buffer.getStride();
    node["size"] = buffer.getSize();
    node["count"] = buffer.getCount();
    node["renderAttributes"] = buffer.isRenderAttributes();
    node["instanceBuffer"] = buffer.isInstanceBuffer();

    // Encode attributes
    YAML::Node attributesNode;
    for (const auto& [type, attr] : buffer.getAttributes()) {
        YAML::Node attrNode;
        attrNode["type"] = attributeTypeToString(type);
        attrNode["dataType"] = attributeDataTypeToString(attr.getDataType());
        attrNode["bufferName"] = attr.getBufferName();
        attrNode["elements"] = attr.getElements();
        attrNode["offset"] = attr.getOffset();
        attrNode["count"] = attr.getCount();
        attrNode["normalized"] = attr.getNormalized();
        attrNode["perInstance"] = attr.getPerInstance();
        attributesNode.push_back(attrNode);
    }
    node["attributes"] = attributesNode;

    // Encode buffer data if it exists
    if (buffer.getData() && buffer.getSize() > 0) {
        std::string base64Data = Base64::encode(buffer.getData(), buffer.getSize());
        node["data"] = base64Data;
    }

    return node;
}

void Editor::Stream::decodeBuffer(Buffer& buffer, const YAML::Node& node) {
    if (!node.IsMap()) return;

    // Decode buffer properties
    buffer.setType(stringToBufferType(node["type"].as<std::string>()));
    buffer.setUsage(stringToBufferUsage(node["usage"].as<std::string>()));
    buffer.setStride(node["stride"].as<unsigned int>());
    buffer.setSize(node["size"].as<size_t>());
    buffer.setCount(node["count"].as<unsigned int>());
    buffer.setRenderAttributes(node["renderAttributes"].as<bool>());
    buffer.setInstanceBuffer(node["instanceBuffer"].as<bool>());

    // Decode attributes
    if (node["attributes"]) {
        for (const auto& attrNode : node["attributes"]) {
            AttributeType type = stringToAttributeType(attrNode["type"].as<std::string>());
            AttributeDataType dataType = stringToAttributeDataType(attrNode["dataType"].as<std::string>());
            std::string bufferName = attrNode["bufferName"].as<std::string>();
            unsigned int elements = attrNode["elements"].as<unsigned int>();
            size_t offset = attrNode["offset"].as<size_t>();
            unsigned int count = attrNode["count"].as<unsigned int>();
            bool normalized = attrNode["normalized"].as<bool>();
            bool perInstance = attrNode["perInstance"].as<bool>();

            Attribute attr(dataType, bufferName, elements, offset, count, normalized, perInstance);
            buffer.addAttribute(type, attr);
        }
    }

    // Decode buffer data if it exists
    if (node["data"]) {
        std::string base64Data = node["data"].as<std::string>();
        std::vector<unsigned char> decodedData = Base64::decode(base64Data);

        buffer.importData(decodedData.data(), decodedData.size());
    }
}

YAML::Node Editor::Stream::encodeInterleavedBuffer(const InterleavedBuffer& buffer) {
    YAML::Node node = encodeBuffer(buffer);
    node["vertexSize"] = buffer.getVertexSize();
    return node;
}

void Editor::Stream::decodeInterleavedBuffer(InterleavedBuffer& buffer, const YAML::Node& node) {
    decodeBuffer(buffer, node);

    if (node["vertexSize"]) {
        buffer.setVertexSize(node["vertexSize"].as<unsigned int>());
    }
}

YAML::Node Editor::Stream::encodeIndexBuffer(const IndexBuffer& buffer) {
    YAML::Node node = encodeBuffer(buffer);
    return node;
}

void Editor::Stream::decodeIndexBuffer(IndexBuffer& buffer, const YAML::Node& node) {
    decodeBuffer(buffer, node);
}

YAML::Node Editor::Stream::encodeExternalBuffer(const ExternalBuffer& buffer) {
    YAML::Node node = encodeBuffer(buffer); // Use base Buffer encoding
    node["name"] = buffer.getName(); // Add ExternalBuffer specific property
    return node;
}

void Editor::Stream::decodeExternalBuffer(ExternalBuffer& buffer, const YAML::Node& node) {
    decodeBuffer(buffer, node); // Use base Buffer decoding
    buffer.setName(node["name"].as<std::string>()); // Set ExternalBuffer specific property
}

YAML::Node Editor::Stream::encodeSubmesh(const Submesh& submesh) {
    YAML::Node node;

    node["material"] = encodeMaterial(submesh.material);
    node["textureRect"] = encodeRect(submesh.textureRect);
    node["primitiveType"] = primitiveTypeToString(submesh.primitiveType);
    node["vertexCount"] = submesh.vertexCount;
    node["faceCulling"] = submesh.faceCulling;
    node["textureShadow"] = submesh.textureShadow;

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

    return node;
}

Submesh Editor::Stream::decodeSubmesh(const YAML::Node& node) {
    Submesh submesh;

    submesh.material = decodeMaterial(node["material"]);
    submesh.textureRect = decodeRect(node["textureRect"]);
    submesh.primitiveType = stringToPrimitiveType(node["primitiveType"].as<std::string>());
    submesh.vertexCount = node["vertexCount"].as<uint32_t>();
    submesh.faceCulling = node["faceCulling"].as<bool>();
    submesh.textureShadow = node["textureShadow"].as<bool>();

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

    return submesh;
}

YAML::Node Editor::Stream::encodeAABB(const AABB& aabb) {
    YAML::Node node;
    node["min"] = encodeVector3(aabb.getMinimum());
    node["max"] = encodeVector3(aabb.getMaximum());
    return node;
}

AABB Editor::Stream::decodeAABB(const YAML::Node& node) {
    Vector3 min = decodeVector3(node["min"]);
    Vector3 max = decodeVector3(node["max"]);
    return AABB(min, max);
}

YAML::Node Editor::Stream::encodeUIComponent(const UIComponent& ui) {
    YAML::Node node;
    //node["loaded"] = ui.loaded;
    //node["loadCalled"] = ui.loadCalled;
    node["buffer"] = encodeBuffer(ui.buffer);
    node["indices"] = encodeBuffer(ui.indices);
    node["minBufferCount"] = ui.minBufferCount;
    node["minIndicesCount"] = ui.minIndicesCount;

    //node["render"] = {}; // ObjectRender not serialized here
    //node["shader"] = {}; // shared_ptr<ShaderRender> not serialized
    //node["shaderProperties"] = ui.shaderProperties;
    //node["slotVSParams"] = ui.slotVSParams;
    //node["slotFSParams"] = ui.slotFSParams;

    node["primitiveType"] = primitiveTypeToString(ui.primitiveType);
    node["vertexCount"] = ui.vertexCount;

    node["aabb"] = encodeAABB(ui.aabb);
    node["worldAABB"] = encodeAABB(ui.worldAABB);

    node["texture"] = encodeTexture(ui.texture);
    node["color"] = encodeVector4(ui.color);
    // FunctionSubscribe fields are not serializable

    node["automaticFlipY"] = ui.automaticFlipY;
    node["flipY"] = ui.flipY;

    node["pointerMoved"] = ui.pointerMoved;
    node["focused"] = ui.focused;

    //node["needReload"] = ui.needReload;
    //node["needUpdateAABB"] = ui.needUpdateAABB;
    //node["needUpdateBuffer"] = ui.needUpdateBuffer;
    //node["needUpdateTexture"] = ui.needUpdateTexture;

    return node;
}

UIComponent Editor::Stream::decodeUIComponent(const YAML::Node& node) {
    UIComponent ui;
    //ui.loaded = node["loaded"].as<bool>();
    //ui.loadCalled = node["loadCalled"].as<bool>();

    decodeBuffer(ui.buffer, node["buffer"]);
    decodeBuffer(ui.indices, node["indices"]);
    ui.minBufferCount = node["minBufferCount"].as<unsigned int>();
    ui.minIndicesCount = node["minIndicesCount"].as<unsigned int>();

    // ui.render and ui.shader can't be deserialized here
    // ui.shaderProperties = node["shaderProperties"].as<std::string>();
    //ui.slotVSParams = node["slotVSParams"].as<int>();
    //ui.slotFSParams = node["slotFSParams"].as<int>();

    ui.primitiveType = stringToPrimitiveType(node["primitiveType"].as<std::string>());
    ui.vertexCount = node["vertexCount"].as<unsigned int>();

    ui.aabb = decodeAABB(node["aabb"]);
    ui.worldAABB = decodeAABB(node["worldAABB"]);

    ui.texture = decodeTexture(node["texture"]);
    ui.color = decodeVector4(node["color"]);

    ui.automaticFlipY = node["automaticFlipY"].as<bool>();
    ui.flipY = node["flipY"].as<bool>();

    ui.pointerMoved = node["pointerMoved"].as<bool>();
    ui.focused = node["focused"].as<bool>();

    //ui.needReload = node["needReload"].as<bool>();
    //ui.needUpdateAABB = node["needUpdateAABB"].as<bool>();
    //ui.needUpdateBuffer = node["needUpdateBuffer"].as<bool>();
    //ui.needUpdateTexture = node["needUpdateTexture"].as<bool>();

    return ui;
}

YAML::Node Editor::Stream::encodeUILayoutComponent(const UILayoutComponent& layout) {
    YAML::Node node;
    node["width"] = layout.width;
    node["height"] = layout.height;
    node["anchorPointLeft"] = layout.anchorPointLeft;
    node["anchorPointTop"] = layout.anchorPointTop;
    node["anchorPointRight"] = layout.anchorPointRight;
    node["anchorPointBottom"] = layout.anchorPointBottom;
    node["anchorOffsetLeft"] = layout.anchorOffsetLeft;
    node["anchorOffsetTop"] = layout.anchorOffsetTop;
    node["anchorOffsetRight"] = layout.anchorOffsetRight;
    node["anchorOffsetBottom"] = layout.anchorOffsetBottom;
    node["positionOffset"] = encodeVector2(layout.positionOffset);
    node["anchorPreset"] = static_cast<int>(layout.anchorPreset);
    node["usingAnchors"] = layout.usingAnchors;
    node["panel"] = layout.panel;
    node["containerBoxIndex"] = layout.containerBoxIndex;
    node["scissor"] = encodeRect(layout.scissor);
    node["ignoreScissor"] = layout.ignoreScissor;
    node["ignoreEvents"] = layout.ignoreEvents;
    //node["needUpdateSizes"] = layout.needUpdateSizes;

    return node;
}

UILayoutComponent Editor::Stream::decodeUILayoutComponent(const YAML::Node& node) {
    UILayoutComponent layout;
    layout.width = node["width"].as<unsigned int>();
    layout.height = node["height"].as<unsigned int>();
    layout.anchorPointLeft = node["anchorPointLeft"].as<float>();
    layout.anchorPointTop = node["anchorPointTop"].as<float>();
    layout.anchorPointRight = node["anchorPointRight"].as<float>();
    layout.anchorPointBottom = node["anchorPointBottom"].as<float>();
    layout.anchorOffsetLeft = node["anchorOffsetLeft"].as<int>();
    layout.anchorOffsetTop = node["anchorOffsetTop"].as<int>();
    layout.anchorOffsetRight = node["anchorOffsetRight"].as<int>();
    layout.anchorOffsetBottom = node["anchorOffsetBottom"].as<int>();
    layout.positionOffset = decodeVector2(node["positionOffset"]);
    layout.anchorPreset = static_cast<AnchorPreset>(node["anchorPreset"].as<int>());
    layout.usingAnchors = node["usingAnchors"].as<bool>();
    layout.panel = node["panel"].as<Entity>();
    layout.containerBoxIndex = node["containerBoxIndex"].as<int>();
    layout.scissor = decodeRect(node["scissor"]);
    layout.ignoreScissor = node["ignoreScissor"].as<bool>();
    layout.ignoreEvents = node["ignoreEvents"].as<bool>();
    //layout.needUpdateSizes = node["needUpdateSizes"].as<bool>();

    return layout;
}

YAML::Node Editor::Stream::encodeImageComponent(const ImageComponent& image) {
    YAML::Node node;
    node["patchMarginLeft"] = image.patchMarginLeft;
    node["patchMarginRight"] = image.patchMarginRight;
    node["patchMarginTop"] = image.patchMarginTop;
    node["patchMarginBottom"] = image.patchMarginBottom;
    node["textureScaleFactor"] = image.textureScaleFactor;
    //node["needUpdatePatches"] = image.needUpdatePatches;

    return node;
}

ImageComponent Editor::Stream::decodeImageComponent(const YAML::Node& node) {
    ImageComponent image;
    image.patchMarginLeft = node["patchMarginLeft"].as<int>();
    image.patchMarginRight = node["patchMarginRight"].as<int>();
    image.patchMarginTop = node["patchMarginTop"].as<int>();
    image.patchMarginBottom = node["patchMarginBottom"].as<int>();
    image.textureScaleFactor = node["textureScaleFactor"].as<float>();
    //image.needUpdatePatches = node["needUpdatePatches"].as<bool>();

    return image;
}

YAML::Node Editor::Stream::encodeLightComponent(const LightComponent& light) {
    YAML::Node node;

    node["type"] = lightTypeToString(light.type);
    node["direction"] = encodeVector3(light.direction);
    node["worldDirection"] = encodeVector3(light.worldDirection);
    node["color"] = encodeVector3(light.color);
    node["range"] = light.range;
    node["intensity"] = light.intensity;
    node["innerConeCos"] = light.innerConeCos;
    node["outerConeCos"] = light.outerConeCos;
    node["shadows"] = light.shadows;
    node["automaticShadowCamera"] = light.automaticShadowCamera;
    node["shadowBias"] = light.shadowBias;
    node["mapResolution"] = light.mapResolution;
    node["shadowCameraNearFar"] = encodeVector2(light.shadowCameraNearFar);
    node["numShadowCascades"] = light.numShadowCascades;

    return node;
}

LightComponent Editor::Stream::decodeLightComponent(const YAML::Node& node) {
    LightComponent light;

    light.type = stringToLightType(node["type"].as<std::string>());
    light.direction = decodeVector3(node["direction"]);
    light.worldDirection = decodeVector3(node["worldDirection"]);
    light.color = decodeVector3(node["color"]);
    light.range = node["range"].as<float>();
    light.intensity = node["intensity"].as<float>();
    light.innerConeCos = node["innerConeCos"].as<float>();
    light.outerConeCos = node["outerConeCos"].as<float>();
    light.shadows = node["shadows"].as<bool>();
    light.automaticShadowCamera = node["automaticShadowCamera"].as<bool>();
    light.shadowBias = node["shadowBias"].as<float>();
    light.mapResolution = node["mapResolution"].as<unsigned int>();
    light.shadowCameraNearFar = decodeVector2(node["shadowCameraNearFar"]);
    light.numShadowCascades = node["numShadowCascades"].as<unsigned int>();

    return light;
}

void Editor::Stream::encodeComponentsAux(YAML::Node& compNode, const Entity entity, const EntityRegistry* registry, Signature signature) {
    if (signature.test(registry->getComponentId<Transform>())) {
        Transform transform = registry->getComponent<Transform>(entity);
        compNode[Catalog::getComponentName(ComponentType::Transform, true)] = encodeTransform(transform);
    }

    if (signature.test(registry->getComponentId<MeshComponent>())) {
        MeshComponent mesh = registry->getComponent<MeshComponent>(entity);
        compNode[Catalog::getComponentName(ComponentType::MeshComponent, true)] = encodeMeshComponent(mesh);
    }

    if (signature.test(registry->getComponentId<UIComponent>())) {
        UIComponent ui = registry->getComponent<UIComponent>(entity);
        compNode[Catalog::getComponentName(ComponentType::UIComponent, true)] = encodeUIComponent(ui);
    }

    if (signature.test(registry->getComponentId<UILayoutComponent>())) {
        UILayoutComponent layout = registry->getComponent<UILayoutComponent>(entity);
        compNode[Catalog::getComponentName(ComponentType::UILayoutComponent, true)] = encodeUILayoutComponent(layout);
    }

    if (signature.test(registry->getComponentId<ImageComponent>())) {
        ImageComponent image = registry->getComponent<ImageComponent>(entity);
        compNode[Catalog::getComponentName(ComponentType::ImageComponent, true)] = encodeImageComponent(image);
    }

    if (signature.test(registry->getComponentId<LightComponent>())) {
        LightComponent light = registry->getComponent<LightComponent>(entity);
        compNode[Catalog::getComponentName(ComponentType::LightComponent, true)] = encodeLightComponent(light);
    }
}

void Editor::Stream::decodeComponentsAux(Entity entity, Entity parent, EntityRegistry* registry, const YAML::Node& compNode){
    std::string compName;

    compName = Catalog::getComponentName(ComponentType::Transform, true);
    if (compNode[compName]) {
        Transform transform = decodeTransform(compNode[compName]);
        transform.parent = parent;
        registry->addComponent<Transform>(entity, transform);
    }

    compName = Catalog::getComponentName(ComponentType::MeshComponent, true);
    if (compNode[compName]) {
        MeshComponent mesh = decodeMeshComponent(compNode[compName]);
        registry->addComponent<MeshComponent>(entity, mesh);
    }

    compName = Catalog::getComponentName(ComponentType::UIComponent, true);
    if (compNode[compName]) {
        UIComponent ui = decodeUIComponent(compNode[compName]);
        registry->addComponent<UIComponent>(entity, ui);
    }

    compName = Catalog::getComponentName(ComponentType::UILayoutComponent, true);
    if (compNode[compName]) {
        UILayoutComponent layout = decodeUILayoutComponent(compNode[compName]);
        registry->addComponent<UILayoutComponent>(entity, layout);
    }

    compName = Catalog::getComponentName(ComponentType::ImageComponent, true);
    if (compNode[compName]) {
        ImageComponent image = decodeImageComponent(compNode[compName]);
        registry->addComponent<ImageComponent>(entity, image);
    }

    compName = Catalog::getComponentName(ComponentType::LightComponent, true);
    if (compNode[compName]) {
        LightComponent light = decodeLightComponent(compNode[compName]);
        registry->addComponent<LightComponent>(entity, light);
    }
}

YAML::Node Editor::Stream::encodeProject(Project* project) {
    YAML::Node root;

    root["name"] = project->getName();
    root["nextSceneId"] = project->getNextSceneId();
    root["selectedScene"] = project->getSelectedSceneId();

    root["windowWidth"] = project->getWindowWidth();
    root["windowHeight"] = project->getWindowHeight();

    // Add scenes array
    YAML::Node scenesNode;
    for (const auto& sceneProject : project->getScenes()) {
        YAML::Node sceneNode;
        if (!sceneProject.filepath.empty()) {
            sceneNode["filepath"] = sceneProject.filepath.string();
            scenesNode.push_back(sceneNode);
        }
    }
    root["scenes"] = scenesNode;

    return root;
}

void Editor::Stream::decodeProject(Project* project, const YAML::Node& node) {
    if (!node.IsMap()) return;

    if (node["name"]) {
        project->setName(node["name"].as<std::string>());
    }

    // Set nextSceneId if it exists in the node and is greater than current
    if (node["nextSceneId"]) {
        uint32_t nextId = node["nextSceneId"].as<uint32_t>();
        if (nextId > project->getNextSceneId()) {
            project->setNextSceneId(nextId);
        }
    }

    if (node["selectedScene"]) {
        project->setSelectedSceneId(node["selectedScene"].as<uint32_t>());
    }

    if (node["windowWidth"] && node["windowHeight"]) {
        project->setWindowSize(
            node["windowWidth"].as<unsigned int>(),
            node["windowHeight"].as<unsigned int>()
        );
    }

    // Load scenes information
    if (node["scenes"]) {
        for (const auto& sceneNode : node["scenes"]) {
            if (sceneNode["filepath"]) {
                fs::path scenePath = sceneNode["filepath"].as<std::string>();
                if (fs::exists(scenePath)) {
                    project->openScene(scenePath);
                }
            }
        }
    }
}

YAML::Node Editor::Stream::encodeSceneProject(const Project* project, const SceneProject* sceneProject){
    YAML::Node root;
    root["id"] = sceneProject->id;
    root["name"] = sceneProject->name;
    root["scene"] = encodeScene(sceneProject->scene);
    root["sceneType"] = sceneTypeToString(sceneProject->sceneType);

    YAML::Node entitiesNode;
    for (Entity entity : sceneProject->entities) {
        if (Transform* transform = sceneProject->scene->findComponent<Transform>(entity)) {
            if (transform->parent == NULL_ENTITY) {
                entitiesNode.push_back(encodeEntity(entity, sceneProject->scene, project, sceneProject));
            }
        }else{
            entitiesNode.push_back(encodeEntity(entity, sceneProject->scene, project, sceneProject));
        }
    }
    root["entities"] = entitiesNode;
    return root;
}

void Editor::Stream::decodeSceneProject(SceneProject* sceneProject, const YAML::Node& node) {
    sceneProject->id = node["id"].as<uint32_t>();
    sceneProject->name = node["name"].as<std::string>();
    sceneProject->scene = decodeScene(sceneProject->scene, node["scene"]);
    sceneProject->sceneType = stringToSceneType(node["sceneType"].as<std::string>());
}

void Editor::Stream::decodeSceneProjectEntities(Project* project, SceneProject* sceneProject, const YAML::Node& node){
    auto entitiesNode = node["entities"];
    for (const auto& entityNode : entitiesNode){
        std::vector<Entity> newEntities = decodeEntity(entityNode, sceneProject->scene, &sceneProject->entities, project, sceneProject, NULL_ENTITY);
    }
}

YAML::Node Editor::Stream::encodeScene(Scene* scene) {
    YAML::Node sceneNode;

    sceneNode["backgroundColor"] = encodeVector4(scene->getBackgroundColor());
    sceneNode["shadowsPCF"] = scene->isShadowsPCF();
    sceneNode["lightState"] = lightStateToString(scene->getLightState());
    sceneNode["globalIlluminationIntensity"] = scene->getGlobalIlluminationIntensity();
    sceneNode["globalIlluminationColor"] = encodeVector3(scene->getGlobalIlluminationColor());
    sceneNode["enableUIEvents"] = scene->isEnableUIEvents();

    return sceneNode;
}

Scene* Editor::Stream::decodeScene(Scene* scene, const YAML::Node& node) {
    if (!scene){
        scene = new Scene();
    }

    if (node["backgroundColor"]) {
        scene->setBackgroundColor(decodeVector4(node["backgroundColor"]));
    }

    if (node["shadowsPCF"]) {
        scene->setShadowsPCF(node["shadowsPCF"].as<bool>());
    }

    if (node["lightState"]) {
        scene->setLightState(stringToLightState(node["lightState"].as<std::string>()));
    }

    if (node["globalIlluminationIntensity"] && node["globalIlluminationColor"]) {
        scene->setGlobalIllumination(
            node["globalIlluminationIntensity"].as<float>(),
            decodeVector3(node["globalIlluminationColor"])
        );
    } else if (node["globalIlluminationIntensity"]) {
        scene->setGlobalIllumination(node["globalIlluminationIntensity"].as<float>());
    } else if (node["globalIlluminationColor"]) {
        scene->setGlobalIllumination(decodeVector3(node["globalIlluminationColor"]));
    }

    if (node["enableUIEvents"]) {
        scene->setEnableUIEvents(node["enableUIEvents"].as<bool>());
    }

    return scene;
}

YAML::Node Editor::Stream::encodeEntity(const Entity entity, const EntityRegistry* registry, const Project* project, const SceneProject* sceneProject, bool keepEntity) {
    std::map<Entity, YAML::Node> entityNodes;

    Signature signature = registry->getSignature(entity);

    if (signature.test(registry->getComponentId<Transform>())) {
        auto transforms = registry->getComponentArray<Transform>();
        size_t firstIndex = transforms->getIndex(entity);

        for (size_t i = firstIndex; i < transforms->size(); ++i) {
            Entity currentEntity = transforms->getEntity(i);

            std::vector<Entity> entities;
            bool hasCurrentEntity = true;
            if (sceneProject){
                std::vector<Entity> entities = sceneProject->entities;
                hasCurrentEntity = std::find(entities.begin(), entities.end(), currentEntity) != entities.end();
            }

            if (hasCurrentEntity) {
                YAML::Node& currentNode = entityNodes[currentEntity];
                currentNode = encodeEntityAux(currentEntity, registry, project, sceneProject, keepEntity);

                Transform& transform = transforms->getComponentFromIndex(i);

                if (entityNodes.find(transform.parent) != entityNodes.end()) {
                    YAML::Node& parentNode = entityNodes[transform.parent];
                    if (!parentNode["children"]) {
                        parentNode["children"] = YAML::Node();
                    }
                    parentNode["children"].push_back(currentNode);
                } else if (i > firstIndex) { // If this is not the first entity
                    break; // No more childs
                }
            }
        }
    }

    return entityNodes[entity];
}

YAML::Node Editor::Stream::encodeEntityAux(const Entity entity, const EntityRegistry* registry, const Project* project, const SceneProject* sceneProject, bool keepEntity) {
    YAML::Node entityNode;

    fs::path sharedPath = "";
    if (project && sceneProject) {
        sharedPath = project->findGroupPathFor(sceneProject->id, entity);
    }

    if (!sharedPath.empty()) {
        const SharedGroup* group = project->getSharedGroup(sharedPath);
        // Check if this is the root entity of the shared group
        if (group->getRootEntity(sceneProject->id) == entity) {
            entityNode["type"] = "SharedEntity";
            entityNode["path"] = sharedPath.string();
        }else{
            entityNode["type"] = "SharedEntityChild";
        }

        if (keepEntity) {
            entityNode["entity"] = entity;
        }

        Signature signature = Catalog::componentTypeMaskToSignature(registry, group->getEntityOverrides(sceneProject->id, entity));
        YAML::Node compNode = entityNode["components"];
        encodeComponentsAux(compNode, entity, registry, signature);

    }else{
        entityNode["type"] = "Entity";

        if (keepEntity) {
            entityNode["entity"] = entity;
        }
        entityNode["name"] = registry->getEntityName(entity);

        Signature signature = registry->getSignature(entity);
        YAML::Node compNode = entityNode["components"];
        encodeComponentsAux(compNode, entity, registry, signature);

    }

    return entityNode;
}

std::vector<Entity> Editor::Stream::decodeEntity(const YAML::Node& entityNode, EntityRegistry* registry, std::vector<Entity>* entities, Project* project, SceneProject* sceneProject, Entity parent) {
    std::vector<Entity> allEntities;

    std::string entityType = entityNode["type"].as<std::string>();

    if (entityType == "SharedEntity") {

        if (project && sceneProject){
            std::filesystem::path sharedPath = entityNode["path"].as<std::string>();
            allEntities = project->importSharedEntity(sceneProject, sharedPath, parent, false, entityNode);
        }

    }else if (entityType == "Entity") {

        Entity entity = NULL_ENTITY;
        if (entityNode["entity"]){
            entity = entityNode["entity"].as<Entity>();
            if (!registry->recreateEntity(entity)){
                entity = registry->createEntity();
            }
        }else{
            entity = registry->createEntity();
        }

        allEntities.push_back(entity);

        if (entities){
            entities->push_back(entity);
        }else if (sceneProject){
            sceneProject->entities.push_back(entity);
        }

        std::string name = entityNode["name"].as<std::string>();
        registry->setEntityName(entity, name);

        decodeComponentsAux(entity, parent, registry, entityNode["components"]);

        // Decode children from actualNode
        if (entityNode["children"]) {
            for (const auto& childNode : entityNode["children"]) {
                std::vector<Entity> childEntities = decodeEntity(childNode, registry, entities, project, sceneProject, entity);
                std::copy(childEntities.begin(), childEntities.end(), std::back_inserter(allEntities));
            }
        }

    }

    return allEntities;
}

YAML::Node Editor::Stream::encodeMaterial(const Material& material) {
    YAML::Node node;

    // Encode shader part properties
    node["baseColorFactor"] = encodeVector4(material.baseColorFactor);
    node["metallicFactor"] = material.metallicFactor;
    node["roughnessFactor"] = material.roughnessFactor;
    node["emissiveFactor"] = encodeVector3(material.emissiveFactor);

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

Material Editor::Stream::decodeMaterial(const YAML::Node& node) {
    Material material;

    material.baseColorFactor = decodeVector4(node["baseColorFactor"]);
    material.metallicFactor = node["metallicFactor"].as<float>();
    material.roughnessFactor = node["roughnessFactor"].as<float>();
    material.emissiveFactor = decodeVector3(node["emissiveFactor"]);

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

YAML::Node Editor::Stream::encodeMeshComponent(const MeshComponent& mesh) {
    YAML::Node node;

    //node["loaded"] = mesh.loaded;
    //node["loadCalled"] = mesh.loadCalled;

    node["buffer"] = encodeInterleavedBuffer(mesh.buffer);
    node["indices"] = encodeIndexBuffer(mesh.indices);

    // Encode external buffers
    YAML::Node eBuffersNode;
    for (unsigned int i = 0; i < mesh.numExternalBuffers; i++) {
        eBuffersNode.push_back(encodeExternalBuffer(mesh.eBuffers[i]));
    }
    node["eBuffers"] = eBuffersNode;

    //node["vertexCount"] = mesh.vertexCount;

    // Encode submeshes
    YAML::Node submeshesNode;
    for(unsigned int i = 0; i < mesh.numSubmeshes; i++) {
        submeshesNode.push_back(encodeSubmesh(mesh.submeshes[i]));
    }
    node["submeshes"] = submeshesNode;
    //node["numSubmeshes"] = mesh.numSubmeshes;

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

    node["receiveLights"] = mesh.receiveLights;
    node["castShadows"] = mesh.castShadows;
    node["receiveShadows"] = mesh.receiveShadows;
    node["shadowsBillboard"] = mesh.shadowsBillboard;
    node["transparent"] = mesh.transparent;

    node["cullingMode"] = cullingModeToString(mesh.cullingMode);
    node["windingOrder"] = windingOrderToString(mesh.windingOrder);

    //node["needUpdateBuffer"] = mesh.needUpdateBuffer;
    //node["needReload"] = mesh.needReload;

    return node;
}

MeshComponent Editor::Stream::decodeMeshComponent(const YAML::Node& node) {
    MeshComponent mesh;

    //mesh.loaded = node["loaded"].as<bool>();
    //mesh.loadCalled = node["loadCalled"].as<bool>();

    // Decode buffers using generic methods
    if (node["buffer"]) {
        decodeInterleavedBuffer(mesh.buffer, node["buffer"]);
    }

    if (node["indices"]) {
        decodeIndexBuffer(mesh.indices, node["indices"]);
    }

    // Decode external buffers
    if (node["eBuffers"]) {
        auto eBuffersNode = node["eBuffers"];
        for (unsigned int i = 0; i < eBuffersNode.size() && i < MAX_EXTERNAL_BUFFERS; i++) {
            decodeExternalBuffer(mesh.eBuffers[i], eBuffersNode[i]);
        }
        mesh.numExternalBuffers = eBuffersNode.size();
    }

    //mesh.vertexCount = node["vertexCount"].as<uint32_t>();

    // Decode submeshes
    auto submeshesNode = node["submeshes"];
    for(unsigned int i = 0; i < submeshesNode.size() && i < MAX_SUBMESHES; i++) {
        mesh.submeshes[i] = decodeSubmesh(submeshesNode[i]);
    }
    mesh.numSubmeshes = submeshesNode.size();

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

    mesh.receiveLights = node["receiveLights"].as<bool>();
    mesh.castShadows = node["castShadows"].as<bool>();
    mesh.receiveShadows = node["receiveShadows"].as<bool>();
    mesh.shadowsBillboard = node["shadowsBillboard"].as<bool>();
    mesh.transparent = node["transparent"].as<bool>();

    mesh.cullingMode = stringToCullingMode(node["cullingMode"].as<std::string>());
    mesh.windingOrder = stringToWindingOrder(node["windingOrder"].as<std::string>());

    //mesh.needUpdateBuffer = node["needUpdateBuffer"].as<bool>();
    //mesh.needReload = node["needReload"].as<bool>();

    return mesh;
}