#pragma once
#include <common/vk_common.h>

namespace nuff::renderer::utils {

    struct ImageTransitionInfo {
        vk::ImageMemoryBarrier2 barrier;
        vk::DependencyInfo dependencyInfo;
    };

    ImageTransitionInfo createImageTransitionInfo(
        vk::Image image,
        vk::ImageLayout old_layout,
        vk::ImageLayout new_layout,
        vk::AccessFlags2 src_access_mask,
        vk::AccessFlags2 dst_access_mask,
        vk::PipelineStageFlags2 src_stage_mask,
        vk::PipelineStageFlags2 dst_stage_mask);

} // namespace nuff::renderer::utils
