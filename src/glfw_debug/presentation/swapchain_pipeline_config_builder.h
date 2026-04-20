#pragma once

#include "core/builders/i_vk_builder.h"

namespace nuff::renderer {

// Must run AFTER SwapchainBuilder and BEFORE PipelineBuilder
class SwapchainPipelineConfigBuilder : public IVkBuilder {
public:
    void build(CoreCtx& ctx) override {
        auto& swapchain = ctx.extension<SwapchainCtxMixin>();
        auto& config = ctx.extension<PipelineConfigMixin>();
        config.colorAttachmentFormat = swapchain.swapchainImageFormat;

        qCInfo(logger()) << "Pipeline format set from swapchain:"
                         << vk::to_string(config.colorAttachmentFormat).c_str();
    }
};

} // namespace nuff::renderer
