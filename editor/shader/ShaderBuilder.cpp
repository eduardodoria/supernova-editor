#include "ShaderBuilder.h"

#include <cstring>
#include <cstdint>

using namespace Supernova;

Editor::ShaderBuilder::ShaderBuilder(){
}

Editor::ShaderBuilder::~ShaderBuilder(){
}

// Mapping functions implementation with camelCase
ShaderVertexType Editor::ShaderBuilder::mapVertexType(supershader::attribute_type_t type) {
    using namespace supershader;
    switch(type) {
        case FLOAT:  return ShaderVertexType::FLOAT;
        case FLOAT2: return ShaderVertexType::FLOAT2;
        case FLOAT3: return ShaderVertexType::FLOAT3;
        case FLOAT4: return ShaderVertexType::FLOAT4;
        case INT:    return ShaderVertexType::INT;
        case INT2:   return ShaderVertexType::INT2;
        case INT3:   return ShaderVertexType::INT3;
        case INT4:   return ShaderVertexType::INT4;
        default:     return ShaderVertexType::FLOAT;
    }
}

ShaderUniformType Editor::ShaderBuilder::mapUniformType(supershader::uniform_type_t type) {
    using namespace supershader;
    switch(type) {
        case uniform_type_t::FLOAT:  return ShaderUniformType::FLOAT;
        case uniform_type_t::FLOAT2: return ShaderUniformType::FLOAT2;
        case uniform_type_t::FLOAT3: return ShaderUniformType::FLOAT3;
        case uniform_type_t::FLOAT4: return ShaderUniformType::FLOAT4;
        case uniform_type_t::INT:    return ShaderUniformType::INT;
        case uniform_type_t::INT2:   return ShaderUniformType::INT2;
        case uniform_type_t::INT3:   return ShaderUniformType::INT3;
        case uniform_type_t::INT4:   return ShaderUniformType::INT4;
        case uniform_type_t::MAT3:   return ShaderUniformType::MAT3;
        case uniform_type_t::MAT4:   return ShaderUniformType::MAT4;
        default:                     return ShaderUniformType::FLOAT;
    }
}

TextureType Editor::ShaderBuilder::mapTextureType(supershader::texture_type_t type) {
    using namespace supershader;
    switch(type) {
        case texture_type_t::TEXTURE_2D:    return TextureType::TEXTURE_2D;
        case texture_type_t::TEXTURE_3D:    return TextureType::TEXTURE_3D;
        case texture_type_t::TEXTURE_CUBE:  return TextureType::TEXTURE_CUBE;
        case texture_type_t::TEXTURE_ARRAY: return TextureType::TEXTURE_ARRAY;
        default:                            return TextureType::TEXTURE_2D;
    }
}

TextureSamplerType Editor::ShaderBuilder::mapSamplerType(supershader::texture_samplertype_t type) {
    using namespace supershader;
    switch(type) {
        case texture_samplertype_t::FLOAT: return TextureSamplerType::FLOAT;
        case texture_samplertype_t::SINT:  return TextureSamplerType::SINT;
        case texture_samplertype_t::UINT:  return TextureSamplerType::UINT;
        case texture_samplertype_t::DEPTH: return TextureSamplerType::DEPTH;
        default:                           return TextureSamplerType::FLOAT;
    }
}

SamplerType Editor::ShaderBuilder::mapSamplerFilterType(supershader::sampler_type_t type) {
    using namespace supershader;
    switch(type) {
        case sampler_type_t::FILTERING:   return SamplerType::FILTERING;
        case sampler_type_t::COMPARISON:  return SamplerType::COMPARISON;
        default:                          return SamplerType::FILTERING;
    }
}

ShaderStorageBufferType Editor::ShaderBuilder::mapStorageType(supershader::storage_buffer_type_t type) {
    using namespace supershader;
    switch(type) {
        case storage_buffer_type_t::STRUCT: return ShaderStorageBufferType::STRUCT;
        default:                            return ShaderStorageBufferType::STRUCT;
    }
}

