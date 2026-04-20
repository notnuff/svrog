#pragma once

#include <functional>
#include <memory>

#include "core/context/ctx.h"
#include "presentation/i_render_target.h"

namespace nuff::renderer {

class Renderer {
public:
    using RecreateCallback = std::function<void()>;

    void setContext(CoreCtx* ctx);
    void setRenderTarget(IRenderTarget* renderTarget);
    void setRecreateCallback(RecreateCallback callback);

    void notifyFramebufferResized();

    void stopAndWait() const;
    void drawFrame();

private:
    void recordRendering();

    CoreCtx* m_ctx = nullptr;
    IRenderTarget* m_renderTarget = nullptr;
    RecreateCallback m_recreateCallback;
    bool m_framebufferResized = false;
};

} // namespace nuff::renderer

