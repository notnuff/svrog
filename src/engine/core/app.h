#pragma once

#include "engine.h"
#include "platform_config.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

namespace nuff::app {

class App {
public:
    App();
    virtual ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    bool initialize();
    void shutdown();
    void tick();

    virtual void resize(uint32_t width, uint32_t height);
    void notifyFramebufferResized();

    bool isInitialized() const { return m_initialized; }

    engine::Engine&       engineSystem()       { return m_engine; }
    const engine::Engine& engineSystem() const { return m_engine; }

protected:
    virtual std::unique_ptr<engine::PlatformConfig> makePlatformConfig() = 0;
    virtual std::string                             defaultScenePath() const;

    virtual void onMouseMove(double x, double y);
    virtual void onMouseButton(int button, int action, int mods);
    virtual void onKey(int key, int scancode, int action, int mods);

private:
    engine::Engine                        m_engine;
    std::chrono::steady_clock::time_point m_lastFrameTime;
    bool                                  m_hasLastFrameTime = false;
    bool                                  m_initialized = false;
};

} // namespace nuff::app
