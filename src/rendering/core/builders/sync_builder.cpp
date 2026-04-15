#include "core/builders/sync_builder.h"

namespace nuff::renderer {

void SyncBuilder::build(CoreCtx& ctx) {
    auto& present = ctx.extension<PresentCtxMixin>();
    auto& swapchain = ctx.extension<SwapchainCtxMixin>();

    present.imageAvailableSemaphores.clear();
    present.renderFinishedSemaphores.clear();
    present.inFlightFences.clear();

    present.imageAvailableSemaphores.reserve(present.maxFramesInFlight);
    present.inFlightFences.reserve(present.maxFramesInFlight);

    const auto swapchainImageCount = swapchain.swapchainImages.size();
    present.renderFinishedSemaphores.reserve(swapchainImageCount);

    vk::SemaphoreCreateInfo semaphoreInfo{};
    vk::FenceCreateInfo fenceInfo{
        .flags = vk::FenceCreateFlagBits::eSignaled
    };

    for (size_t i = 0; i < present.maxFramesInFlight; i++) {
        present.imageAvailableSemaphores.emplace_back(ctx.device, semaphoreInfo);
        present.inFlightFences.emplace_back(ctx.device, fenceInfo);
    }
    
    for (size_t i = 0; i < swapchainImageCount; i++) {
        present.renderFinishedSemaphores.emplace_back(ctx.device, semaphoreInfo);
    }

    qCInfo(logger()) << "Sync objects created";
}

} // namespace nuff::renderer

