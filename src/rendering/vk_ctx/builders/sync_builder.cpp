#include "vk_ctx/builders/sync_builder.h"

namespace nuff::renderer {

void SyncBuilder::build(VkCtx& ctx) {
    ctx.imageAvailableSemaphores.clear();
    ctx.renderFinishedSemaphores.clear();
    ctx.inFlightFences.clear();

    ctx.imageAvailableSemaphores.reserve(VkCtx::MAX_FRAMES_IN_FLIGHT);
    ctx.inFlightFences.reserve(VkCtx::MAX_FRAMES_IN_FLIGHT);

    const auto swapchainImageCount = ctx.swapchainImages.size();
    ctx.renderFinishedSemaphores.reserve(swapchainImageCount);

    vk::SemaphoreCreateInfo semaphoreInfo{};
    vk::FenceCreateInfo fenceInfo{
        .flags = vk::FenceCreateFlagBits::eSignaled
    };

    for (size_t i = 0; i < VkCtx::MAX_FRAMES_IN_FLIGHT; i++) {
        ctx.imageAvailableSemaphores.emplace_back(ctx.device, semaphoreInfo);
        ctx.inFlightFences.emplace_back(ctx.device, fenceInfo);
    }
    
    for (size_t i = 0; i < swapchainImageCount; i++) {
        ctx.renderFinishedSemaphores.emplace_back(ctx.device, semaphoreInfo);
    }

    qCInfo(logger()) << "Sync objects created";
}

} // namespace nuff::renderer

