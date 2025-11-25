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

std::string Editor::Stream::cameraTypeToString(CameraType type) {
    switch (type) {
        case CameraType::CAMERA_2D: return "camera_2d";
        case CameraType::CAMERA_ORTHO: return "camera_ortho";
        case CameraType::CAMERA_PERSPECTIVE: return "camera_perspective";
        default: return "camera_2d";
    }
}

CameraType Editor::Stream::stringToCameraType(const std::string& str) {
    if (str == "camera_2d") return CameraType::CAMERA_2D;
    if (str == "camera_ortho") return CameraType::CAMERA_ORTHO;
    if (str == "camera_perspective") return CameraType::CAMERA_PERSPECTIVE;
    return CameraType::CAMERA_2D;
}

std::string Editor::Stream::pivotPresetToString(PivotPreset preset) {
    switch (preset) {
        case PivotPreset::CENTER:
            return "center";
        case PivotPreset::TOP_CENTER:
            return "top_center";
        case PivotPreset::BOTTOM_CENTER:
            return "bottom_center";
        case PivotPreset::LEFT_CENTER:
            return "left_center";
        case PivotPreset::RIGHT_CENTER:
            return "right_center";
        case PivotPreset::TOP_LEFT:
            return "top_left";
        case PivotPreset::BOTTOM_LEFT:
            return "bottom_left";
        case PivotPreset::TOP_RIGHT:
            return "top_right";
        case PivotPreset::BOTTOM_RIGHT:
            return "bottom_right";
        default:
            return "bottom_left";
    }
}

PivotPreset Editor::Stream::stringToPivotPreset(const std::string& str) {
    if (str == "center") return PivotPreset::CENTER;
    if (str == "top_center") return PivotPreset::TOP_CENTER;
    if (str == "bottom_center") return PivotPreset::BOTTOM_CENTER;
    if (str == "left_center") return PivotPreset::LEFT_CENTER;
    if (str == "right_center") return PivotPreset::RIGHT_CENTER;
    if (str == "top_left") return PivotPreset::TOP_LEFT;
    if (str == "bottom_left") return PivotPreset::BOTTOM_LEFT;
    if (str == "top_right") return PivotPreset::TOP_RIGHT;
    if (str == "bottom_right") return PivotPreset::BOTTOM_RIGHT;
    return PivotPreset::BOTTOM_LEFT;
}

std::string Editor::Stream::scriptTypeToString(ScriptType type) {
    switch (type) {
        case ScriptType::SUBCLASS:     return "subclass";
        case ScriptType::SCRIPT_CLASS: return "script_class";
        case ScriptType::SCRIPT_LUA:   return "script_lua";
        default:                       return "subclass";
    }
}

ScriptType Editor::Stream::stringToScriptType(const std::string& str) {
    if (str == "subclass")     return ScriptType::SUBCLASS;
    if (str == "script_class") return ScriptType::SCRIPT_CLASS;
    if (str == "script_lua")   return ScriptType::SCRIPT_LUA;

    return ScriptType::SUBCLASS;
}

std::string Editor::Stream::entityRefKindToString(EntityRefKind kind){
    switch(kind){
        case EntityRefKind::LocalEntity: return "local_entity";
        case EntityRefKind::SharedEntity: return "shared_entity";
        case EntityRefKind::None:
        default: return "none";
    }
}

