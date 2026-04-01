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
        {
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = *vertShaderModule,
            .pName = "vertMain"
        },
        {
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = *fragShaderModule,
            .pName = "fragMain"
        }
    };

    std::vector dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };
    vk::PipelineDynamicStateCreateInfo dynamicStateInfo = {
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };

    // TODO: add data transfer later
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
        .topology = vk::PrimitiveTopology::eTriangleList,
        .primitiveRestartEnable = vk::False
    };

    // TODO: split screen by using
    vk::Viewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(ctx.swapchainExtent.width),
        .height = static_cast<float>(ctx.swapchainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    vk::Rect2D scissor{
        .offset = {0, 0},
        .extent = ctx.swapchainExtent
    };

    vk::PipelineViewportStateCreateInfo viewportState{
        .viewportCount = 1,
        // TODO: actual viewport should be set dynamically
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };

    vk::PipelineRasterizationStateCreateInfo rasterizer{
        .depthClampEnable = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = vk::False,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f
    };

    vk::PipelineMultisampleStateCreateInfo multisampling{
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = vk::False
    };

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable = vk::False,
        .srcColorBlendFactor = vk::BlendFactor::eOne,
        .dstColorBlendFactor = vk::BlendFactor::eZero,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp = vk::BlendOp::eAdd,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    };

    vk::PipelineColorBlendStateCreateInfo colorBlending{
        .logicOpEnable = vk::False,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    ctx.pipelineLayout = vk::raii::PipelineLayout(ctx.device, pipelineLayoutInfo);

    vk::GraphicsPipelineCreateInfo pipelineInfo{
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pColorBlendState = &colorBlending,
        // .pDynamicState = &dynamicStateInfo,
        .layout = *ctx.pipelineLayout,
        .renderPass = *ctx.renderPass,
        .subpass = 0
    };

    ctx.graphicsPipeline = vk::raii::Pipeline(ctx.device, nullptr, pipelineInfo);

    qCInfo(logger()) << "Graphics pipeline created";
}

} // namespace nuff::renderer

