#include "platform_config.h"

namespace nuff::engine {

std::unique_ptr<renderer::CoreCtx> PlatformConfig::buildContext() {
    auto initializer = makeInitializer();
    if (!initializer) return nullptr;
    return initializer->buildCtx();
}

std::unique_ptr<renderer::IRenderTarget>
PlatformConfig::createRenderTarget(renderer::CoreCtx& ctx) {
    return makeRenderTarget(ctx);
}

void PlatformConfig::onRecreate(renderer::CoreCtx& /*ctx*/,
                                renderer::IRenderTarget& /*target*/) {}

} // namespace nuff::engine
