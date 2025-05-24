#pragma once

#include "ShaderData.h"
#include <vector>

#include "supershader.h"
#include "shaders.h"

namespace supershader {
    struct spirvcross_t;
    struct input_t;
    struct args_t;
}

namespace Supernova::Editor {

    class ShaderBuilder {
    private:
        ShaderData shaderData;

        // Mapping functions declarations with camelCase
        ShaderVertexType mapVertexType(supershader::attribute_type_t type);
        ShaderUniformType mapUniformType(supershader::uniform_type_t type);
        TextureType mapTextureType(supershader::texture_type_t type);
        TextureSamplerType mapSamplerType(supershader::texture_samplertype_t type);
        SamplerType mapSamplerFilterType(supershader::sampler_type_t type);
        ShaderStorageBufferType mapStorageType(supershader::storage_buffer_type_t type);
        ShaderStageType mapStageType(supershader::stage_type_t type);
        ShaderLang mapLang(supershader::lang_type_t lang);

        void convertToShaderData(
            const std::vector<supershader::spirvcross_t>& spirvcrossvec,
            const std::vector<supershader::input_t>& inputs,
            const supershader::args_t& args);

    public:
        ShaderBuilder();
        virtual ~ShaderBuilder();

        void execute();

        ShaderData& getShaderData();
    };

}