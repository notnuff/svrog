#pragma once

#include "vk_ctx/builders/i_vk_builder.h"

#include <string>
#include <vector>

namespace nuff::renderer {

// PipelineBuilder - Creates graphics pipeline
class PipelineBuilder : public IVkBuilder {
public:
    PipelineBuilder& setVertexShaderPath(const std::string& path);
    PipelineBuilder& setFragmentShaderPath(const std::string& path);
    PipelineBuilder& setVertexShaderCode(const std::vector<uint32_t>& code);
    PipelineBuilder& setFragmentShaderCode(const std::vector<uint32_t>& code);

    void build(VkCtx& ctx) override;

private:
    std::string m_vertexShaderPath;
    std::string m_fragmentShaderPath;
    std::vector<uint32_t> m_vertexShaderCode;
    std::vector<uint32_t> m_fragmentShaderCode;

    static std::vector<uint32_t> readShaderFile(const std::string& path);
    static vk::raii::ShaderModule createShaderModule(const vk::raii::Device& device, const std::vector<uint32_t>& code);
};

} // namespace nuff::renderer

