#pragma once

#include "Project.h"

#include "pool/ShaderPool.h"
#include "ShaderData.h"
#include <vector>
#include <unordered_map>
#include <mutex>
#include <future>

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
        static std::unordered_map<ShaderKey, ShaderData> shaderDataCache;
        static std::unordered_map<ShaderKey, std::future<ShaderData>> pendingBuilds;
        static std::mutex cacheMutex;

        static std::atomic<bool> shutdownRequested;

        // Mapping functions declarations with camelCase
        ShaderVertexType mapVertexType(supershader::attribute_type_t type);
        ShaderUniformType mapUniformType(supershader::uniform_type_t type);
        TextureType mapTextureType(supershader::texture_type_t type);
        TextureSamplerType mapSamplerType(supershader::texture_samplertype_t type);
        SamplerType mapSamplerFilterType(supershader::sampler_type_t type);
        ShaderStorageBufferType mapStorageType(supershader::storage_buffer_type_t type);
        ShaderStageType mapStageType(supershader::stage_type_t type);
        ShaderLang mapLang(supershader::lang_type_t lang);

        ShaderData convertToShaderData(
            const std::vector<supershader::spirvcross_t>& spirvcrossvec,
            const std::vector<supershader::input_t>& inputs,
            const supershader::args_t& args);

        void addMeshPropertyDefinitions(std::vector<supershader::define_t>& defs, const uint32_t prop);
        void addDepthMeshPropertyDefinitions(std::vector<supershader::define_t>& defs, const uint32_t prop);
        void addUIPropertyDefinitions(std::vector<supershader::define_t>& defs, const uint32_t prop);
        void addPointsPropertyDefinitions(std::vector<supershader::define_t>& defs, const uint32_t prop);
        void addLinesPropertyDefinitions(std::vector<supershader::define_t>& defs, const uint32_t prop);

        ShaderData buildShaderInternal(ShaderKey shaderKey, Project* project);
        std::string getShaderDisplayName(ShaderKey key);

    public:
        ShaderBuilder();
        virtual ~ShaderBuilder();

        ShaderBuildResult buildShader(ShaderKey shaderKey, Project* project);

        static void requestShutdown();

        ShaderData& getShaderData(ShaderKey shaderKey);
    };

}