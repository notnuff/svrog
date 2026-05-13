#include "app.h"

#include "logging/colored_logger.h"

namespace nuff::app {

App::App() {
    nuff::logging::installColoredLogger();
}

App::~App() = default;

bool App::initialize() {
    if (m_initialized) return false;

    auto config = makePlatformConfig();
    if (!config) return false;

    if (!m_engine.initialize(std::move(config))) return false;

    const auto scenePath = defaultScenePath();
    m_engine.loadDefaultScene(scenePath);

    m_initialized = true;
    return true;
}

void App::shutdown() {
    if (!m_initialized) return;
    m_engine.shutdown();
    m_initialized = false;
}

void App::tick() {
    if (!m_initialized) return;

    const auto now = std::chrono::steady_clock::now();
    float dt = 0.0f;
    if (m_hasLastFrameTime) {
        dt = std::chrono::duration<float>(now - m_lastFrameTime).count();
    }
    m_lastFrameTime = now;
    m_hasLastFrameTime = true;

    m_engine.tick(dt);
}

void App::resize(uint32_t width, uint32_t height) {
    if (!m_initialized) return;
    m_engine.onTargetResized(width, height);
}

void App::notifyFramebufferResized() {
    if (!m_initialized) return;
    m_engine.notifyFramebufferResized();
}

std::string App::defaultScenePath() const {
    return {};
}

void App::onMouseMove(double /*x*/, double /*y*/) {}
void App::onMouseButton(int /*button*/, int /*action*/, int /*mods*/) {}
void App::onKey(int /*key*/, int /*scancode*/, int /*action*/, int /*mods*/) {}

} // namespace nuff::app
