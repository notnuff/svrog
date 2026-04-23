#include "core/builders/pipeline_builder.h"
#include "primitives/vertex.h"
#include "primitives/push_constants.h"
#include "utils/shader_utils.h"
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

void PipelineBuilder::build(CoreCtx& ctx) {
    auto& pipelineConfig = ctx.extension<PipelineConfigMixin>();
    auto& pipeline = ctx.extension<PipelineCtxMixin>();

    std::vector<char> vertCode = m_vertexShaderCode.empty()
        ? nuff::utils::readFile(m_vertexShaderPath) : m_vertexShaderCode;
    std::vector<char> fragCode = m_fragmentShaderCode.empty()
        ? nuff::utils::readFile(m_fragmentShaderPath) : m_fragmentShaderCode;

    vk::raii::ShaderModule vertShaderModule = utils::createShaderModule(ctx.device, vertCode);
    vk::raii::ShaderModule fragShaderModule = utils::createShaderModule(ctx.device, fragCode);

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

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data()
    };

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
        .topology = vk::PrimitiveTopology::eTriangleList,
        .primitiveRestartEnable = vk::False
    };
    
    vk::PipelineViewportStateCreateInfo viewportState{
        .viewportCount = 1,
        .scissorCount = 1,
    };

    vk::PipelineRasterizationStateCreateInfo rasterizer{
        .depthClampEnable = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eCounterClockwise,
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

    // vk::PipelineColorBlendAttachmentState colorBlendAttachment{
    //     .blendEnable = vk::False,
    //     .srcColorBlendFactor = vk::BlendFactor::eOne,
    //     .dstColorBlendFactor = vk::BlendFactor::eZero,
    //     .colorBlendOp = vk::BlendOp::eAdd,
    //     .srcAlphaBlendFactor = vk::BlendFactor::eOne,
    //     .dstAlphaBlendFactor = vk::BlendFactor::eZero,
    //     .alphaBlendOp = vk::BlendOp::eAdd,
    //     .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
    //         vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    // };

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable         = vk::True,
        .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
        .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
        .colorBlendOp        = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp        = vk::BlendOp::eAdd,
        .colorWriteMask      = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};
    
    vk::PipelineColorBlendStateCreateInfo colorBlending{
        .logicOpEnable = vk::False,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment
    };

    std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {{
        {
            .binding = 0,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eVertex
        },
        {
            .binding = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eFragment
        }
    }};

    vk::DescriptorSetLayoutCreateInfo layoutInfo{
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data()
    };
    pipeline.descriptorSetLayout = vk::raii::DescriptorSetLayout(ctx.device, layoutInfo);

    vk::DescriptorSetLayout setLayout = *pipeline.descriptorSetLayout;

    vk::PushConstantRange pushConstantRange{
        .stageFlags = vk::ShaderStageFlagBits::eFragment,
        .offset = 0,
        .size = sizeof(TimePushConstantData)
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
        .setLayoutCount = 1,
        .pSetLayouts = &setLayout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange
    };
    pipeline.pipelineLayout = vk::raii::PipelineLayout(ctx.device, pipelineLayoutInfo);

    vk::StructureChain pipelineInfoChain = {
        vk::GraphicsPipelineCreateInfo {
            .stageCount = 2,
            .pStages = shaderStages,
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pColorBlendState = &colorBlending,
            .pDynamicState = &dynamicStateInfo,
            .layout = *pipeline.pipelineLayout,
            .renderPass = nullptr,

        },
        vk::PipelineRenderingCreateInfo {
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &pipelineConfig.colorAttachmentFormat
        }
    };


    pipeline.graphicsPipeline = vk::raii::Pipeline(
        ctx.device,
        nullptr,
        pipelineInfoChain.get<vk::GraphicsPipelineCreateInfo>()
    );

    qCInfo(logger()) << "Graphics pipeline created";
}

} // namespace nuff::renderer

