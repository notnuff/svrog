#include "vk_ctx/builders/sync_builder.h"

namespace nuff::renderer {

void SyncBuilder::build(VkCtx& ctx) {
    ctx.imageAvailableSemaphores.resize(VkCtx::MAX_FRAMES_IN_FLIGHT);
    ctx.renderFinishedSemaphores.resize(VkCtx::MAX_FRAMES_IN_FLIGHT);
    ctx.inFlightFences.resize(VkCtx::MAX_FRAMES_IN_FLIGHT);

    vk::SemaphoreCreateInfo semaphoreInfo{};
    vk::FenceCreateInfo fenceInfo{vk::FenceCreateFlagBits::eSignaled};

    for (size_t i = 0; i < VkCtx::MAX_FRAMES_IN_FLIGHT; i++) {
        ctx.imageAvailableSemaphores[i] = ctx.device.createSemaphore(semaphoreInfo);
        ctx.renderFinishedSemaphores[i] = ctx.device.createSemaphore(semaphoreInfo);
        ctx.inFlightFences[i] = ctx.device.createFence(fenceInfo);
    }

    qCInfo(logger()) << "Sync objects created";
}

} // namespace nuff::renderer

