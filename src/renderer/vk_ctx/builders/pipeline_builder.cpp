#include "vk_ctx/builders/pipeline_builder.h"

#include <fstream>
#include <stdexcept>

namespace nuff::renderer {

PipelineBuilder& PipelineBuilder::setVertexShaderPath(const std::string& path) {
    m_vertexShaderPath = path;
    return *this;
}

PipelineBuilder& PipelineBuilder::setFragmentShaderPath(const std::string& path) {
    m_fragmentShaderPath = path;
    return *this;
}

PipelineBuilder& PipelineBuilder::setVertexShaderCode(const std::vector<uint32_t>& code) {
    m_vertexShaderCode = code;
    return *this;
}

PipelineBuilder& PipelineBuilder::setFragmentShaderCode(const std::vector<uint32_t>& code) {
    m_fragmentShaderCode = code;
    return *this;
}

std::vector<uint32_t> PipelineBuilder::readShaderFile(const std::string& path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + path);
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(fileSize));
    file.close();

    return buffer;
}

vk::raii::ShaderModule PipelineBuilder::createShaderModule(const vk::raii::Device& device, const std::vector<uint32_t>& code) {
    vk::ShaderModuleCreateInfo createInfo{{}, code.size() * sizeof(uint32_t), code.data()};
    return vk::raii::ShaderModule(device, createInfo);
}


void PipelineBuilder::build(VkCtx& ctx) {
    std::vector<uint32_t> vertCode = m_vertexShaderCode.empty()
        ? readShaderFile(m_vertexShaderPath) : m_vertexShaderCode;
    std::vector<uint32_t> fragCode = m_fragmentShaderCode.empty()
        ? readShaderFile(m_fragmentShaderPath) : m_fragmentShaderCode;

    vk::raii::ShaderModule vertShaderModule = createShaderModule(ctx.device, vertCode);
    vk::raii::ShaderModule fragShaderModule = createShaderModule(ctx.device, fragCode);

    vk::PipelineShaderStageCreateInfo shaderStages[] = {
        {{}, vk::ShaderStageFlagBits::eVertex, *vertShaderModule, "main"},
        {{}, vk::ShaderStageFlagBits::eFragment, *fragShaderModule, "main"}
    };

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
        {}, vk::PrimitiveTopology::eTriangleList, VK_FALSE
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
        {}, VK_FALSE, VK_FALSE,
        vk::PolygonMode::eFill,
        vk::CullModeFlagBits::eBack,
        vk::FrontFace::eClockwise,
        VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f
    };

    vk::PipelineMultisampleStateCreateInfo multisampling{
        {}, vk::SampleCountFlagBits::e1, VK_FALSE
    };

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{
        VK_FALSE,
        vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
        vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    };

    vk::PipelineColorBlendStateCreateInfo colorBlending{
        {}, VK_FALSE, vk::LogicOp::eCopy,
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

