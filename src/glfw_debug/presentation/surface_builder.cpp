#include "surface_builder.h"

#include <stdexcept>

namespace nuff::renderer {

// TODO: try off-screen rendering and rendering over the internet via WebRTC
// TODO: adapt to QWindow
SurfaceBuilder& SurfaceBuilder::setSurfaceCreator(SurfaceCreatorFn creator) {
    m_surfaceCreator = std::move(creator);
    return *this;
}

void SurfaceBuilder::build(CoreCtx& ctx) {
    if (!m_surfaceCreator) {
        throw std::runtime_error("SurfaceBuilder: No surface creator function set");
    }
    auto& swapchain = ctx.extension<SwapchainCtxMixin>();
    VkSurfaceKHR rawSurface = m_surfaceCreator(*ctx.instance);
    swapchain.surface = vk::raii::SurfaceKHR(ctx.instance, rawSurface);
    qCInfo(logger()) << "Surface created";
}

} // namespace nuff::renderer

