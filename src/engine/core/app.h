#pragma once

#include "engine.h"
#include "platform_config.h"

#include <QtCore/qtclasshelpermacros.h>

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

namespace nuff::engine::input { class InputSystem; }

namespace nuff::app {

class App {
public:
    App();
    virtual ~App();

    Q_DISABLE_COPY(App)

    bool initialize();
    void shutdown();
    void tick();

    virtual void resize(uint32_t width, uint32_t height);
    void notifyFramebufferResized();

    bool isInitialized() const { return m_initialized; }

    engine::Engine&       engineSystem()       { return m_engine; }
    const engine::Engine& engineSystem() const { return m_engine; }

    engine::input::InputSystem&       input()       { return m_engine.input(); }
    const engine::input::InputSystem& input() const { return m_engine.input(); }

protected:
    virtual std::unique_ptr<engine::PlatformConfig> makePlatformConfig() = 0;
    virtual std::string                             defaultScenePath() const;

private:
    engine::Engine                        m_engine;
    std::chrono::steady_clock::time_point m_lastFrameTime;
    bool                                  m_hasLastFrameTime = false;
    bool                                  m_initialized = false;
};

} // namespace nuff::app