EntityRefKind Editor::Stream::stringToEntityRefKind(const std::string& str){
    if (str == "local_entity") return EntityRefKind::LocalEntity;
    if (str == "shared_entity") return EntityRefKind::SharedEntity;
    return EntityRefKind::None;
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

YAML::Node Editor::Stream::encodeSpriteFrameData(const SpriteFrameData& frameData) {
    YAML::Node node;
    node["active"] = frameData.active;
    node["name"] = frameData.name;
    node["rect"] = encodeRect(frameData.rect);
    return node;
}

SpriteFrameData Editor::Stream::decodeSpriteFrameData(const YAML::Node& node) {
    SpriteFrameData frameData;
    if (node["active"]) frameData.active = node["active"].as<bool>();
    if (node["name"]) frameData.name = node["name"].as<std::string>();
    if (node["rect"]) frameData.rect = decodeRect(node["rect"]);
    return frameData;
}

YAML::Node Editor::Stream::encodeEntityRef(const EntityRef& ref) {
    YAML::Node node;
    const EntityLocator& loc = ref.locator;

    if (loc.kind != EntityRefKind::None) {
        YAML::Node locNode;
        locNode["kind"] = entityRefKindToString(loc.kind);

        if (loc.scopedEntity != NULL_ENTITY)
            locNode["scopedEntity"] = loc.scopedEntity;

        if (loc.kind == EntityRefKind::LocalEntity) {
            if (loc.sceneId != 0)
                locNode["sceneId"] = loc.sceneId;
        } else if (loc.kind == EntityRefKind::SharedEntity) {
            if (!loc.sharedPath.empty())
                locNode["sharedPath"] = loc.sharedPath;
        }

        node["locator"] = locNode;
    }
    return node;
}

EntityRef Editor::Stream::decodeEntityRef(const YAML::Node& node) {
    EntityRef ref;
    if (!node || !node.IsMap()) {
        return ref; // defaults: entityIndex = -1, sceneId = 0, entity=NULL_ENTITY, scene=nullptr
    }
    if (node["locator"] && node["locator"].IsMap()) {
        auto locNode = node["locator"];
        EntityLocator loc;

        if (locNode["kind"])
            loc.kind = stringToEntityRefKind(locNode["kind"].as<std::string>());

        if (locNode["scopedEntity"])
            loc.scopedEntity = locNode["scopedEntity"].as<Entity>();

        if (loc.kind == EntityRefKind::LocalEntity) {
            if (locNode["sceneId"])
                loc.sceneId = locNode["sceneId"].as<uint32_t>();
        } else if (loc.kind == EntityRefKind::SharedEntity) {
            if (locNode["sharedPath"])
                loc.sharedPath = locNode["sharedPath"].as<std::string>();
        }

        ref.locator = loc;
    }
    return ref;
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

YAML::Node Editor::Stream::encodeSceneProject(const Project* project, const SceneProject* sceneProject) {
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
        decodeEntity(entityNode, sceneProject->scene, &sceneProject->entities, project, sceneProject);
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

YAML::Node Editor::Stream::encodeEntity(const Entity entity, const EntityRegistry* registry, const Project* project, const SceneProject* sceneProject) {
    std::map<Entity, YAML::Node> entityNodes;

    bool hasCurrentEntity = true;
    if (sceneProject){
        std::vector<Entity> entities = sceneProject->entities;
        hasCurrentEntity = std::find(entities.begin(), entities.end(), entity) != entities.end();
    }

    if (hasCurrentEntity) {
        YAML::Node& currentNode = entityNodes[entity];
        currentNode = encodeEntityAux(entity, registry, project, sceneProject);

        Signature signature = registry->getSignature(entity);

        if (signature.test(registry->getComponentId<Transform>())) {
            auto transforms = registry->getComponentArray<Transform>();
            size_t firstIndex = transforms->getIndex(entity);

            for (size_t i = firstIndex + 1; i < transforms->size(); ++i) {
                Entity currentEntity = transforms->getEntity(i);

                if (sceneProject){
                    std::vector<Entity> entities = sceneProject->entities;
                    hasCurrentEntity = std::find(entities.begin(), entities.end(), currentEntity) != entities.end();
                }

                if (hasCurrentEntity) {
                    YAML::Node& currentNode = entityNodes[currentEntity];
                    currentNode = encodeEntityAux(currentEntity, registry, project, sceneProject);

                    Transform& transform = transforms->getComponentFromIndex(i);

                    if (entityNodes.find(transform.parent) != entityNodes.end()) {
                        YAML::Node& parentNode = entityNodes[transform.parent];
                        if (!parentNode["children"]) {
                            parentNode["children"] = YAML::Node();
                        }
                        parentNode["children"].push_back(currentNode);
                    } else {
                        break; // No more childs
                    }
                }
            }
        }
    }

    return entityNodes[entity];
}

YAML::Node Editor::Stream::encodeEntityAux(const Entity entity, const EntityRegistry* registry, const Project* project, const SceneProject* sceneProject) {
    YAML::Node entityNode;

    fs::path sharedPath = "";
    if (project && sceneProject) {
        sharedPath = project->findGroupPathFor(sceneProject->id, entity);
    }

    if (!sharedPath.empty()) {
        const SharedGroup* group = project->getSharedGroup(sharedPath);
        const uint32_t instanceId = group->getInstanceId(sceneProject->id, entity);
        // Check if this is the root entity of the shared group
        if (group->getRootEntity(sceneProject->id, instanceId) == entity) {
            entityNode["type"] = "SharedEntity";
            entityNode["path"] = sharedPath.string();
        }else{
            entityNode["type"] = "SharedEntityChild";
        }

        entityNode["entity"] = entity;

        Signature signature = Catalog::componentMaskToSignature(registry, group->getEntityOverrides(sceneProject->id, entity));
        entityNode["components"] = encodeComponents(entity, registry, signature);

    }else{
        entityNode["type"] = "Entity";

        entityNode["entity"] = entity;
        entityNode["name"] = registry->getEntityName(entity);

        Signature signature = registry->getSignature(entity);
        entityNode["components"] = encodeComponents(entity, registry, signature);

    }

    return entityNode;
}

YAML::Node Editor::Stream::encodeScriptProperty(const ScriptProperty& prop) {
    YAML::Node node;
    node["name"] = prop.name;
    node["displayName"] = prop.displayName;
    node["type"] = static_cast<int>(prop.type);

    // Store ptrTypeName if it's a pointer type
    if (prop.type == ScriptPropertyType::EntityPointer && !prop.ptrTypeName.empty()) {
        node["ptrTypeName"] = prop.ptrTypeName;
    }

    switch (prop.type) {
        case ScriptPropertyType::Bool:
            node["value"] = std::get<bool>(prop.value);
            node["defaultValue"] = std::get<bool>(prop.defaultValue);
            break;
        case ScriptPropertyType::Int:
            node["value"] = std::get<int>(prop.value);
            node["defaultValue"] = std::get<int>(prop.defaultValue);
            break;
        case ScriptPropertyType::Float:
            node["value"] = std::get<float>(prop.value);
            node["defaultValue"] = std::get<float>(prop.defaultValue);
            break;
        case ScriptPropertyType::String:
            node["value"] = std::get<std::string>(prop.value);
            node["defaultValue"] = std::get<std::string>(prop.defaultValue);
            break;
        case ScriptPropertyType::Vector2:
            node["value"] = encodeVector2(std::get<Vector2>(prop.value));
            node["defaultValue"] = encodeVector2(std::get<Vector2>(prop.defaultValue));
            break;
        case ScriptPropertyType::Vector3:
        case ScriptPropertyType::Color3:
            node["value"] = encodeVector3(std::get<Vector3>(prop.value));
            node["defaultValue"] = encodeVector3(std::get<Vector3>(prop.defaultValue));
            break;
        case ScriptPropertyType::Vector4:
        case ScriptPropertyType::Color4:
            node["value"] = encodeVector4(std::get<Vector4>(prop.value));
            node["defaultValue"] = encodeVector4(std::get<Vector4>(prop.defaultValue));
            break;
        case ScriptPropertyType::EntityPointer:
            node["value"] = encodeEntityRef(std::get<EntityRef>(prop.value));
            node["defaultValue"] = encodeEntityRef(std::get<EntityRef>(prop.defaultValue));
            break;
    }

    return node;
}

ScriptProperty Editor::Stream::decodeScriptProperty(const YAML::Node& node) {
    ScriptProperty prop;

    if (node["name"]) prop.name = node["name"].as<std::string>();
    if (node["displayName"]) prop.displayName = node["displayName"].as<std::string>();
    if (node["type"]) prop.type = static_cast<ScriptPropertyType>(node["type"].as<int>());

    // Restore ptrTypeName if it exists
    if (node["ptrTypeName"]) {
        prop.ptrTypeName = node["ptrTypeName"].as<std::string>();
    }

    if (node["value"]) {
        switch (prop.type) {
            case ScriptPropertyType::Bool:
                prop.value = node["value"].as<bool>();
                prop.defaultValue = node["defaultValue"] ? node["defaultValue"].as<bool>() : false;
                break;
            case ScriptPropertyType::Int:
                prop.value = node["value"].as<int>();
                prop.defaultValue = node["defaultValue"] ? node["defaultValue"].as<int>() : 0;
                break;
            case ScriptPropertyType::Float:
                prop.value = node["value"].as<float>();
                prop.defaultValue = node["defaultValue"] ? node["defaultValue"].as<float>() : 0.0f;
                break;
            case ScriptPropertyType::String:
                prop.value = node["value"].as<std::string>();
                prop.defaultValue = node["defaultValue"] ? node["defaultValue"].as<std::string>() : std::string("");
                break;
            case ScriptPropertyType::Vector2:
                prop.value = decodeVector2(node["value"]);
                prop.defaultValue = node["defaultValue"] ? decodeVector2(node["defaultValue"]) : Vector2();
                break;
            case ScriptPropertyType::Vector3:
            case ScriptPropertyType::Color3:
                prop.value = decodeVector3(node["value"]);
                prop.defaultValue = node["defaultValue"] ? decodeVector3(node["defaultValue"]) : Vector3();
                break;
            case ScriptPropertyType::Vector4:
            case ScriptPropertyType::Color4:
                prop.value = decodeVector4(node["value"]);
                prop.defaultValue = node["defaultValue"] ? decodeVector4(node["defaultValue"]) : Vector4();
                break;
            case ScriptPropertyType::EntityPointer: {
                prop.value = decodeEntityRef(node["value"]);
                prop.defaultValue = node["defaultValue"] ? decodeEntityRef(node["defaultValue"]) : EntityRef();
                break;
            }
        }
    } else {
        // Initialize with default values if no value is provided
        switch (prop.type) {
            case ScriptPropertyType::Bool: prop.value = false; prop.defaultValue = false; break;
            case ScriptPropertyType::Int: prop.value = 0; prop.defaultValue = 0; break;
            case ScriptPropertyType::Float: prop.value = 0.0f; prop.defaultValue = 0.0f; break;
            case ScriptPropertyType::String: prop.value = std::string(""); prop.defaultValue = std::string(""); break;
            case ScriptPropertyType::Vector2: prop.value = Vector2(); prop.defaultValue = Vector2(); break;
            case ScriptPropertyType::Vector3:
            case ScriptPropertyType::Color3: prop.value = Vector3(); prop.defaultValue = Vector3(); break;
            case ScriptPropertyType::Vector4:
            case ScriptPropertyType::Color4: prop.value = Vector4(); prop.defaultValue = Vector4(); break;
            case ScriptPropertyType::EntityPointer: prop.value = EntityRef(); prop.defaultValue = EntityRef(); break;
        }
    }

    return prop;
}

std::vector<Entity> Editor::Stream::decodeEntity(const YAML::Node& entityNode, EntityRegistry* registry, std::vector<Entity>* entities, Project* project, SceneProject* sceneProject, Entity parent, bool returnSharedEntities, bool createNewIfExists) {
    std::vector<Entity> allEntities;

    std::string entityType = entityNode["type"].as<std::string>();

    if (entityType == "SharedEntity") {

        if (project && sceneProject){
            std::filesystem::path sharedPath = entityNode["path"].as<std::string>();
            std::vector<Entity> sharedEntities = project->importSharedEntity(sceneProject, entities, sharedPath, parent, false, entityNode);
            if (returnSharedEntities) {
                allEntities.insert(allEntities.end(), sharedEntities.begin(), sharedEntities.end());
            }
        }

    }else if (entityType == "Entity") {

        Entity entity = NULL_ENTITY;
        if (entityNode["entity"]){
            entity = entityNode["entity"].as<Entity>();
            if (!registry->recreateEntity(entity)){
                if (createNewIfExists){
                    entity = registry->createUserEntity();
                }
            }
        }else{
            entity = registry->createUserEntity();
        }

        allEntities.push_back(entity);

        if (entities){
            entities->push_back(entity);
        }

        std::string name = entityNode["name"].as<std::string>();
        registry->setEntityName(entity, name);

        if (entityNode["components"]){
            decodeComponents(entity, parent, registry, entityNode["components"]);
        }

        // Decode children from actualNode
        if (entityNode["children"]) {
            for (const auto& childNode : entityNode["children"]) {
                std::vector<Entity> childEntities = decodeEntity(childNode, registry, entities, project, sceneProject, entity, returnSharedEntities, createNewIfExists);
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

YAML::Node Editor::Stream::encodeComponents(const Entity entity, const EntityRegistry* registry, Signature signature) {
    YAML::Node compNode;

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

    if (signature.test(registry->getComponentId<SpriteComponent>())) {
        SpriteComponent sprite = registry->getComponent<SpriteComponent>(entity);
        compNode[Catalog::getComponentName(ComponentType::SpriteComponent, true)] = encodeSpriteComponent(sprite);
    }

    if (signature.test(registry->getComponentId<LightComponent>())) {
        LightComponent light = registry->getComponent<LightComponent>(entity);
        compNode[Catalog::getComponentName(ComponentType::LightComponent, true)] = encodeLightComponent(light);
    }

    if (signature.test(registry->getComponentId<CameraComponent>())) {
        CameraComponent camera = registry->getComponent<CameraComponent>(entity);
        compNode[Catalog::getComponentName(ComponentType::CameraComponent, true)] = encodeCameraComponent(camera);
    }

    if (signature.test(registry->getComponentId<ScriptComponent>())) {
        ScriptComponent script = registry->getComponent<ScriptComponent>(entity);
        compNode[Catalog::getComponentName(ComponentType::ScriptComponent, true)] = encodeScriptComponent(script);
    }

    return compNode;
}

void Editor::Stream::decodeComponents(Entity entity, Entity parent, EntityRegistry* registry, const YAML::Node& compNode){
    std::string compName;

    Signature signature = registry->getSignature(entity);

    compName = Catalog::getComponentName(ComponentType::Transform, true);
    if (compNode[compName]) {
        Transform transform = decodeTransform(compNode[compName], registry->findComponent<Transform>(entity));
        transform.parent = parent;
        if (!signature.test(registry->getComponentId<Transform>())){
            registry->addComponent<Transform>(entity, transform);
        }else{
            registry->addEntityChild(transform.parent, entity, false);
            registry->getComponent<Transform>(entity) = transform;
        }
    }

    compName = Catalog::getComponentName(ComponentType::MeshComponent, true);
    if (compNode[compName]) {
        MeshComponent mesh = decodeMeshComponent(compNode[compName], registry->findComponent<MeshComponent>(entity));
        if (!signature.test(registry->getComponentId<MeshComponent>())){
            registry->addComponent<MeshComponent>(entity, mesh);
        }else{
            registry->getComponent<MeshComponent>(entity) = mesh;
        }
    }

    compName = Catalog::getComponentName(ComponentType::UIComponent, true);
    if (compNode[compName]) {
        UIComponent ui = decodeUIComponent(compNode[compName], registry->findComponent<UIComponent>(entity));
        if (!signature.test(registry->getComponentId<UIComponent>())){
            registry->addComponent<UIComponent>(entity, ui);
        }else{
            registry->getComponent<UIComponent>(entity) = ui;
        }
    }

    compName = Catalog::getComponentName(ComponentType::UILayoutComponent, true);
    if (compNode[compName]) {
        UILayoutComponent layout = decodeUILayoutComponent(compNode[compName], registry->findComponent<UILayoutComponent>(entity));
        if (!signature.test(registry->getComponentId<UILayoutComponent>())){
            registry->addComponent<UILayoutComponent>(entity, layout);
        }else{
            registry->getComponent<UILayoutComponent>(entity) = layout;
        }
    }

    compName = Catalog::getComponentName(ComponentType::ImageComponent, true);
    if (compNode[compName]) {
        ImageComponent image = decodeImageComponent(compNode[compName], registry->findComponent<ImageComponent>(entity));
        if (!signature.test(registry->getComponentId<ImageComponent>())){
            registry->addComponent<ImageComponent>(entity, image);
        }else{
            registry->getComponent<ImageComponent>(entity) = image;
        }
    }

    compName = Catalog::getComponentName(ComponentType::SpriteComponent, true);
    if (compNode[compName]) {
        SpriteComponent sprite = decodeSpriteComponent(compNode[compName], registry->findComponent<SpriteComponent>(entity));
        if (!signature.test(registry->getComponentId<SpriteComponent>())){
            registry->addComponent<SpriteComponent>(entity, sprite);
        }else{
            registry->getComponent<SpriteComponent>(entity) = sprite;
        }
    }

    compName = Catalog::getComponentName(ComponentType::LightComponent, true);
    if (compNode[compName]) {
        LightComponent light = decodeLightComponent(compNode[compName], registry->findComponent<LightComponent>(entity));
        if (!signature.test(registry->getComponentId<LightComponent>())){
            registry->addComponent<LightComponent>(entity, light);
        }else{
            registry->getComponent<LightComponent>(entity) = light;
        }
    }

    compName = Catalog::getComponentName(ComponentType::CameraComponent, true);
    if (compNode[compName]) {
        CameraComponent camera = decodeCameraComponent(compNode[compName], registry->findComponent<CameraComponent>(entity));
        if (!signature.test(registry->getComponentId<CameraComponent>())){
            registry->addComponent<CameraComponent>(entity, camera);
        }else{
            registry->getComponent<CameraComponent>(entity) = camera;
        }
    }

    compName = Catalog::getComponentName(ComponentType::ScriptComponent, true);
    if (compNode[compName]) {
        ScriptComponent script = decodeScriptComponent(compNode[compName], registry->findComponent<ScriptComponent>(entity));
        if (!signature.test(registry->getComponentId<ScriptComponent>())){
            registry->addComponent<ScriptComponent>(entity, script);
        }else{
            registry->getComponent<ScriptComponent>(entity) = script;
        }
    }
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

Transform Editor::Stream::decodeTransform(const YAML::Node& node, const Transform* oldTransform) {
    Transform transform;

    // Use old values as defaults if provided
    if (oldTransform) {
        transform = *oldTransform;
    }

    if (node["position"]) transform.position = decodeVector3(node["position"]);
    if (node["rotation"]) transform.rotation = decodeQuaternion(node["rotation"]);
    if (node["scale"]) transform.scale = decodeVector3(node["scale"]);
    if (node["worldPosition"]) transform.worldPosition = decodeVector3(node["worldPosition"]);
    if (node["worldRotation"]) transform.worldRotation = decodeQuaternion(node["worldRotation"]);
    if (node["worldScale"]) transform.worldScale = decodeVector3(node["worldScale"]);
    if (node["localMatrix"]) transform.localMatrix = decodeMatrix4(node["localMatrix"]);
    if (node["modelMatrix"]) transform.modelMatrix = decodeMatrix4(node["modelMatrix"]);
    if (node["normalMatrix"]) transform.normalMatrix = decodeMatrix4(node["normalMatrix"]);
    if (node["modelViewProjectionMatrix"]) transform.modelViewProjectionMatrix = decodeMatrix4(node["modelViewProjectionMatrix"]);
    if (node["visible"]) transform.visible = node["visible"].as<bool>();
    //transform.parent = node["parent"].as<Entity>();
    if (node["distanceToCamera"]) transform.distanceToCamera = node["distanceToCamera"].as<float>();
    if (node["billboardRotation"]) transform.billboardRotation = decodeQuaternion(node["billboardRotation"]);
    if (node["billboard"]) transform.billboard = node["billboard"].as<bool>();
    if (node["fakeBillboard"]) transform.fakeBillboard = node["fakeBillboard"].as<bool>();
    if (node["cylindricalBillboard"]) transform.cylindricalBillboard = node["cylindricalBillboard"].as<bool>();
    //transform.needUpdateChildVisibility = node["needUpdateChildVisibility"].as<bool>();
    //transform.needUpdate = node["needUpdate"].as<bool>();

    return transform;
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

MeshComponent Editor::Stream::decodeMeshComponent(const YAML::Node& node, const MeshComponent* oldMesh) {
    MeshComponent mesh;

    // Use old values as defaults if provided
    if (oldMesh) {
        mesh = *oldMesh;
    }

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
    if (node["submeshes"]) {
        auto submeshesNode = node["submeshes"];
        for(unsigned int i = 0; i < submeshesNode.size() && i < MAX_SUBMESHES; i++) {
            mesh.submeshes[i] = decodeSubmesh(submeshesNode[i]);
        }
        mesh.numSubmeshes = submeshesNode.size();
    }

    // Decode bones matrix
    if (node["bonesMatrix"]) {
        auto bonesNode = node["bonesMatrix"];
        for(int i = 0; i < MAX_BONES; i++) {
            mesh.bonesMatrix[i] = decodeMatrix4(bonesNode[i]);
        }
    }

    if (node["normAdjustJoint"]) mesh.normAdjustJoint = node["normAdjustJoint"].as<int>();
    if (node["normAdjustWeight"]) mesh.normAdjustWeight = node["normAdjustWeight"].as<float>();

    // Decode morph weights
    if (node["morphWeights"]) {
        auto morphWeightsNode = node["morphWeights"];
        for(int i = 0; i < MAX_MORPHTARGETS; i++) {
            mesh.morphWeights[i] = morphWeightsNode[i].as<float>();
        }
    }

    if (node["aabb"]) mesh.aabb = decodeAABB(node["aabb"]);
    if (node["verticesAABB"]) mesh.verticesAABB = decodeAABB(node["verticesAABB"]);
    if (node["worldAABB"]) mesh.worldAABB = decodeAABB(node["worldAABB"]);

    if (node["receiveLights"]) mesh.receiveLights = node["receiveLights"].as<bool>();
    if (node["castShadows"]) mesh.castShadows = node["castShadows"].as<bool>();
    if (node["receiveShadows"]) mesh.receiveShadows = node["receiveShadows"].as<bool>();
    if (node["shadowsBillboard"]) mesh.shadowsBillboard = node["shadowsBillboard"].as<bool>();
    if (node["transparent"]) mesh.transparent = node["transparent"].as<bool>();

    if (node["cullingMode"]) mesh.cullingMode = stringToCullingMode(node["cullingMode"].as<std::string>());
    if (node["windingOrder"]) mesh.windingOrder = stringToWindingOrder(node["windingOrder"].as<std::string>());

    //mesh.needUpdateBuffer = node["needUpdateBuffer"].as<bool>();
    //mesh.needReload = node["needReload"].as<bool>();

    return mesh;
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

UIComponent Editor::Stream::decodeUIComponent(const YAML::Node& node, const UIComponent* oldUI) {
    UIComponent ui;

    // Use old values as defaults if provided
    if (oldUI) {
        ui = *oldUI;
    }

    //ui.loaded = node["loaded"].as<bool>();
    //ui.loadCalled = node["loadCalled"].as<bool>();

    if (node["buffer"]) decodeBuffer(ui.buffer, node["buffer"]);
    if (node["indices"]) decodeBuffer(ui.indices, node["indices"]);
    if (node["minBufferCount"]) ui.minBufferCount = node["minBufferCount"].as<unsigned int>();
    if (node["minIndicesCount"]) ui.minIndicesCount = node["minIndicesCount"].as<unsigned int>();

    // ui.render and ui.shader can't be deserialized here
    // ui.shaderProperties = node["shaderProperties"].as<std::string>();
    //ui.slotVSParams = node["slotVSParams"].as<int>();
    //ui.slotFSParams = node["slotFSParams"].as<int>();

    if (node["primitiveType"]) ui.primitiveType = stringToPrimitiveType(node["primitiveType"].as<std::string>());
    if (node["vertexCount"]) ui.vertexCount = node["vertexCount"].as<unsigned int>();

    if (node["aabb"]) ui.aabb = decodeAABB(node["aabb"]);
    if (node["worldAABB"]) ui.worldAABB = decodeAABB(node["worldAABB"]);

    if (node["texture"]) ui.texture = decodeTexture(node["texture"]);
    if (node["color"]) ui.color = decodeVector4(node["color"]);

    if (node["automaticFlipY"]) ui.automaticFlipY = node["automaticFlipY"].as<bool>();
    if (node["flipY"]) ui.flipY = node["flipY"].as<bool>();

    if (node["pointerMoved"]) ui.pointerMoved = node["pointerMoved"].as<bool>();
    if (node["focused"]) ui.focused = node["focused"].as<bool>();

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

UILayoutComponent Editor::Stream::decodeUILayoutComponent(const YAML::Node& node, const UILayoutComponent* oldLayout) {
    UILayoutComponent layout;

    // Use old values as defaults if provided
    if (oldLayout) {
        layout = *oldLayout;
    }

    if (node["width"]) layout.width = node["width"].as<unsigned int>();
    if (node["height"]) layout.height = node["height"].as<unsigned int>();
    if (node["anchorPointLeft"]) layout.anchorPointLeft = node["anchorPointLeft"].as<float>();
    if (node["anchorPointTop"]) layout.anchorPointTop = node["anchorPointTop"].as<float>();
    if (node["anchorPointRight"]) layout.anchorPointRight = node["anchorPointRight"].as<float>();
    if (node["anchorPointBottom"]) layout.anchorPointBottom = node["anchorPointBottom"].as<float>();
    if (node["anchorOffsetLeft"]) layout.anchorOffsetLeft = node["anchorOffsetLeft"].as<int>();
    if (node["anchorOffsetTop"]) layout.anchorOffsetTop = node["anchorOffsetTop"].as<int>();
    if (node["anchorOffsetRight"]) layout.anchorOffsetRight = node["anchorOffsetRight"].as<int>();
    if (node["anchorOffsetBottom"]) layout.anchorOffsetBottom = node["anchorOffsetBottom"].as<int>();
    if (node["positionOffset"]) layout.positionOffset = decodeVector2(node["positionOffset"]);
    if (node["anchorPreset"]) layout.anchorPreset = static_cast<AnchorPreset>(node["anchorPreset"].as<int>());
    if (node["usingAnchors"]) layout.usingAnchors = node["usingAnchors"].as<bool>();
    if (node["panel"]) layout.panel = node["panel"].as<Entity>();
    if (node["containerBoxIndex"]) layout.containerBoxIndex = node["containerBoxIndex"].as<int>();
    if (node["scissor"]) layout.scissor = decodeRect(node["scissor"]);
    if (node["ignoreScissor"]) layout.ignoreScissor = node["ignoreScissor"].as<bool>();
    if (node["ignoreEvents"]) layout.ignoreEvents = node["ignoreEvents"].as<bool>();
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

ImageComponent Editor::Stream::decodeImageComponent(const YAML::Node& node, const ImageComponent* oldImage) {
    ImageComponent image;

    // Use old values as defaults if provided
    if (oldImage) {
        image = *oldImage;
    }

    if (node["patchMarginLeft"]) image.patchMarginLeft = node["patchMarginLeft"].as<int>();
    if (node["patchMarginRight"]) image.patchMarginRight = node["patchMarginRight"].as<int>();
    if (node["patchMarginTop"]) image.patchMarginTop = node["patchMarginTop"].as<int>();
    if (node["patchMarginBottom"]) image.patchMarginBottom = node["patchMarginBottom"].as<int>();
    if (node["textureScaleFactor"]) image.textureScaleFactor = node["textureScaleFactor"].as<float>();
    //image.needUpdatePatches = node["needUpdatePatches"].as<bool>();

    return image;
}

YAML::Node Editor::Stream::encodeSpriteComponent(const SpriteComponent& sprite) {
    YAML::Node node;
    node["width"] = sprite.width;
    node["height"] = sprite.height;
    node["automaticFlipY"] = sprite.automaticFlipY;
    node["flipY"] = sprite.flipY;
    node["textureScaleFactor"] = sprite.textureScaleFactor;
    node["pivotPreset"] = pivotPresetToString(sprite.pivotPreset);
    //node["needUpdateSprite"] = sprite.needUpdateSprite;

    YAML::Node framesNode;
    for (int i = 0; i < MAX_SPRITE_FRAMES; i++) {
        if (sprite.framesRect[i].active) {
            framesNode[i] = encodeSpriteFrameData(sprite.framesRect[i]);
        }
    }
    if (framesNode.size() > 0) {
        node["framesRect"] = framesNode;
    }

    return node;
}

SpriteComponent Editor::Stream::decodeSpriteComponent(const YAML::Node& node, const SpriteComponent* oldSprite) {
    SpriteComponent sprite;

    // Use old values as defaults if provided
    if (oldSprite) {
        sprite = *oldSprite;
    }

    if (node["width"]) sprite.width = node["width"].as<unsigned int>();
    if (node["height"]) sprite.height = node["height"].as<unsigned int>();
    if (node["automaticFlipY"]) sprite.automaticFlipY = node["automaticFlipY"].as<bool>();
    if (node["flipY"]) sprite.flipY = node["flipY"].as<bool>();
    if (node["textureScaleFactor"]) sprite.textureScaleFactor = node["textureScaleFactor"].as<float>();
    if (node["pivotPreset"]) sprite.pivotPreset = stringToPivotPreset(node["pivotPreset"].as<std::string>());
    //if (node["needUpdateSprite"]) sprite.needUpdateSprite = node["needUpdateSprite"].as<bool>();

    if (node["framesRect"]) {
        for (const auto& frameNode : node["framesRect"]) {
            int index = frameNode.first.as<int>();
            if (index >= 0 && index < MAX_SPRITE_FRAMES) {
                sprite.framesRect[index] = decodeSpriteFrameData(frameNode.second);
            }
        }
    }

    return sprite;
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

LightComponent Editor::Stream::decodeLightComponent(const YAML::Node& node, const LightComponent* oldLight) {
    LightComponent light;

    // Use old values as defaults if provided
    if (oldLight) {
        light = *oldLight;
    }

    if (node["type"]) light.type = stringToLightType(node["type"].as<std::string>());
    if (node["direction"]) light.direction = decodeVector3(node["direction"]);
    if (node["worldDirection"]) light.worldDirection = decodeVector3(node["worldDirection"]);
    if (node["color"]) light.color = decodeVector3(node["color"]);
    if (node["range"]) light.range = node["range"].as<float>();
    if (node["intensity"]) light.intensity = node["intensity"].as<float>();
    if (node["innerConeCos"]) light.innerConeCos = node["innerConeCos"].as<float>();
    if (node["outerConeCos"]) light.outerConeCos = node["outerConeCos"].as<float>();
    if (node["shadows"]) light.shadows = node["shadows"].as<bool>();
    if (node["automaticShadowCamera"]) light.automaticShadowCamera = node["automaticShadowCamera"].as<bool>();
    if (node["shadowBias"]) light.shadowBias = node["shadowBias"].as<float>();
    if (node["mapResolution"]) light.mapResolution = node["mapResolution"].as<unsigned int>();
    if (node["shadowCameraNearFar"]) light.shadowCameraNearFar = decodeVector2(node["shadowCameraNearFar"]);
    if (node["numShadowCascades"]) light.numShadowCascades = node["numShadowCascades"].as<unsigned int>();

    return light;
}

YAML::Node Editor::Stream::encodeCameraComponent(const CameraComponent& camera) {
    YAML::Node node;

    node["type"] = cameraTypeToString(camera.type);
    node["target"] = encodeVector3(camera.target);
    node["up"] = encodeVector3(camera.up);
    node["leftClip"] = camera.leftClip;
    node["rightClip"] = camera.rightClip;
    node["bottomClip"] = camera.bottomClip;
    node["topClip"] = camera.topClip;
    node["yfov"] = camera.yfov;
    node["aspect"] = camera.aspect;
    node["nearClip"] = camera.nearClip;
    node["farClip"] = camera.farClip;
    node["renderToTexture"] = camera.renderToTexture;
    node["transparentSort"] = camera.transparentSort;
    node["automatic"] = camera.automatic;

    return node;
}

CameraComponent Editor::Stream::decodeCameraComponent(const YAML::Node& node, const CameraComponent* oldCamera) {
    CameraComponent camera;

    // Use old values as defaults if provided
    if (oldCamera) {
        camera = *oldCamera;
    }

    if (node["type"]) camera.type = stringToCameraType(node["type"].as<std::string>());
    if (node["target"]) camera.target = decodeVector3(node["target"]);
    if (node["up"]) camera.up = decodeVector3(node["up"]);
    if (node["leftClip"]) camera.leftClip = node["leftClip"].as<float>();
    if (node["rightClip"]) camera.rightClip = node["rightClip"].as<float>();
    if (node["bottomClip"]) camera.bottomClip = node["bottomClip"].as<float>();
    if (node["topClip"]) camera.topClip = node["topClip"].as<float>();
    if (node["yfov"]) camera.yfov = node["yfov"].as<float>();
    if (node["aspect"]) camera.aspect = node["aspect"].as<float>();
    if (node["nearClip"]) camera.nearClip = node["nearClip"].as<float>();
    if (node["farClip"]) camera.farClip = node["farClip"].as<float>();
    if (node["renderToTexture"]) camera.renderToTexture = node["renderToTexture"].as<bool>();
    if (node["transparentSort"]) camera.transparentSort = node["transparentSort"].as<bool>();
    if (node["automatic"]) camera.automatic = node["automatic"].as<bool>();

    return camera;
}

YAML::Node Editor::Stream::encodeScriptComponent(const ScriptComponent& script) {
    YAML::Node node;

    if (!script.scripts.empty()) {
        YAML::Node scriptsNode;
        for (const auto& scriptEntry : script.scripts) {
            YAML::Node scriptNode;

            scriptNode["type"] = scriptTypeToString(scriptEntry.type);

            if (!scriptEntry.path.empty())
                scriptNode["path"] = scriptEntry.path;

            if (!scriptEntry.headerPath.empty())
                scriptNode["headerPath"] = scriptEntry.headerPath;

            if (!scriptEntry.className.empty())
                scriptNode["className"] = scriptEntry.className;

            scriptNode["enabled"] = scriptEntry.enabled;

            if (!scriptEntry.properties.empty()) {
                YAML::Node propsNode;
                for (const auto& prop : scriptEntry.properties) {
                    propsNode.push_back(encodeScriptProperty(prop));
                }
                scriptNode["properties"] = propsNode;
            }

            scriptsNode.push_back(scriptNode);
        }
        node["scripts"] = scriptsNode;
    }

    return node;
}

ScriptComponent Editor::Stream::decodeScriptComponent(const YAML::Node& node, const ScriptComponent* oldScript) {
    ScriptComponent script;

    // Use old values as defaults if provided
    if (oldScript) {
        script = *oldScript;
    }

    script.scripts.clear();

    if (!node["scripts"] || !node["scripts"].IsSequence())
        return script;

    for (const auto& scriptNode : node["scripts"]) {
        ScriptEntry entry;

        if (scriptNode["type"]) {
            std::string typeStr = scriptNode["type"].as<std::string>();
            entry.type = stringToScriptType(typeStr);
        }

        if (scriptNode["path"])
            entry.path = scriptNode["path"].as<std::string>();

        if (scriptNode["headerPath"])
            entry.headerPath = scriptNode["headerPath"].as<std::string>();

        if (scriptNode["className"])
            entry.className = scriptNode["className"].as<std::string>();

        if (scriptNode["enabled"])
            entry.enabled = scriptNode["enabled"].as<bool>();

        if (scriptNode["properties"] && scriptNode["properties"].IsSequence()) {
            for (const auto& propNode : scriptNode["properties"]) {
                entry.properties.push_back(decodeScriptProperty(propNode));
            }
        }

        script.scripts.push_back(std::move(entry));
    }

    return script;
}