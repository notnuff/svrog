#include "vk_ctx/vk_ctx.h"

namespace nuff::renderer {

// TODO: make a cleaner or move clean responsibility to respective builders
void VkCtx::cleanup() {
    if (device) {
        device.waitIdle();
    }

    for (auto& semaphore : renderFinishedSemaphores) {
        if (semaphore) device.destroySemaphore(semaphore);
    }
    for (auto& semaphore : imageAvailableSemaphores) {
        if (semaphore) device.destroySemaphore(semaphore);
    }
    for (auto& fence : inFlightFences) {
        if (fence) device.destroyFence(fence);
    }

    if (commandPool) {
        device.destroyCommandPool(commandPool);
    }

    for (auto& framebuffer : framebuffers) {
        if (framebuffer) device.destroyFramebuffer(framebuffer);
    }

    if (graphicsPipeline) {
        device.destroyPipeline(graphicsPipeline);
    }
    if (pipelineLayout) {
        device.destroyPipelineLayout(pipelineLayout);
    }

    if (renderPass) {
        device.destroyRenderPass(renderPass);
    }

    for (auto& imageView : swapchainImageViews) {
        if (imageView) device.destroyImageView(imageView);
    }

    if (swapchain) {
        device.destroySwapchainKHR(swapchain);
    }

    if (device) {
        device.destroy();
    }

    if (surface && instance) {
        instance.destroySurfaceKHR(surface);
    }

    // TODO: encapsulate destruction logic to the respective builders
#ifndef NDEBUG
    if (debugMessenger && instance) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            static_cast<VkInstance>(instance), "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(static_cast<VkInstance>(instance), debugMessenger, nullptr);
        }
    }
#endif

    if (instance) {
        instance.destroy();
    }
}

} // namespace nuff::renderer

