#include "Stream.h"

#include "Base64.h"
#include "Catalog.h"
#include "Out.h"
#include "util/ProjectUtils.h"

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

std::string Editor::Stream::containerTypeToString(ContainerType type) {
    switch (type) {
        case ContainerType::VERTICAL: return "vertical";
        case ContainerType::HORIZONTAL: return "horizontal";
        case ContainerType::VERTICAL_WRAP: return "vertical_wrap";
        case ContainerType::HORIZONTAL_WRAP: return "horizontal_wrap";
        default: return "vertical";
    }
}

ContainerType Editor::Stream::stringToContainerType(const std::string& str) {
    if (str == "vertical") return ContainerType::VERTICAL;
    if (str == "horizontal") return ContainerType::HORIZONTAL;
    if (str == "vertical_wrap") return ContainerType::VERTICAL_WRAP;
    if (str == "horizontal_wrap") return ContainerType::HORIZONTAL_WRAP;
    return ContainerType::VERTICAL;
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

std::string Editor::Stream::uiEventStateToString(UIEventState state) {
    switch (state) {
        case UIEventState::NOT_SET: return "not_set";
        case UIEventState::ENABLED: return "true";
        case UIEventState::DISABLED: return "false";
        default: return "not_set";
    }
}

UIEventState Editor::Stream::stringToUIEventState(const std::string& str) {
    if (str == "not_set") return UIEventState::NOT_SET;
    if (str == "true") return UIEventState::ENABLED;
    if (str == "false") return UIEventState::DISABLED;
    return UIEventState::NOT_SET; // Default
}

std::string Editor::Stream::cameraTypeToString(CameraType type) {
    switch (type) {
        case CameraType::CAMERA_UI: return "camera_ui";
        case CameraType::CAMERA_ORTHO: return "camera_ortho";
        case CameraType::CAMERA_PERSPECTIVE: return "camera_perspective";
        default: return "camera_perspective";
    }
}

CameraType Editor::Stream::stringToCameraType(const std::string& str) {
    if (str == "camera_ui") return CameraType::CAMERA_UI;
    if (str == "camera_ortho") return CameraType::CAMERA_ORTHO;
    if (str == "camera_perspective") return CameraType::CAMERA_PERSPECTIVE;
    return CameraType::CAMERA_PERSPECTIVE;
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
    if (texture.empty() || texture.isFramebuffer())
        return node;

    const bool isCube = texture.isCubeMap() || (texture.getNumFaces() == 6);
    node["textureType"] = isCube ? "CUBE" : "2D";

    // Source selection: prefer filesystem paths; fall back to id only when there is no path.
    if (!isCube) {
        const std::string path = texture.getPath(0);
        if (!path.empty()) {
            node["source"] = "path";
            node["path"] = path;
        } else {
            node["source"] = "id";
            node["id"] = texture.getId();
        }
    } else {
        bool hasAnyNonZeroFace = false;
        bool hasAnyEmptyFace = false;
        for (int f = 1; f < 6; f++) {
            if (!texture.getPath((size_t)f).empty()) hasAnyNonZeroFace = true;
            if (texture.getPath((size_t)f).empty()) hasAnyEmptyFace = true;
        }

        const bool singleFileCube = !texture.getPath(0).empty() && !hasAnyNonZeroFace;
        if (singleFileCube) {
            node["cubeMode"] = "single";
            node["source"] = "path";
            node["path"] = texture.getPath(0);
        } else {
            node["cubeMode"] = "faces";
            node["source"] = "paths";
            YAML::Node pathsNode;
            pathsNode.SetStyle(YAML::EmitterStyle::Flow);
            for (int f = 0; f < 6; f++) {
                pathsNode.push_back(texture.getPath((size_t)f));
            }
            node["paths"] = pathsNode;
            node["hasEmptyFaces"] = hasAnyEmptyFace;
        }
    }

    node["minFilter"] = textureFilterToString(texture.getMinFilter());
    node["magFilter"] = textureFilterToString(texture.getMagFilter());
    node["wrapU"] = textureWrapToString(texture.getWrapU());
    node["wrapV"] = textureWrapToString(texture.getWrapV());
    node["releaseDataAfterLoad"] = texture.isReleaseDataAfterLoad();
    return node;
}

Texture Editor::Stream::decodeTexture(const YAML::Node& node) {
    Texture texture;
    if (node.IsMap()) { // Check if node has data
        const std::string textureType = node["textureType"] ? node["textureType"].as<std::string>() : std::string();
        const std::string source = node["source"] ? node["source"].as<std::string>() : std::string();

        if (textureType == "CUBE") {
            const std::string cubeMode = node["cubeMode"] ? node["cubeMode"].as<std::string>() : std::string();
            if (cubeMode == "single") {
                if (node["path"]) {
                    texture.setCubeMap(node["path"].as<std::string>());
                }
            } else {
                if (node["paths"] && node["paths"].IsSequence() && node["paths"].size() == 6) {
                    std::array<std::string, 6> paths;
                    for (size_t f = 0; f < 6; f++) {
                        paths[f] = node["paths"][f].as<std::string>();
                    }

                    // Set per-face paths individually (supports incomplete cubemaps).
                    for (size_t f = 0; f < 6; f++) {
                        texture.setCubePath(f, paths[f]);
                    }
                }
            }
        } else if (textureType == "2D") {
            if (source == "path") {
                if (node["path"]) {
                    texture.setPath(node["path"].as<std::string>());
                }
            } else if (source == "id") {
                if (node["id"]) {
                    texture.setId(node["id"].as<std::string>());
                }
            }
        }

        if (node["minFilter"]) texture.setMinFilter(stringToTextureFilter(node["minFilter"].as<std::string>()));
        if (node["magFilter"]) texture.setMagFilter(stringToTextureFilter(node["magFilter"].as<std::string>()));
        if (node["wrapU"]) texture.setWrapU(stringToTextureWrap(node["wrapU"].as<std::string>()));
        if (node["wrapV"]) texture.setWrapV(stringToTextureWrap(node["wrapV"].as<std::string>()));

        //if (node["isFramebuffer"] && node["isFramebuffer"].as<bool>()) {
        //    texture.setIsFramebuffer(true);
        //}

        //if (node["width"] && node["height"]) {
        //    texture.setWidth(node["width"].as<int>());
        //    texture.setHeight(node["height"].as<int>());
        //}

        if (node["releaseDataAfterLoad"]) texture.setReleaseDataAfterLoad(node["releaseDataAfterLoad"].as<bool>());
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

Submesh Editor::Stream::decodeSubmesh(const YAML::Node& node, const Submesh* oldSubmesh) {
    Submesh submesh;
    if (oldSubmesh) {
        submesh = *oldSubmesh;
    }

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
            sceneNode["opened"] = sceneProject.opened;
            sceneNode["showAllJoints"]        = sceneProject.displaySettings.showAllJoints;
            sceneNode["hideAllBodies"]        = sceneProject.displaySettings.hideAllBodies;
            sceneNode["hideCameraView"]       = sceneProject.displaySettings.hideCameraView;
            sceneNode["hideLightIcons"]       = sceneProject.displaySettings.hideLightIcons;
            sceneNode["hideContainerGuides"]  = sceneProject.displaySettings.hideContainerGuides;
            sceneNode["hideGrid"]             = sceneProject.displaySettings.hideGrid;
            sceneNode["hideSelectionOutline"] = sceneProject.displaySettings.hideSelectionOutline;
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
    bool anyOpened = false;
    if (node["scenes"]) {
        for (const auto& sceneNode : node["scenes"]) {
            if (sceneNode["filepath"]) {
                fs::path scenePath = sceneNode["filepath"].as<std::string>();
                if (scenePath.is_relative()) {
                    scenePath = project->getProjectPath() / scenePath;
                }
                bool opened = sceneNode["opened"] ? sceneNode["opened"].as<bool>() : false;
                if (opened) anyOpened = true;
                if (fs::exists(scenePath)) {
                    project->loadScene(scenePath, opened);
                    // Restore display settings into the just-loaded scene
                    auto& scenes = project->getScenes();
                    if (!scenes.empty()) {
                        SceneDisplaySettings& ds = scenes.back().displaySettings;
                        if (sceneNode["showAllJoints"])        ds.showAllJoints        = sceneNode["showAllJoints"].as<bool>();
                        if (sceneNode["hideAllBodies"])        ds.hideAllBodies        = sceneNode["hideAllBodies"].as<bool>();
                        if (sceneNode["hideCameraView"])       ds.hideCameraView       = sceneNode["hideCameraView"].as<bool>();
                        if (sceneNode["hideLightIcons"])       ds.hideLightIcons       = sceneNode["hideLightIcons"].as<bool>();
                        if (sceneNode["hideContainerGuides"])  ds.hideContainerGuides  = sceneNode["hideContainerGuides"].as<bool>();
                        if (sceneNode["hideGrid"])             ds.hideGrid             = sceneNode["hideGrid"].as<bool>();
                        if (sceneNode["hideSelectionOutline"]) ds.hideSelectionOutline = sceneNode["hideSelectionOutline"].as<bool>();
                    }
                }
            }
        }
    }
    if (!anyOpened && project->getScenes().size() > 0){
        project->getScenes()[0].opened = true;
    }
}

YAML::Node Editor::Stream::encodeSceneProject(const Project* project, const SceneProject* sceneProject) {
    YAML::Node root;
    root["id"] = sceneProject->id;
    root["name"] = sceneProject->name;
    root["scene"] = encodeScene(sceneProject->scene);
    root["sceneType"] = sceneTypeToString(sceneProject->sceneType);
    root["mainCamera"] = sceneProject->mainCamera;

    if (!sceneProject->childScenes.empty()) {
        YAML::Node childScenesNode;
        for (const auto& childSceneId : sceneProject->childScenes) {
            childScenesNode.push_back(childSceneId);
        }
        root["childScenes"] = childScenesNode;
    }

    if (!sceneProject->cppScripts.empty()) {
        YAML::Node scriptsNode;
        for (const auto& script : sceneProject->cppScripts) {
            YAML::Node scriptNode;
            scriptNode["path"] = script.path.generic_string();
            if (!script.headerPath.empty()) {
                scriptNode["headerPath"] = script.headerPath.generic_string();
            }
            if (!script.className.empty()) {
                scriptNode["className"] = script.className;
            }

            if (!script.properties.empty()) {
                YAML::Node propertiesNode;
                for (const auto& prop : script.properties) {
                    YAML::Node propNode;
                    propNode["name"] = prop.name;
                    propNode["isPtr"] = prop.isPtr;
                    if (!prop.ptrTypeName.empty()) {
                        propNode["ptrTypeName"] = prop.ptrTypeName;
                    }
                    propertiesNode.push_back(propNode);
                }
                scriptNode["properties"] = propertiesNode;
            }

            scriptsNode.push_back(scriptNode);
        }
        root["cppScripts"] = scriptsNode;
    }

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

void Editor::Stream::decodeSceneProject(SceneProject* sceneProject, const YAML::Node& node, bool loadScene) {
    if (node["id"]) sceneProject->id = node["id"].as<uint32_t>();
    if (node["name"]) sceneProject->name = node["name"].as<std::string>();
    if (loadScene){
        if (node["scene"]) sceneProject->scene = decodeScene(sceneProject->scene, node["scene"]);
    }
    if (node["sceneType"]) sceneProject->sceneType = stringToSceneType(node["sceneType"].as<std::string>());
    if (node["mainCamera"]) sceneProject->mainCamera = node["mainCamera"].as<Entity>();

    sceneProject->childScenes.clear();
    if (node["childScenes"]) {
        for (const auto& childSceneNode : node["childScenes"]) {
            sceneProject->childScenes.push_back(childSceneNode.as<uint32_t>());
        }
    }

    sceneProject->cppScripts.clear();
    if (node["cppScripts"]) {
        for (const auto& scriptNode : node["cppScripts"]) {
            SceneScriptSource script;

            if (scriptNode["path"]) {
                script.path = fs::path(scriptNode["path"].as<std::string>());
            }
            if (scriptNode["headerPath"]) {
                script.headerPath = fs::path(scriptNode["headerPath"].as<std::string>());
            }
            if (scriptNode["className"]) {
                script.className = scriptNode["className"].as<std::string>();
            }
            if (scriptNode["properties"]) {
                for (const auto& propNode : scriptNode["properties"]) {
                    ScriptPropertyInfo prop;
                    if (propNode["name"]) {
                        prop.name = propNode["name"].as<std::string>();
                    }
                    if (propNode["isPtr"]) {
                        prop.isPtr = propNode["isPtr"].as<bool>();
                    }
                    if (propNode["ptrTypeName"]) {
                        prop.ptrTypeName = propNode["ptrTypeName"].as<std::string>();
                    }
                    script.properties.push_back(prop);
                }
            }

            sceneProject->cppScripts.push_back(script);
        }
    } else if (node["cppScriptPaths"]) {
        for (const auto& pathNode : node["cppScriptPaths"]) {
            SceneScriptSource script;
            script.path = fs::path(pathNode.as<std::string>());
            sceneProject->cppScripts.push_back(script);
        }
    }
}

void Editor::Stream::decodeSceneProjectEntities(Project* project, SceneProject* sceneProject, const YAML::Node& node){
    sceneProject->entities.clear();
    sceneProject->selectedEntities.clear();

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
    sceneNode["enableUIEvents"] = uiEventStateToString(scene->getEnableUIEvents());

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
        scene->setEnableUIEvents(stringToUIEventState(node["enableUIEvents"].as<std::string>()));
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
        YAML::Node components = encodeComponents(entity, registry, signature);

        if ((components.IsMap() && components.size() > 0)){
            entityNode["components"] = components;
        }

    }else{
        entityNode["type"] = "Entity";

        entityNode["entity"] = entity;
        entityNode["name"] = registry->getEntityName(entity);

        Signature signature = registry->getSignature(entity);
        YAML::Node components = encodeComponents(entity, registry, signature);

        if ((components.IsMap() && components.size() > 0)){
            entityNode["components"] = components;
        }
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

    if (signature.test(registry->getComponentId<ButtonComponent>())) {
        ButtonComponent button = registry->getComponent<ButtonComponent>(entity);
        compNode[Catalog::getComponentName(ComponentType::ButtonComponent, true)] = encodeButtonComponent(button);
    }

    if (signature.test(registry->getComponentId<UILayoutComponent>())) {
        UILayoutComponent layout = registry->getComponent<UILayoutComponent>(entity);
        compNode[Catalog::getComponentName(ComponentType::UILayoutComponent, true)] = encodeUILayoutComponent(layout);
    }

    if (signature.test(registry->getComponentId<UIContainerComponent>())) {
        UIContainerComponent container = registry->getComponent<UIContainerComponent>(entity);
        compNode[Catalog::getComponentName(ComponentType::UIContainerComponent, true)] = encodeUIContainerComponent(container);
    }

    if (signature.test(registry->getComponentId<TextComponent>())) {
        TextComponent text = registry->getComponent<TextComponent>(entity);
        compNode[Catalog::getComponentName(ComponentType::TextComponent, true)] = encodeTextComponent(text);
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

    if (signature.test(registry->getComponentId<SkyComponent>())) {
        SkyComponent sky = registry->getComponent<SkyComponent>(entity);
        compNode[Catalog::getComponentName(ComponentType::SkyComponent, true)] = encodeSkyComponent(sky);
    }

    if (signature.test(registry->getComponentId<Body2DComponent>())) {
        Body2DComponent body = registry->getComponent<Body2DComponent>(entity);
        compNode[Catalog::getComponentName(ComponentType::Body2DComponent, true)] = encodeBody2DComponent(body);
    }

    if (signature.test(registry->getComponentId<Body3DComponent>())) {
        Body3DComponent body = registry->getComponent<Body3DComponent>(entity);
        compNode[Catalog::getComponentName(ComponentType::Body3DComponent, true)] = encodeBody3DComponent(body);
    }

    if (signature.test(registry->getComponentId<Joint2DComponent>())) {
        Joint2DComponent joint = registry->getComponent<Joint2DComponent>(entity);
        compNode[Catalog::getComponentName(ComponentType::Joint2DComponent, true)] = encodeJoint2DComponent(joint);
    }

    if (signature.test(registry->getComponentId<Joint3DComponent>())) {
        Joint3DComponent joint = registry->getComponent<Joint3DComponent>(entity);
        compNode[Catalog::getComponentName(ComponentType::Joint3DComponent, true)] = encodeJoint3DComponent(joint);
    }

    return compNode;
}

void Editor::Stream::decodeComponents(Entity entity, Entity parent, EntityRegistry* registry, const YAML::Node& compNode){
    std::string compName;

    Signature signature = registry->getSignature(entity);

    compName = Catalog::getComponentName(ComponentType::Transform, true);
    if (compNode[compName]) {
        Transform* existing = registry->findComponent<Transform>(entity);
        Transform transform = decodeTransform(compNode[compName], existing);
        transform.parent = parent;
        if (!signature.test(registry->getComponentId<Transform>())){
            registry->addComponent<Transform>(entity, transform);
        }else{
            int flags = Catalog::getChangedUpdateFlags(ComponentType::Transform, existing, &transform);
            registry->addEntityChild(transform.parent, entity, false);
            registry->getComponent<Transform>(entity) = transform;
            Catalog::updateEntity(registry, entity, flags);
        }
    }

    compName = Catalog::getComponentName(ComponentType::MeshComponent, true);
    if (compNode[compName]) {
        MeshComponent* existing = registry->findComponent<MeshComponent>(entity);
        MeshComponent mesh = decodeMeshComponent(compNode[compName], existing);
        if (!signature.test(registry->getComponentId<MeshComponent>())){
            registry->addComponent<MeshComponent>(entity, mesh);
        }else{
            int flags = Catalog::getChangedUpdateFlags(ComponentType::MeshComponent, existing, &mesh);
            registry->getComponent<MeshComponent>(entity) = mesh;
            Catalog::updateEntity(registry, entity, flags);
        }
    }

    compName = Catalog::getComponentName(ComponentType::UIComponent, true);
    if (compNode[compName]) {
        UIComponent* existing = registry->findComponent<UIComponent>(entity);
        UIComponent ui = decodeUIComponent(compNode[compName], existing);
        if (!signature.test(registry->getComponentId<UIComponent>())){
            registry->addComponent<UIComponent>(entity, ui);
        }else{
            int flags = Catalog::getChangedUpdateFlags(ComponentType::UIComponent, existing, &ui);
            registry->getComponent<UIComponent>(entity) = ui;
            Catalog::updateEntity(registry, entity, flags);
        }
    }

    compName = Catalog::getComponentName(ComponentType::ButtonComponent, true);
    if (compNode[compName]) {
        ButtonComponent* existing = registry->findComponent<ButtonComponent>(entity);
        ButtonComponent button = decodeButtonComponent(compNode[compName], existing);
        if (!signature.test(registry->getComponentId<ButtonComponent>())){
            registry->addComponent<ButtonComponent>(entity, button);
        }else{
            int flags = Catalog::getChangedUpdateFlags(ComponentType::ButtonComponent, existing, &button);
            registry->getComponent<ButtonComponent>(entity) = button;
            Catalog::updateEntity(registry, entity, flags);
        }
    }

    compName = Catalog::getComponentName(ComponentType::UILayoutComponent, true);
    if (compNode[compName]) {
        UILayoutComponent* existing = registry->findComponent<UILayoutComponent>(entity);
        UILayoutComponent layout = decodeUILayoutComponent(compNode[compName], existing);
        if (!signature.test(registry->getComponentId<UILayoutComponent>())){
            registry->addComponent<UILayoutComponent>(entity, layout);
        }else{
            int flags = Catalog::getChangedUpdateFlags(ComponentType::UILayoutComponent, existing, &layout);
            registry->getComponent<UILayoutComponent>(entity) = layout;
            Catalog::updateEntity(registry, entity, flags);
        }
    }

    compName = Catalog::getComponentName(ComponentType::UIContainerComponent, true);
    if (compNode[compName]) {
        UIContainerComponent* existing = registry->findComponent<UIContainerComponent>(entity);
        UIContainerComponent container = decodeUIContainerComponent(compNode[compName], existing);
        if (!signature.test(registry->getComponentId<UIContainerComponent>())){
            registry->addComponent<UIContainerComponent>(entity, container);
        }else{
            int flags = Catalog::getChangedUpdateFlags(ComponentType::UIContainerComponent, existing, &container);
            registry->getComponent<UIContainerComponent>(entity) = container;
            Catalog::updateEntity(registry, entity, flags);
        }
    }

    compName = Catalog::getComponentName(ComponentType::TextComponent, true);
    if (compNode[compName]) {
        TextComponent* existing = registry->findComponent<TextComponent>(entity);
        TextComponent text = decodeTextComponent(compNode[compName], existing);
        if (!signature.test(registry->getComponentId<TextComponent>())){
            registry->addComponent<TextComponent>(entity, text);
        }else{
            int flags = Catalog::getChangedUpdateFlags(ComponentType::TextComponent, existing, &text);
            registry->getComponent<TextComponent>(entity) = text;
            Catalog::updateEntity(registry, entity, flags);
        }
    }

    compName = Catalog::getComponentName(ComponentType::ImageComponent, true);
    if (compNode[compName]) {
        ImageComponent* existing = registry->findComponent<ImageComponent>(entity);
        ImageComponent image = decodeImageComponent(compNode[compName], existing);
        if (!signature.test(registry->getComponentId<ImageComponent>())){
            registry->addComponent<ImageComponent>(entity, image);
        }else{
            int flags = Catalog::getChangedUpdateFlags(ComponentType::ImageComponent, existing, &image);
            registry->getComponent<ImageComponent>(entity) = image;
            Catalog::updateEntity(registry, entity, flags);
        }
    }

    compName = Catalog::getComponentName(ComponentType::SpriteComponent, true);
    if (compNode[compName]) {
        SpriteComponent* existing = registry->findComponent<SpriteComponent>(entity);
        SpriteComponent sprite = decodeSpriteComponent(compNode[compName], existing);
        if (!signature.test(registry->getComponentId<SpriteComponent>())){
            registry->addComponent<SpriteComponent>(entity, sprite);
        }else{
            int flags = Catalog::getChangedUpdateFlags(ComponentType::SpriteComponent, existing, &sprite);
            registry->getComponent<SpriteComponent>(entity) = sprite;
            Catalog::updateEntity(registry, entity, flags);
        }
    }

    compName = Catalog::getComponentName(ComponentType::LightComponent, true);
    if (compNode[compName]) {
        LightComponent* existing = registry->findComponent<LightComponent>(entity);
        LightComponent light = decodeLightComponent(compNode[compName], existing);
        if (!signature.test(registry->getComponentId<LightComponent>())){
            registry->addComponent<LightComponent>(entity, light);
        }else{
            int flags = Catalog::getChangedUpdateFlags(ComponentType::LightComponent, existing, &light);
            registry->getComponent<LightComponent>(entity) = light;
            Catalog::updateEntity(registry, entity, flags);
        }
    }

    compName = Catalog::getComponentName(ComponentType::CameraComponent, true);
    if (compNode[compName]) {
        CameraComponent* existing = registry->findComponent<CameraComponent>(entity);
        CameraComponent camera = decodeCameraComponent(compNode[compName], existing);
        if (!signature.test(registry->getComponentId<CameraComponent>())){
            registry->addComponent<CameraComponent>(entity, camera);
        }else{
            int flags = Catalog::getChangedUpdateFlags(ComponentType::CameraComponent, existing, &camera);
            registry->getComponent<CameraComponent>(entity) = camera;
            Catalog::updateEntity(registry, entity, flags);
        }
    }

    compName = Catalog::getComponentName(ComponentType::ScriptComponent, true);
    if (compNode[compName]) {
        ScriptComponent* existing = registry->findComponent<ScriptComponent>(entity);
        ScriptComponent script = decodeScriptComponent(compNode[compName], existing);
        if (!signature.test(registry->getComponentId<ScriptComponent>())){
            registry->addComponent<ScriptComponent>(entity, script);
        }else{
            int flags = Catalog::getChangedUpdateFlags(ComponentType::ScriptComponent, existing, &script);
            registry->getComponent<ScriptComponent>(entity) = script;
            Catalog::updateEntity(registry, entity, flags);
        }
    }

    compName = Catalog::getComponentName(ComponentType::SkyComponent, true);
    if (compNode[compName]) {
        SkyComponent* existing = registry->findComponent<SkyComponent>(entity);
        SkyComponent sky = decodeSkyComponent(compNode[compName], existing);
        if (!signature.test(registry->getComponentId<SkyComponent>())){
            registry->addComponent<SkyComponent>(entity, sky);
        }else{
            int flags = Catalog::getChangedUpdateFlags(ComponentType::SkyComponent, existing, &sky);
            registry->getComponent<SkyComponent>(entity) = sky;
            Catalog::updateEntity(registry, entity, flags);
        }
    }

    compName = Catalog::getComponentName(ComponentType::Body2DComponent, true);
    if (compNode[compName]) {
        Body2DComponent* existing = registry->findComponent<Body2DComponent>(entity);
        Body2DComponent body = decodeBody2DComponent(compNode[compName], existing);
        if (!signature.test(registry->getComponentId<Body2DComponent>())){
            registry->addComponent<Body2DComponent>(entity, body);
        }else{
            int flags = Catalog::getChangedUpdateFlags(ComponentType::Body2DComponent, existing, &body);
            registry->getComponent<Body2DComponent>(entity) = body;
            Catalog::updateEntity(registry, entity, flags);
        }
    }

    compName = Catalog::getComponentName(ComponentType::Body3DComponent, true);
    if (compNode[compName]) {
        Body3DComponent* existing = registry->findComponent<Body3DComponent>(entity);
        Body3DComponent body = decodeBody3DComponent(compNode[compName], existing);
        if (!signature.test(registry->getComponentId<Body3DComponent>())){
            registry->addComponent<Body3DComponent>(entity, body);
        }else{
            int flags = Catalog::getChangedUpdateFlags(ComponentType::Body3DComponent, existing, &body);
            registry->getComponent<Body3DComponent>(entity) = body;
            Catalog::updateEntity(registry, entity, flags);
        }
    }

    compName = Catalog::getComponentName(ComponentType::Joint2DComponent, true);
    if (compNode[compName]) {
        Joint2DComponent* existing = registry->findComponent<Joint2DComponent>(entity);
        Joint2DComponent joint = decodeJoint2DComponent(compNode[compName], existing);
        if (!signature.test(registry->getComponentId<Joint2DComponent>())){
            registry->addComponent<Joint2DComponent>(entity, joint);
        }else{
            int flags = Catalog::getChangedUpdateFlags(ComponentType::Joint2DComponent, existing, &joint);
            registry->getComponent<Joint2DComponent>(entity) = joint;
            Catalog::updateEntity(registry, entity, flags);
        }
    }

    compName = Catalog::getComponentName(ComponentType::Joint3DComponent, true);
    if (compNode[compName]) {
        Joint3DComponent* existing = registry->findComponent<Joint3DComponent>(entity);
        Joint3DComponent joint = decodeJoint3DComponent(compNode[compName], existing);
        if (!signature.test(registry->getComponentId<Joint3DComponent>())){
            registry->addComponent<Joint3DComponent>(entity, joint);
        }else{
            int flags = Catalog::getChangedUpdateFlags(ComponentType::Joint3DComponent, existing, &joint);
            registry->getComponent<Joint3DComponent>(entity) = joint;
            Catalog::updateEntity(registry, entity, flags);
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
            mesh.submeshes[i] = decodeSubmesh(submeshesNode[i], &mesh.submeshes[i]);
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

YAML::Node Editor::Stream::encodeButtonComponent(const ButtonComponent& button) {
    YAML::Node node;
    node["label"] = button.label;
    node["textureNormal"] = encodeTexture(button.textureNormal);
    node["texturePressed"] = encodeTexture(button.texturePressed);
    node["textureDisabled"] = encodeTexture(button.textureDisabled);
    node["colorNormal"] = encodeVector4(button.colorNormal);
    node["colorPressed"] = encodeVector4(button.colorPressed);
    node["colorDisabled"] = encodeVector4(button.colorDisabled);
    node["disabled"] = button.disabled;
    return node;
}

ButtonComponent Editor::Stream::decodeButtonComponent(const YAML::Node& node, const ButtonComponent* oldButton) {
    ButtonComponent button;

    if (oldButton) {
        button = *oldButton;
    }

    if (node["label"]) button.label = node["label"].as<Entity>();
    if (node["textureNormal"]) button.textureNormal = decodeTexture(node["textureNormal"]);
    if (node["texturePressed"]) button.texturePressed = decodeTexture(node["texturePressed"]);
    if (node["textureDisabled"]) button.textureDisabled = decodeTexture(node["textureDisabled"]);
    if (node["colorNormal"]) button.colorNormal = decodeVector4(node["colorNormal"]);
    if (node["colorPressed"]) button.colorPressed = decodeVector4(node["colorPressed"]);
    if (node["colorDisabled"]) button.colorDisabled = decodeVector4(node["colorDisabled"]);
    if (node["disabled"]) button.disabled = node["disabled"].as<bool>();

    return button;
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
    //node["needUpdateAnchorOffsets"] = layout.needUpdateAnchorOffsets;

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
    //layout.needUpdateAnchorOffsets = node["needUpdateAnchorOffsets"].as<bool>();

    return layout;
}

YAML::Node Editor::Stream::encodeUIContainerComponent(const UIContainerComponent& container) {
    YAML::Node node;
    node["type"] = containerTypeToString(container.type);
    node["useAllWrapSpace"] = container.useAllWrapSpace;
    node["wrapCellWidth"] = container.wrapCellWidth;
    node["wrapCellHeight"] = container.wrapCellHeight;
    return node;
}

UIContainerComponent Editor::Stream::decodeUIContainerComponent(const YAML::Node& node, const UIContainerComponent* oldContainer) {
    UIContainerComponent container;

    if (oldContainer) {
        container = *oldContainer;
    }

    if (node["type"]) container.type = stringToContainerType(node["type"].as<std::string>());
    if (node["useAllWrapSpace"]) container.useAllWrapSpace = node["useAllWrapSpace"].as<bool>();
    if (node["fillAvailableSpace"]) container.useAllWrapSpace = node["fillAvailableSpace"].as<bool>();
    if (node["wrapCellWidth"]) container.wrapCellWidth = node["wrapCellWidth"].as<unsigned int>();
    if (node["wrapCellHeight"]) container.wrapCellHeight = node["wrapCellHeight"].as<unsigned int>();

    return container;
}

YAML::Node Editor::Stream::encodeTextComponent(const TextComponent& text) {
    YAML::Node node;

    node["font"] = text.font;
    node["text"] = text.text;
    node["fontSize"] = text.fontSize;
    node["multiline"] = text.multiline;
    node["maxTextSize"] = text.maxTextSize;
    node["fixedWidth"] = text.fixedWidth;
    node["fixedHeight"] = text.fixedHeight;
    node["pivotBaseline"] = text.pivotBaseline;
    node["pivotCentered"] = text.pivotCentered;

    return node;
}

TextComponent Editor::Stream::decodeTextComponent(const YAML::Node& node, const TextComponent* oldText) {
    TextComponent text;

    if (oldText) {
        text = *oldText;
    }

    if (node["font"]) text.font = node["font"].as<std::string>();
    if (node["text"]) text.text = node["text"].as<std::string>();
    if (node["fontSize"]) text.fontSize = node["fontSize"].as<unsigned int>();
    if (node["multiline"]) text.multiline = node["multiline"].as<bool>();
    if (node["maxTextSize"]) text.maxTextSize = node["maxTextSize"].as<unsigned int>();
    if (node["fixedWidth"]) text.fixedWidth = node["fixedWidth"].as<bool>();
    if (node["fixedHeight"]) text.fixedHeight = node["fixedHeight"].as<bool>();
    if (node["pivotBaseline"]) text.pivotBaseline = node["pivotBaseline"].as<bool>();
    if (node["pivotCentered"]) text.pivotCentered = node["pivotCentered"].as<bool>();

    return text;
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
        const YAML::Node& framesNode = node["framesRect"];

        if (framesNode.IsSequence()) {
            for (std::size_t i = 0; i < framesNode.size() && i < MAX_SPRITE_FRAMES; i++) {
                if (!framesNode[i] || framesNode[i].IsNull()) {
                    continue;
                }

                sprite.framesRect[i] = decodeSpriteFrameData(framesNode[i]);
            }
        } else {
            for (const auto& frameNode : framesNode) {
                int index = frameNode.first.as<int>();
                if (index >= 0 && index < MAX_SPRITE_FRAMES) {
                    sprite.framesRect[index] = decodeSpriteFrameData(frameNode.second);
                }
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
    node["useTarget"] = camera.useTarget;
    node["autoResize"] = camera.autoResize;

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
    if (node["useTarget"]) camera.useTarget = node["useTarget"].as<bool>();
    if (node["autoResize"]) camera.autoResize = node["autoResize"].as<bool>();

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

YAML::Node Editor::Stream::encodeSkyComponent(const SkyComponent& sky) {
    YAML::Node node;

    const bool useDefaultTexture = (sky.texture.getId() == "editor:resources:default_sky");
    node["useDefaultTexture"] = useDefaultTexture;
    node["texture"] = encodeTexture(sky.texture);
    node["color"] = encodeVector4(sky.color);
    node["rotation"] = sky.rotation;

    return node;
}

SkyComponent Editor::Stream::decodeSkyComponent(const YAML::Node& node, const SkyComponent* oldSky) {
    SkyComponent sky;

    if (oldSky) {
        sky = *oldSky;
    }

    const bool useDefaultTexture = node["useDefaultTexture"] ? node["useDefaultTexture"].as<bool>() : false;
    if (useDefaultTexture) {
        ProjectUtils::setDefaultSkyTexture(sky.texture);
    }

    if (node["texture"]) {
        if (useDefaultTexture) {
            const YAML::Node& texNode = node["texture"];
            if (texNode["minFilter"]) sky.texture.setMinFilter(stringToTextureFilter(texNode["minFilter"].as<std::string>()));
            if (texNode["magFilter"]) sky.texture.setMagFilter(stringToTextureFilter(texNode["magFilter"].as<std::string>()));
            if (texNode["wrapU"]) sky.texture.setWrapU(stringToTextureWrap(texNode["wrapU"].as<std::string>()));
            if (texNode["wrapV"]) sky.texture.setWrapV(stringToTextureWrap(texNode["wrapV"].as<std::string>()));
            if (texNode["releaseDataAfterLoad"]) sky.texture.setReleaseDataAfterLoad(texNode["releaseDataAfterLoad"].as<bool>());
        } else {
            sky.texture = decodeTexture(node["texture"]);
        }
    }
    if (node["color"]) sky.color = decodeVector4(node["color"]);
    if (node["rotation"]) sky.rotation = node["rotation"].as<float>();

    return sky;
}

// ==============================
// Body/Joint type string converters
// ==============================

std::string Editor::Stream::bodyTypeToString(BodyType type) {
    switch (type) {
        case BodyType::STATIC: return "static";
        case BodyType::KINEMATIC: return "kinematic";
        case BodyType::DYNAMIC: return "dynamic";
        default: return "static";
    }
}

BodyType Editor::Stream::stringToBodyType(const std::string& str) {
    if (str == "kinematic") return BodyType::KINEMATIC;
    if (str == "dynamic") return BodyType::DYNAMIC;
    return BodyType::STATIC;
}

std::string Editor::Stream::shape2DTypeToString(Shape2DType type) {
    switch (type) {
        case Shape2DType::POLYGON: return "polygon";
        case Shape2DType::CIRCLE: return "circle";
        case Shape2DType::CAPSULE: return "capsule";
        case Shape2DType::SEGMENT: return "segment";
        case Shape2DType::CHAIN: return "chain";
        default: return "polygon";
    }
}

Shape2DType Editor::Stream::stringToShape2DType(const std::string& str) {
    if (str == "circle") return Shape2DType::CIRCLE;
    if (str == "capsule") return Shape2DType::CAPSULE;
    if (str == "segment") return Shape2DType::SEGMENT;
    if (str == "chain") return Shape2DType::CHAIN;
    return Shape2DType::POLYGON;
}

std::string Editor::Stream::shape3DTypeToString(Shape3DType type) {
    switch (type) {
        case Shape3DType::SPHERE: return "sphere";
        case Shape3DType::BOX: return "box";
        case Shape3DType::CAPSULE: return "capsule";
        case Shape3DType::TAPERED_CAPSULE: return "tapered_capsule";
        case Shape3DType::CYLINDER: return "cylinder";
        case Shape3DType::CONVEX_HULL: return "convex_hull";
        case Shape3DType::MESH: return "mesh";
        case Shape3DType::HEIGHTFIELD: return "heightfield";
        default: return "sphere";
    }
}

Shape3DType Editor::Stream::stringToShape3DType(const std::string& str) {
    if (str == "box") return Shape3DType::BOX;
    if (str == "capsule") return Shape3DType::CAPSULE;
    if (str == "tapered_capsule") return Shape3DType::TAPERED_CAPSULE;
    if (str == "cylinder") return Shape3DType::CYLINDER;
    if (str == "convex_hull") return Shape3DType::CONVEX_HULL;
    if (str == "mesh") return Shape3DType::MESH;
    if (str == "heightfield") return Shape3DType::HEIGHTFIELD;
    return Shape3DType::SPHERE;
}

std::string Editor::Stream::joint2DTypeToString(Joint2DType type) {
    switch (type) {
        case Joint2DType::DISTANCE: return "distance";
        case Joint2DType::REVOLUTE: return "revolute";
        case Joint2DType::PRISMATIC: return "prismatic";
        case Joint2DType::MOUSE: return "mouse";
        case Joint2DType::WHEEL: return "wheel";
        case Joint2DType::WELD: return "weld";
        case Joint2DType::MOTOR: return "motor";
        default: return "distance";
    }
}

Joint2DType Editor::Stream::stringToJoint2DType(const std::string& str) {
    if (str == "revolute") return Joint2DType::REVOLUTE;
    if (str == "prismatic") return Joint2DType::PRISMATIC;
    if (str == "mouse") return Joint2DType::MOUSE;
    if (str == "wheel") return Joint2DType::WHEEL;
    if (str == "weld") return Joint2DType::WELD;
    if (str == "motor") return Joint2DType::MOTOR;
    return Joint2DType::DISTANCE;
}

std::string Editor::Stream::joint3DTypeToString(Joint3DType type) {
    switch (type) {
        case Joint3DType::FIXED: return "fixed";
        case Joint3DType::DISTANCE: return "distance";
        case Joint3DType::POINT: return "point";
        case Joint3DType::HINGE: return "hinge";
        case Joint3DType::CONE: return "cone";
        case Joint3DType::PRISMATIC: return "prismatic";
        case Joint3DType::SWINGTWIST: return "swingtwist";
        case Joint3DType::SIXDOF: return "sixdof";
        case Joint3DType::PATH: return "path";
        case Joint3DType::GEAR: return "gear";
        case Joint3DType::RACKANDPINON: return "rackandpinion";
        case Joint3DType::PULLEY: return "pulley";
        default: return "fixed";
    }
}

Joint3DType Editor::Stream::stringToJoint3DType(const std::string& str) {
    if (str == "distance") return Joint3DType::DISTANCE;
    if (str == "point") return Joint3DType::POINT;
    if (str == "hinge") return Joint3DType::HINGE;
    if (str == "cone") return Joint3DType::CONE;
    if (str == "prismatic") return Joint3DType::PRISMATIC;
    if (str == "swingtwist") return Joint3DType::SWINGTWIST;
    if (str == "sixdof") return Joint3DType::SIXDOF;
    if (str == "path") return Joint3DType::PATH;
    if (str == "gear") return Joint3DType::GEAR;
    if (str == "rackandpinion") return Joint3DType::RACKANDPINON;
    if (str == "pulley") return Joint3DType::PULLEY;
    return Joint3DType::FIXED;
}

// ==============================
// Body2DComponent encode/decode
// ==============================

YAML::Node Editor::Stream::encodeBody2DComponent(const Body2DComponent& body) {
    YAML::Node node;

    node["type"] = bodyTypeToString(body.type);
    node["numShapes"] = static_cast<unsigned int>(body.numShapes);

    YAML::Node shapesNode;
    for (size_t i = 0; i < body.numShapes; i++) {
        YAML::Node shapeNode;
        shapeNode["type"] = shape2DTypeToString(body.shapes[i].type);
        shapeNode["pointA"] = encodeVector2(body.shapes[i].pointA);
        shapeNode["pointB"] = encodeVector2(body.shapes[i].pointB);
        shapeNode["radius"] = body.shapes[i].radius;
        shapeNode["loop"] = body.shapes[i].loop;

        shapeNode["density"] = body.shapes[i].density;
        shapeNode["friction"] = body.shapes[i].friction;
        shapeNode["restitution"] = body.shapes[i].restitution;
        shapeNode["enableHitEvents"] = body.shapes[i].enableHitEvents;
        shapeNode["contactEvents"] = body.shapes[i].contactEvents;
        shapeNode["preSolveEvents"] = body.shapes[i].preSolveEvents;
        shapeNode["sensorEvents"] = body.shapes[i].sensorEvents;
        shapeNode["categoryBits"] = body.shapes[i].categoryBits;
        shapeNode["maskBits"] = body.shapes[i].maskBits;
        shapeNode["groupIndex"] = body.shapes[i].groupIndex;

        YAML::Node verticesNode;
        for (size_t j = 0; j < body.shapes[i].verticesCount; j++) {
            verticesNode.push_back(encodeVector2(body.shapes[i].vertices[j]));
        }
        shapeNode["vertices"] = verticesNode;

        shapesNode.push_back(shapeNode);
    }
    node["shapes"] = shapesNode;

    return node;
}

Body2DComponent Editor::Stream::decodeBody2DComponent(const YAML::Node& node, const Body2DComponent* oldBody) {
    Body2DComponent body;

    if (oldBody) {
        body = *oldBody;
    }

    if (node["type"]) body.type = stringToBodyType(node["type"].as<std::string>());
    if (node["numShapes"]) body.numShapes = node["numShapes"].as<unsigned int>();

    if (node["shapes"]) {
        size_t count = std::min(node["shapes"].size(), (size_t)MAX_SHAPES);
        for (size_t i = 0; i < count; i++) {
            if (node["shapes"][i]["type"]) {
                body.shapes[i].type = stringToShape2DType(node["shapes"][i]["type"].as<std::string>());
            }
            if (node["shapes"][i]["pointA"]) {
                body.shapes[i].pointA = decodeVector2(node["shapes"][i]["pointA"]);
            }
            if (node["shapes"][i]["pointB"]) {
                body.shapes[i].pointB = decodeVector2(node["shapes"][i]["pointB"]);
            }
            if (node["shapes"][i]["radius"]) body.shapes[i].radius = node["shapes"][i]["radius"].as<float>();
            if (node["shapes"][i]["loop"]) body.shapes[i].loop = node["shapes"][i]["loop"].as<bool>();

            if (node["shapes"][i]["density"]) body.shapes[i].density = node["shapes"][i]["density"].as<float>();
            if (node["shapes"][i]["friction"]) body.shapes[i].friction = node["shapes"][i]["friction"].as<float>();
            if (node["shapes"][i]["restitution"]) body.shapes[i].restitution = node["shapes"][i]["restitution"].as<float>();
            if (node["shapes"][i]["enableHitEvents"]) body.shapes[i].enableHitEvents = node["shapes"][i]["enableHitEvents"].as<bool>();
            if (node["shapes"][i]["contactEvents"]) body.shapes[i].contactEvents = node["shapes"][i]["contactEvents"].as<bool>();
            if (node["shapes"][i]["preSolveEvents"]) body.shapes[i].preSolveEvents = node["shapes"][i]["preSolveEvents"].as<bool>();
            if (node["shapes"][i]["sensorEvents"]) body.shapes[i].sensorEvents = node["shapes"][i]["sensorEvents"].as<bool>();
            if (node["shapes"][i]["categoryBits"]) body.shapes[i].categoryBits = node["shapes"][i]["categoryBits"].as<uint16_t>();
            if (node["shapes"][i]["maskBits"]) body.shapes[i].maskBits = node["shapes"][i]["maskBits"].as<uint16_t>();
            if (node["shapes"][i]["groupIndex"]) body.shapes[i].groupIndex = node["shapes"][i]["groupIndex"].as<int16_t>();

            if (node["shapes"][i]["vertices"]) {
                size_t vcount = std::min(node["shapes"][i]["vertices"].size(), (size_t)MAX_SHAPE_POINTS_2D);
                body.shapes[i].verticesCount = (uint8_t)vcount;
                for (size_t j = 0; j < vcount; j++) {
                    body.shapes[i].vertices[j] = decodeVector2(node["shapes"][i]["vertices"][j]);
                }
            }

            body.shapes[i].shape = b2_nullShapeId;
            body.shapes[i].chain = b2_nullChainId;
        }
    }

    if (oldBody && b2Body_IsValid(oldBody->body)) {
        body.needReloadBody = true;
        body.needUpdateShapes = true;
    }

    return body;
}

// ==============================
// Body3DComponent encode/decode
// ==============================

YAML::Node Editor::Stream::encodeBody3DComponent(const Body3DComponent& body) {
    YAML::Node node;

    node["type"] = bodyTypeToString(body.type);
    node["numShapes"] = static_cast<unsigned int>(body.numShapes);

    YAML::Node shapesNode;
    for (size_t i = 0; i < body.numShapes; i++) {
        YAML::Node shapeNode;
        shapeNode["type"] = shape3DTypeToString(body.shapes[i].type);
        shapeNode["position"] = encodeVector3(body.shapes[i].position);
        shapeNode["rotation"] = encodeQuaternion(body.shapes[i].rotation);
        shapeNode["width"] = body.shapes[i].width;
        shapeNode["height"] = body.shapes[i].height;
        shapeNode["depth"] = body.shapes[i].depth;
        shapeNode["radius"] = body.shapes[i].radius;
        shapeNode["halfHeight"] = body.shapes[i].halfHeight;
        shapeNode["topRadius"] = body.shapes[i].topRadius;
        shapeNode["bottomRadius"] = body.shapes[i].bottomRadius;
        shapeNode["density"] = body.shapes[i].density;

        switch (body.shapes[i].source) {
            case Shape3DSource::RAW_VERTICES: shapeNode["source"] = "raw_vertices"; break;
            case Shape3DSource::RAW_MESH: shapeNode["source"] = "raw_mesh"; break;
            case Shape3DSource::ENTITY_MESH: shapeNode["source"] = "entity_mesh"; break;
            case Shape3DSource::ENTITY_HEIGHTFIELD: shapeNode["source"] = "entity_heightfield"; break;
            default: shapeNode["source"] = "none"; break;
        }

        shapeNode["sourceEntity"] = body.shapes[i].sourceEntity;
        shapeNode["samplesSize"] = body.shapes[i].samplesSize;

        YAML::Node verticesNode;
        for (size_t j = 0; j < body.shapes[i].numVertices; j++) {
            verticesNode.push_back(encodeVector3(body.shapes[i].vertices[j]));
        }
        shapeNode["vertices"] = verticesNode;

        YAML::Node indicesNode;
        for (size_t j = 0; j < body.shapes[i].numIndices; j++) {
            indicesNode.push_back(body.shapes[i].indices[j]);
        }
        shapeNode["indices"] = indicesNode;

        shapesNode.push_back(shapeNode);
    }
    node["shapes"] = shapesNode;

    return node;
}

Body3DComponent Editor::Stream::decodeBody3DComponent(const YAML::Node& node, const Body3DComponent* oldBody) {
    Body3DComponent body;
    constexpr float kMaxSingleShapeLocalOffset = 50.0f;

    if (oldBody) {
        body = *oldBody;
    }

    if (node["type"]) body.type = stringToBodyType(node["type"].as<std::string>());
    if (node["numShapes"]) body.numShapes = node["numShapes"].as<unsigned int>();

    if (node["shapes"]) {
        size_t count = std::min(node["shapes"].size(), (size_t)MAX_SHAPES);
        for (size_t i = 0; i < count; i++) {
            if (node["shapes"][i]["type"]) {
                body.shapes[i].type = stringToShape3DType(node["shapes"][i]["type"].as<std::string>());
            }
            if (node["shapes"][i]["position"]) {
                body.shapes[i].position = decodeVector3(node["shapes"][i]["position"]);

                if (body.numShapes == 1 && body.shapes[i].position.length() > kMaxSingleShapeLocalOffset) {
                    Log::warn("Body3D shape local position is too large (%.2f). Resetting to [0, 0, 0] to avoid unstable physics.", body.shapes[i].position.length());
                    body.shapes[i].position = Vector3::ZERO;
                }
            }
            if (node["shapes"][i]["rotation"]) {
                body.shapes[i].rotation = decodeQuaternion(node["shapes"][i]["rotation"]);
            }
            if (node["shapes"][i]["width"]) body.shapes[i].width = node["shapes"][i]["width"].as<float>();
            if (node["shapes"][i]["height"]) body.shapes[i].height = node["shapes"][i]["height"].as<float>();
            if (node["shapes"][i]["depth"]) body.shapes[i].depth = node["shapes"][i]["depth"].as<float>();
            if (node["shapes"][i]["radius"]) body.shapes[i].radius = node["shapes"][i]["radius"].as<float>();
            if (node["shapes"][i]["halfHeight"]) body.shapes[i].halfHeight = node["shapes"][i]["halfHeight"].as<float>();
            if (node["shapes"][i]["topRadius"]) body.shapes[i].topRadius = node["shapes"][i]["topRadius"].as<float>();
            if (node["shapes"][i]["bottomRadius"]) body.shapes[i].bottomRadius = node["shapes"][i]["bottomRadius"].as<float>();
            if (node["shapes"][i]["density"]) body.shapes[i].density = node["shapes"][i]["density"].as<float>();

            if (node["shapes"][i]["source"]) {
                std::string source = node["shapes"][i]["source"].as<std::string>();
                if (source == "raw_vertices") body.shapes[i].source = Shape3DSource::RAW_VERTICES;
                else if (source == "raw_mesh") body.shapes[i].source = Shape3DSource::RAW_MESH;
                else if (source == "entity_mesh") body.shapes[i].source = Shape3DSource::ENTITY_MESH;
                else if (source == "entity_heightfield") body.shapes[i].source = Shape3DSource::ENTITY_HEIGHTFIELD;
                else body.shapes[i].source = Shape3DSource::NONE;
            }

            if (body.shapes[i].source == Shape3DSource::NONE) {
                if (body.shapes[i].type == Shape3DType::CONVEX_HULL) {
                    body.shapes[i].source = Shape3DSource::ENTITY_MESH;
                } else if (body.shapes[i].type == Shape3DType::MESH) {
                    body.shapes[i].source = Shape3DSource::ENTITY_MESH;
                } else if (body.shapes[i].type == Shape3DType::HEIGHTFIELD) {
                    body.shapes[i].source = Shape3DSource::ENTITY_HEIGHTFIELD;
                }
            }

            if (node["shapes"][i]["sourceEntity"]) body.shapes[i].sourceEntity = node["shapes"][i]["sourceEntity"].as<Entity>();

            if (node["shapes"][i]["samplesSize"]) body.shapes[i].samplesSize = node["shapes"][i]["samplesSize"].as<unsigned int>();

            if (node["shapes"][i]["vertices"]) {
                size_t vcount = std::min(node["shapes"][i]["vertices"].size(), (size_t)MAX_SHAPE_VERTICES_3D);
                body.shapes[i].numVertices = (uint16_t)vcount;
                for (size_t j = 0; j < vcount; j++) {
                    body.shapes[i].vertices[j] = decodeVector3(node["shapes"][i]["vertices"][j]);
                }
            }

            if (node["shapes"][i]["indices"]) {
                size_t icount = std::min(node["shapes"][i]["indices"].size(), (size_t)MAX_SHAPE_INDICES_3D);
                body.shapes[i].numIndices = (uint16_t)icount;
                for (size_t j = 0; j < icount; j++) {
                    body.shapes[i].indices[j] = node["shapes"][i]["indices"][j].as<uint16_t>();
                }
            }

            body.shapes[i].shape = NULL;
        }
    }

    if (oldBody && !oldBody->body.IsInvalid()) {
        body.needReloadBody = true;
        body.needUpdateShapes = true;
    }

    return body;
}

// ==============================
// Joint2DComponent encode/decode
// ==============================

YAML::Node Editor::Stream::encodeJoint2DComponent(const Joint2DComponent& joint) {
    YAML::Node node;

    node["type"] = joint2DTypeToString(joint.type);
    node["bodyA"] = joint.bodyA;
    node["bodyB"] = joint.bodyB;
    node["anchorA"] = encodeVector2(joint.anchorA);
    node["anchorB"] = encodeVector2(joint.anchorB);
    node["axis"] = encodeVector2(joint.axis);
    node["target"] = encodeVector2(joint.target);
    node["autoAnchors"] = joint.autoAnchors;
    node["rope"] = joint.rope;

    return node;
}

Joint2DComponent Editor::Stream::decodeJoint2DComponent(const YAML::Node& node, const Joint2DComponent* oldJoint) {
    Joint2DComponent joint;

    if (oldJoint) {
        joint = *oldJoint;
    }

    if (node["type"]) joint.type = stringToJoint2DType(node["type"].as<std::string>());
    if (node["bodyA"]) joint.bodyA = node["bodyA"].as<Entity>();
    if (node["bodyB"]) joint.bodyB = node["bodyB"].as<Entity>();
    if (node["anchorA"]) joint.anchorA = decodeVector2(node["anchorA"]);
    if (node["anchorB"]) joint.anchorB = decodeVector2(node["anchorB"]);
    if (node["axis"]) joint.axis = decodeVector2(node["axis"]);
    if (node["target"]) joint.target = decodeVector2(node["target"]);
    if (node["autoAnchors"]) joint.autoAnchors = node["autoAnchors"].as<bool>();
    if (node["rope"]) joint.rope = node["rope"].as<bool>();

    if (oldJoint && b2Joint_IsValid(oldJoint->joint)) {
        joint.needUpdateJoint = true;
    }

    return joint;
}

// ==============================
// Joint3DComponent encode/decode
// ==============================

YAML::Node Editor::Stream::encodeJoint3DComponent(const Joint3DComponent& joint) {
    YAML::Node node;

    node["type"] = joint3DTypeToString(joint.type);
    node["bodyA"] = joint.bodyA;
    node["bodyB"] = joint.bodyB;
    node["anchorA"] = encodeVector3(joint.anchorA);
    node["anchorB"] = encodeVector3(joint.anchorB);
    node["anchor"] = encodeVector3(joint.anchor);
    node["axis"] = encodeVector3(joint.axis);
    node["normal"] = encodeVector3(joint.normal);
    node["twistAxis"] = encodeVector3(joint.twistAxis);
    node["planeAxis"] = encodeVector3(joint.planeAxis);
    node["axisX"] = encodeVector3(joint.axisX);
    node["axisY"] = encodeVector3(joint.axisY);
    node["limitsMin"] = joint.limitsMin;
    node["limitsMax"] = joint.limitsMax;
    node["normalHalfConeAngle"] = joint.normalHalfConeAngle;
    node["planeHalfConeAngle"] = joint.planeHalfConeAngle;
    node["twistMinAngle"] = joint.twistMinAngle;
    node["twistMaxAngle"] = joint.twistMaxAngle;
    node["fixedPointA"] = encodeVector3(joint.fixedPointA);
    node["fixedPointB"] = encodeVector3(joint.fixedPointB);
    node["hingeA"] = joint.hingeA;
    node["hingeB"] = joint.hingeB;
    node["hinge"] = joint.hinge;
    node["slider"] = joint.slider;
    node["numTeethGearA"] = joint.numTeethGearA;
    node["numTeethGearB"] = joint.numTeethGearB;
    node["numTeethRack"] = joint.numTeethRack;
    node["numTeethGear"] = joint.numTeethGear;
    node["rackLength"] = joint.rackLength;
    YAML::Node pathPointsNode;
    for (const Vector3& point : joint.pathPoints){
        pathPointsNode.push_back(encodeVector3(point));
    }
    node["pathPoints"] = pathPointsNode;
    node["pathPosition"] = encodeVector3(joint.pathPosition);
    node["isLooping"] = joint.isLooping;
    node["autoAnchors"] = joint.autoAnchors;

    return node;
}

Joint3DComponent Editor::Stream::decodeJoint3DComponent(const YAML::Node& node, const Joint3DComponent* oldJoint) {
    Joint3DComponent joint;

    if (oldJoint) {
        joint = *oldJoint;
    }

    if (node["type"]) joint.type = stringToJoint3DType(node["type"].as<std::string>());
    if (node["bodyA"]) joint.bodyA = node["bodyA"].as<Entity>();
    if (node["bodyB"]) joint.bodyB = node["bodyB"].as<Entity>();
    if (node["anchorA"]) joint.anchorA = decodeVector3(node["anchorA"]);
    if (node["anchorB"]) joint.anchorB = decodeVector3(node["anchorB"]);
    if (node["anchor"]) joint.anchor = decodeVector3(node["anchor"]);
    if (node["axis"]) joint.axis = decodeVector3(node["axis"]);
    if (node["normal"]) joint.normal = decodeVector3(node["normal"]);
    if (node["twistAxis"]) joint.twistAxis = decodeVector3(node["twistAxis"]);
    if (node["planeAxis"]) joint.planeAxis = decodeVector3(node["planeAxis"]);
    if (node["axisX"]) joint.axisX = decodeVector3(node["axisX"]);
    if (node["axisY"]) joint.axisY = decodeVector3(node["axisY"]);
    if (node["limitsMin"]) joint.limitsMin = node["limitsMin"].as<float>();
    if (node["limitsMax"]) joint.limitsMax = node["limitsMax"].as<float>();
    if (node["normalHalfConeAngle"]) joint.normalHalfConeAngle = node["normalHalfConeAngle"].as<float>();
    if (node["planeHalfConeAngle"]) joint.planeHalfConeAngle = node["planeHalfConeAngle"].as<float>();
    if (node["twistMinAngle"]) joint.twistMinAngle = node["twistMinAngle"].as<float>();
    if (node["twistMaxAngle"]) joint.twistMaxAngle = node["twistMaxAngle"].as<float>();
    if (node["fixedPointA"]) joint.fixedPointA = decodeVector3(node["fixedPointA"]);
    if (node["fixedPointB"]) joint.fixedPointB = decodeVector3(node["fixedPointB"]);
    if (node["hingeA"]) joint.hingeA = node["hingeA"].as<Entity>();
    if (node["hingeB"]) joint.hingeB = node["hingeB"].as<Entity>();
    if (node["hinge"]) joint.hinge = node["hinge"].as<Entity>();
    if (node["slider"]) joint.slider = node["slider"].as<Entity>();
    if (node["numTeethGearA"]) joint.numTeethGearA = node["numTeethGearA"].as<int>();
    if (node["numTeethGearB"]) joint.numTeethGearB = node["numTeethGearB"].as<int>();
    if (node["numTeethRack"]) joint.numTeethRack = node["numTeethRack"].as<int>();
    if (node["numTeethGear"]) joint.numTeethGear = node["numTeethGear"].as<int>();
    if (node["rackLength"]) joint.rackLength = node["rackLength"].as<int>();
    if (node["pathPoints"]){
        joint.pathPoints.clear();
        for (const YAML::Node& pointNode : node["pathPoints"]){
            joint.pathPoints.push_back(decodeVector3(pointNode));
        }
    }
    if (node["pathPosition"]) joint.pathPosition = decodeVector3(node["pathPosition"]);
    if (node["isLooping"]) joint.isLooping = node["isLooping"].as<bool>();
    if (node["autoAnchors"]) joint.autoAnchors = node["autoAnchors"].as<bool>();

    if (oldJoint && oldJoint->joint) {
        joint.needUpdateJoint = true;
    }

    return joint;
}
