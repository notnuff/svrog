#pragma once

#include <QObject>
#include <common/vk_common.h>

class IRenderTarget : public QObject {
public:

    virtual ~IRenderTarget() override = default;

    virtual const vk::raii::CommandBuffer& getTargetCommandBuffer() = 0;

    virtual vk::raii::ImageView getTargetImageView() = 0;
    virtual const vk::Extent2D& getTargetImageExtent() = 0;
    virtual vk::Format getTargetImageFormat() = 0;

signals:
    // std::vector<vk::Image> swapchainImages;
    // std::vector<vk::raii::ImageView> swapchainImageViews;
    // vk::Format swapchainImageFormat;
    // vk::Extent2D swapchainExtent;
    void imageFormatChanged();
    void imageExtentChanged();

};

