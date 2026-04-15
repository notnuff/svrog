#pragma once

#include <QObject>

#include <common/vk_common.h>
#include <presentation/i_render_target.h>

class SwapchainRenderTarget : public IRenderTarget {
public:

    virtual ~SwapchainRenderTarget() override = default;

    virtual const vk::raii::CommandBuffer& getTargetCommandBuffer() override;

    virtual vk::raii::ImageView getTargetImageView() override;
    virtual const vk::Extent2D& getTargetImageExtent() override;
    virtual vk::Format getTargetImageFormat() override;

signals:
    std::vector<vk::Image> swapchainImages;
    std::vector<vk::raii::ImageView> swapchainImageViews;
    vk::Format swapchainImageFormat;
    vk::Extent2D swapchainExtent;
};

