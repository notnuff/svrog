#pragma once

#include "vk_ctx/vk_ctx.h"

#include <QLoggingCategory>

namespace nuff::renderer {

// Base builder interface for Vulkan components.
class IVkBuilder {
public:
    virtual ~IVkBuilder() = default;
    virtual void build(VkCtx& ctx) = 0;

protected:
    // Shared logging category for all builders
    static const QLoggingCategory& logger() {
        static const QLoggingCategory category("nuff.renderer.vk.builder");
        return category;
    }
};

} // namespace nuff::renderer