ShaderStageType Editor::ShaderBuilder::mapStageType(supershader::stage_type_t type) {
    using namespace supershader;
    switch(type) {
        case STAGE_VERTEX:   return ShaderStageType::VERTEX;
        case STAGE_FRAGMENT: return ShaderStageType::FRAGMENT;
        default:             return ShaderStageType::VERTEX;
    }
}

ShaderLang Editor::ShaderBuilder::mapLang(supershader::lang_type_t lang) {
    using namespace supershader;
    switch(lang) {
        case LANG_GLSL: return ShaderLang::GLSL;
        case LANG_HLSL: return ShaderLang::HLSL;
        case LANG_MSL:  return ShaderLang::MSL;
        default:        return ShaderLang::GLSL;
    }
}

// Implementation of convertToShaderData
void Editor::ShaderBuilder::convertToShaderData(
const std::vector<supershader::spirvcross_t>& spirvcrossvec,
    const std::vector<supershader::input_t>& inputs,
    const supershader::args_t& args) {

    shaderData.lang = mapLang(args.lang);
    shaderData.version = args.version;
    shaderData.es = args.es;

    for (const auto& cross : spirvcrossvec) {
        ShaderStage stage;
        stage.type = mapStageType(cross.stage_type);
        stage.name = args.output_basename;  // Using output basename for stage name
        stage.source = cross.source;

        // Counter variables for slot assignments
        unsigned int texCount = 0, samplerCount = 0, ubCount = 0, sbCount = 0, pairCount = 0;

        // Attributes
        for (const auto& attr : cross.inputs) {
            ShaderAttr sa;
            sa.name = attr.name;
            sa.semanticName = attr.semantic_name;
            sa.semanticIndex = attr.semantic_index;
            sa.location = attr.location;
            sa.type = mapVertexType(attr.type);
            stage.attributes.push_back(sa);
        }

        // Uniform Blocks
        for (const auto& ub : cross.uniform_blocks) {
            ShaderUniformBlock sub;
            sub.name = ub.name;
            sub.instName = ub.inst_name;
            sub.set = ub.set;
            sub.binding = ub.binding;
            sub.slot = ubCount++;
            sub.sizeBytes = ub.size_bytes;
            sub.flattened = ub.flattened;

            for (const auto& u : ub.uniforms) {
                ShaderUniform su;
                su.name = ub.inst_name + "." + u.name;  // Qualified name
                su.type = mapUniformType(u.type);
                su.arrayCount = u.array_count;
                su.offset = u.offset;
                sub.uniforms.push_back(su);
            }
            stage.uniformblocks.push_back(sub);
        }

        // Storage Buffers
        for (const auto& sb : cross.storage_buffers) {
            ShaderStorageBuffer ssb;
            ssb.name = sb.name;
            ssb.instName = sb.inst_name;
            ssb.set = sb.set;
            ssb.binding = sb.binding;
            ssb.slot = sbCount++;
            ssb.sizeBytes = sb.size_bytes;
            ssb.readonly = sb.readonly;
            ssb.type = mapStorageType(sb.type);
            stage.storagebuffers.push_back(ssb);
        }

        // Textures
        for (const auto& tex : cross.textures) {
            ShaderTexture st;
            st.name = tex.name;
            st.set = tex.set;
            st.binding = tex.binding;
            st.slot = texCount++;
            st.type = mapTextureType(tex.type);
            st.samplerType = mapSamplerType(tex.sampler_type);
            stage.textures.push_back(st);
        }

        // Samplers
        for (const auto& s : cross.samplers) {
            ShaderSampler ss;
            ss.name = s.name;
            ss.set = s.set;
            ss.binding = s.binding;
            ss.slot = samplerCount++;
            ss.type = mapSamplerFilterType(s.type);
            stage.samplers.push_back(ss);
        }

        // Texture-Sampler Pairs
        for (const auto& tsp : cross.texture_sampler_pairs) {
            ShaderTextureSamplerPair pair;
            pair.name = tsp.name;
            pair.textureName = tsp.texture_name;
            pair.samplerName = tsp.sampler_name;
            pair.slot = pairCount++;
            stage.textureSamplerPairs.push_back(pair);
        }

        shaderData.stages.push_back(stage);
    }
}

