#pragma once

#include "context/ctx.h"
#include "context/initialization/core_initializer.h"
#include "presentation/i_render_target.h"

#include <memory>

namespace nuff::engine {

class PlatformConfig {
public:
    virtual ~PlatformConfig() = default;

    std::unique_ptr<renderer::CoreCtx>       buildContext();
    std::unique_ptr<renderer::IRenderTarget> createRenderTarget(renderer::CoreCtx& ctx);

    virtual void onRecreate(renderer::CoreCtx& ctx, renderer::IRenderTarget& target);

protected:
    virtual std::unique_ptr<renderer::CoreInitializer> makeInitializer() = 0;
    virtual std::unique_ptr<renderer::IRenderTarget>
        makeRenderTarget(renderer::CoreCtx& ctx) = 0;
};

} // namespace nuff::engine
