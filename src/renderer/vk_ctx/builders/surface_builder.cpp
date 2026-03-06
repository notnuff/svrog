#include "vk_ctx/builders/surface_builder.h"

#include <stdexcept>

namespace nuff::renderer {

SurfaceBuilder& SurfaceBuilder::setSurfaceCreator(SurfaceCreatorFn creator) {
    m_surfaceCreator = std::move(creator);
    return *this;
}

void SurfaceBuilder::build(VkCtx& ctx) {
    if (!m_surfaceCreator) {
        throw std::runtime_error("SurfaceBuilder: No surface creator function set");
    }
    VkSurfaceKHR rawSurface = m_surfaceCreator(*ctx.instance);
    ctx.surface = vk::raii::SurfaceKHR(ctx.instance, rawSurface);
    qCInfo(logger()) << "Surface created";
}

} // namespace nuff::renderer

