#include "image_utils.h"

namespace nuff::renderer::utils {
ImageTransitionInfo createImageTransitionInfo(
    vk::Image image,
    vk::ImageLayout old_layout,
    vk::ImageLayout new_layout,
    vk::AccessFlags2 src_access_mask,
    vk::AccessFlags2 dst_access_mask,
    vk::PipelineStageFlags2 src_stage_mask, vk::PipelineStageFlags2 dst_stage_mask)
{
    ImageTransitionInfo info;
    info.barrier = {
        .srcStageMask = src_stage_mask,
        .srcAccessMask = src_access_mask,
        .dstStageMask = dst_stage_mask,
        .dstAccessMask = dst_access_mask,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlags::BitsType::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
    info.dependencyInfo = {
        .dependencyFlags = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &info.barrier
    };

    return info;
}

} // namespace nuff::renderer::utils