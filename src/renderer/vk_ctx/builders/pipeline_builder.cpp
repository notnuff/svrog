#include "vk_ctx/builders/pipeline_builder.h"
#include "shaders/shader_utils.h"
#include "utils/file_utils.h"

namespace nuff::renderer {

PipelineBuilder& PipelineBuilder::setVertexShaderPath(const std::string& path) {
    m_vertexShaderPath = path;
    return *this;
}

PipelineBuilder& PipelineBuilder::setFragmentShaderPath(const std::string& path) {
    m_fragmentShaderPath = path;
    return *this;
}

PipelineBuilder& PipelineBuilder::setVertexShaderCode(const std::vector<char>& code) {
    m_vertexShaderCode = code;
    return *this;
}

PipelineBuilder& PipelineBuilder::setFragmentShaderCode(const std::vector<char>& code) {
    m_fragmentShaderCode = code;
    return *this;
}

void PipelineBuilder::build(VkCtx& ctx) {
    std::vector<char> vertCode = m_vertexShaderCode.empty()
        ? utils::readFile(m_vertexShaderPath) : m_vertexShaderCode;
    std::vector<char> fragCode = m_fragmentShaderCode.empty()
        ? utils::readFile(m_fragmentShaderPath) : m_fragmentShaderCode;

    vk::raii::ShaderModule vertShaderModule = shaders::createShaderModule(ctx.device, vertCode);
    vk::raii::ShaderModule fragShaderModule = shaders::createShaderModule(ctx.device, fragCode);

    vk::PipelineShaderStageCreateInfo shaderStages[] = {
        {{}, vk::ShaderStageFlagBits::eVertex, *vertShaderModule, "vertMain"},
        {{}, vk::ShaderStageFlagBits::eFragment, *fragShaderModule, "fragMain"}
    };

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
        {}, vk::PrimitiveTopology::eTriangleList, vk::False
    };

    vk::Viewport viewport{
        0.0f, 0.0f,
        static_cast<float>(ctx.swapchainExtent.width),
        static_cast<float>(ctx.swapchainExtent.height),
        0.0f, 1.0f
    };

    vk::Rect2D scissor{{0, 0}, ctx.swapchainExtent};

    vk::PipelineViewportStateCreateInfo viewportState{{}, 1, &viewport, 1, &scissor};

    vk::PipelineRasterizationStateCreateInfo rasterizer{
        {}, vk::False, vk::False,
        vk::PolygonMode::eFill,
        vk::CullModeFlagBits::eBack,
        vk::FrontFace::eClockwise,
        vk::False, 0.0f, 0.0f, 0.0f, 1.0f
    };

    vk::PipelineMultisampleStateCreateInfo multisampling{
        {}, vk::SampleCountFlagBits::e1, vk::False
    };

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{
        vk::False,
        vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
        vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    };

    vk::PipelineColorBlendStateCreateInfo colorBlending{
        {}, vk::False, vk::LogicOp::eCopy,
        1, &colorBlendAttachment
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    ctx.pipelineLayout = vk::raii::PipelineLayout(ctx.device, pipelineLayoutInfo);

    vk::GraphicsPipelineCreateInfo pipelineInfo{
        {},
        2, shaderStages,
        &vertexInputInfo,
        &inputAssembly,
        nullptr,
        &viewportState,
        &rasterizer,
        &multisampling,
        nullptr,
        &colorBlending,
        nullptr,
        *ctx.pipelineLayout,
        *ctx.renderPass,
        0
    };

    ctx.graphicsPipeline = vk::raii::Pipeline(ctx.device, nullptr, pipelineInfo);

    // Shader modules are automatically destroyed by RAII

    qCInfo(logger()) << "Graphics pipeline created";
}

} // namespace nuff::renderer