void Editor::ShaderBuilder::addMeshPropertyDefinitions(std::vector<supershader::define_t> defs, uint32_t prop) {
    if (prop & (1 << 0))  defs.push_back({"MATERIAL_UNLIT", "1"});            // 'Ult'
    if (prop & (1 << 1))  defs.push_back({"HAS_UV_SET1", "1"});               // 'Uv1'
    if (prop & (1 << 2))  defs.push_back({"HAS_UV_SET2", "1"});               // 'Uv2'
    if (prop & (1 << 3))  defs.push_back({"USE_PUNCTUAL", "1"});              // 'Puc'
    if (prop & (1 << 4))  defs.push_back({"USE_SHADOWS", "1"});               // 'Shw'
    if (prop & (1 << 5))  defs.push_back({"USE_SHADOWS_PCF", "1"});           // 'Pcf'
    if (prop & (1 << 6))  defs.push_back({"HAS_NORMALS", "1"});               // 'Nor'
    if (prop & (1 << 7))  defs.push_back({"HAS_NORMAL_MAP", "1"});            // 'Nmp'
    if (prop & (1 << 8))  defs.push_back({"HAS_TANGENTS", "1"});              // 'Tan'
    if (prop & (1 << 9))  defs.push_back({"HAS_VERTEX_COLOR_VEC3", "1"});     // 'Vc3'
    if (prop & (1 << 10)) defs.push_back({"HAS_VERTEX_COLOR_VEC4", "1"});     // 'Vc4'
    if (prop & (1 << 11)) defs.push_back({"HAS_TEXTURERECT", "1"});           // 'Txr'
    if (prop & (1 << 12)) defs.push_back({"HAS_FOG", "1"});                   // 'Fog'
    if (prop & (1 << 13)) defs.push_back({"HAS_SKINNING", "1"});              // 'Ski'
    if (prop & (1 << 14)) defs.push_back({"HAS_MORPHTARGET", "1"});           // 'Mta'
    if (prop & (1 << 15)) defs.push_back({"HAS_MORPHNORMAL", "1"});           // 'Mnr'
    if (prop & (1 << 16)) defs.push_back({"HAS_MORPHTANGENT", "1"});          // 'Mtg'
    if (prop & (1 << 17)) defs.push_back({"HAS_TERRAIN", "1"});               // 'Ter'
    if (prop & (1 << 18)) defs.push_back({"HAS_INSTANCING", "1"});            // 'Ist'
}

void Editor::ShaderBuilder::addDepthMeshPropertyDefinitions(std::vector<supershader::define_t> defs, uint32_t prop) {
    if (prop & (1 << 0))  defs.push_back({"HAS_TEXTURE", "1"});       // 'Tex'
    if (prop & (1 << 1))  defs.push_back({"HAS_SKINNING", "1"});      // 'Ski'
    if (prop & (1 << 2))  defs.push_back({"HAS_MORPHTARGET", "1"});   // 'Mta'
    if (prop & (1 << 3))  defs.push_back({"HAS_MORPHNORMAL", "1"});   // 'Mnr'
    if (prop & (1 << 4))  defs.push_back({"HAS_MORPHTANGENT", "1"});  // 'Mtg'
    if (prop & (1 << 5))  defs.push_back({"HAS_TERRAIN", "1"});       // 'Ter'
    if (prop & (1 << 6))  defs.push_back({"HAS_INSTANCING", "1"});    // 'Ist'
}

void Editor::ShaderBuilder::addUIPropertyDefinitions(std::vector<supershader::define_t> defs, uint32_t prop) {
    if (prop & (1 << 0))  defs.push_back({"HAS_TEXTURE", "1"});              // 'Tex'
    if (prop & (1 << 1))  defs.push_back({"HAS_FONTATLAS_TEXTURE", "1"});    // 'Ftx'
    if (prop & (1 << 2))  defs.push_back({"HAS_VERTEX_COLOR_VEC3", "1"});    // 'Vc3'
    if (prop & (1 << 3))  defs.push_back({"HAS_VERTEX_COLOR_VEC4", "1"});    // 'Vc4'
}

