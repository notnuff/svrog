#pragma once

#include <memory>

#include "core/context/ctx.h"
#include "presentation/i_render_target.h"

namespace nuff::renderer {

class Renderer {
public:

    void setContext(CoreCtx* ctx);
    void setRenderTarget(std::unique_ptr<IRenderTarget>&& renderTarget);

    void stopAndWait() const;
    void drawFrame();

    void recordRenderingInfo(
        const vk::raii::CommandBuffer &commandBuffer,
        const vk::raii::ImageView &targetImageView,
        const vk::Extent2D &extent,
        vk::Image swapchainImage);

private:
    CoreCtx* m_ctx = nullptr;
    std::unique_ptr<IRenderTarget> m_renderTarget;
    uint32_t m_currentFrame = 0;
    bool m_framebufferResized = false;
};

} // namespace nuff::renderer

