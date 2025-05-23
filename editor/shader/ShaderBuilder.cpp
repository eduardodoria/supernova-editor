#include "ShaderBuilder.h"

#include <cstring>

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

void Editor::ShaderBuilder::execute(){
    std::vector<supershader::input_t> inputs;
    supershader::args_t args = supershader::initialize_args();
    args.isValid = true;
    args.vert_file = "mesh.vert";
    args.frag_file = "mesh.frag";
    args.useBuffers = true;
    args.fileBuffers = Editor::shaderMap;
    args.lang = supershader::LANG_GLSL;
    args.version = 410;
    args.output_basename="output";
    args.defines.push_back({"USE_PUNCTUAL", "1"});
    args.defines.push_back({"MAX_LIGHTS", "4"});

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

    printf("Shader generated successfully with %zu stages\n", shaderData.stages.size());
}

ShaderData& Editor::ShaderBuilder::getShaderData() { 
    return shaderData; 
}