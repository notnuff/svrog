#pragma once

#include "core/builders/i_vk_builder.h"

#include <string>
#include <vector>

namespace nuff::renderer {

// PipelineBuilder - Creates graphics pipeline
class PipelineBuilder : public IVkBuilder {
public:
    PipelineBuilder& setVertexShaderPath(const std::string& path);
    PipelineBuilder& setFragmentShaderPath(const std::string& path);
    PipelineBuilder& setVertexShaderCode(const std::vector<char>& code);
    PipelineBuilder& setFragmentShaderCode(const std::vector<char>& code);

    void build(CoreCtx& ctx) override;

private:
    std::string m_vertexShaderPath;
    std::string m_fragmentShaderPath;
    std::vector<char> m_vertexShaderCode;
    std::vector<char> m_fragmentShaderCode;
};

} // namespace nuff::renderer