void Editor::ShaderBuilder::addPointsPropertyDefinitions(std::vector<supershader::define_t> defs, uint32_t prop) {
    if (prop & (1 << 0))  defs.push_back({"HAS_TEXTURE", "1"});              // 'Tex'
    if (prop & (1 << 1))  defs.push_back({"HAS_VERTEX_COLOR_VEC3", "1"});    // 'Vc3'
    if (prop & (1 << 2))  defs.push_back({"HAS_VERTEX_COLOR_VEC4", "1"});    // 'Vc4'
    if (prop & (1 << 3))  defs.push_back({"HAS_TEXTURERECT", "1"});          // 'Txr'
}

void Editor::ShaderBuilder::addLinesPropertyDefinitions(std::vector<supershader::define_t> defs, uint32_t prop) {
    if (prop & (1 << 0))  defs.push_back({"HAS_VERTEX_COLOR_VEC3", "1"});    // 'Vc3'
    if (prop & (1 << 1))  defs.push_back({"HAS_VERTEX_COLOR_VEC4", "1"});    // 'Vc4'
}

void Editor::ShaderBuilder::buildShader(ShaderType shaderType, uint32_t properties){
    std::vector<supershader::input_t> inputs;
    supershader::args_t args = supershader::initialize_args();
    args.isValid = true;
    args.useBuffers = true;
    args.fileBuffers = Editor::shaderMap;
    args.lang = supershader::LANG_GLSL;
    args.version = 410;

    if (shaderType == ShaderType::MESH){
        args.vert_file = "mesh.vert";
        args.frag_file = "mesh.frag";
        addMeshPropertyDefinitions(args.defines, properties);
    }else if (shaderType == ShaderType::DEPTH){
        args.vert_file = "depth.vert";
        args.frag_file = "depth.frag";
        addDepthMeshPropertyDefinitions(args.defines, properties);
    }else if (shaderType == ShaderType::UI){
        args.vert_file = "ui.vert";
        args.frag_file = "ui.frag";
        addUIPropertyDefinitions(args.defines, properties);
    }else if (shaderType == ShaderType::POINTS){
        args.vert_file = "points.vert";
        args.frag_file = "points.frag";
        addPointsPropertyDefinitions(args.defines, properties);
    }else if (shaderType == ShaderType::LINES){
        args.vert_file = "lines.vert";
        args.frag_file = "lines.frag";
        addLinesPropertyDefinitions(args.defines, properties);
    }else if (shaderType == ShaderType::SKYBOX){
        args.vert_file = "sky.vert";
        args.frag_file = "sky.frag";
    }

    if (shaderType == ShaderType::MESH){
        args.defines.push_back({"MAX_LIGHTS", "6"});
        args.defines.push_back({"MAX_SHADOWSMAP", "6"});
        args.defines.push_back({"MAX_SHADOWSCUBEMAP", "1"});
        args.defines.push_back({"MAX_SHADOWCASCADES", "4"});
        args.defines.push_back({"MAX_BONES", "70"});
    }
    if (shaderType == ShaderType::DEPTH){
        args.defines.push_back({"MAX_BONES", "70"});
    }

    if (!supershader::load_input(inputs, args)) {
        printf("Error loading shader input\n");
        return;
    }

    std::vector<supershader::spirv_t> spirvvec;
    spirvvec.resize(inputs.size());
    if (!supershader::compile_to_spirv(spirvvec, inputs, args)) {
        printf("Error compiling to SPIRV\n");
        return;
    }

    std::vector<supershader::spirvcross_t> spirvcrossvec;
    spirvcrossvec.resize(inputs.size());
    if (!supershader::compile_to_lang(spirvcrossvec, spirvvec, inputs, args)) {
        printf("Error cross-compiling\n");
        return;
    }

    convertToShaderData(spirvcrossvec, inputs, args);

    printf("%s\n", shaderData.stages[0].source.c_str());
    printf("%s\n", shaderData.stages[1].source.c_str());
    printf("Shader (%s, %s) generated successfully with %zu stages\n", args.vert_file.c_str(), args.frag_file.c_str(), shaderData.stages.size());
}

ShaderData& Editor::ShaderBuilder::getShaderData() { 
    return shaderData; 
}