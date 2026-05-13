#include "runtime_app.h"

#include <stdexcept>
#include <string>

#include <QLoggingCategory>

#include "glfw_platform_config.h"

namespace L {
Q_LOGGING_CATEGORY(runtimeApp, "nuff.runtime.app")
}

namespace nuff::runtime {

RuntimeApp::~RuntimeApp() {
    if (isInitialized()) shutdown();
}

void RuntimeApp::run() {
    initWindow();

    if (!app::App::initialize()) {
        throw std::runtime_error("Failed to initialize engine");
    }

    mainLoop();
    cleanup();
}

void RuntimeApp::initWindow() {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT,
                                "Runtime - Triangle", nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
}

std::unique_ptr<engine::PlatformConfig> RuntimeApp::makePlatformConfig() {
    const std::string shaderPath = "shaders/triangle_shader.spv";
    qCInfo(L::runtimeApp) << "Loading shader from:" << shaderPath.c_str();

    auto config = std::make_unique<GlfwPlatformConfig>(m_window);
    config->setShaderPath(shaderPath, shaderPath);
    return config;
}

std::string RuntimeApp::defaultScenePath() const {
    return "models/mv_spartan/scene.gltf";
}

void RuntimeApp::framebufferResizeCallback(GLFWwindow* window, int /*width*/, int /*height*/) {
    auto* self = reinterpret_cast<RuntimeApp*>(glfwGetWindowUserPointer(window));
    self->notifyFramebufferResized();
}

void RuntimeApp::mainLoop() {
    qCInfo(L::runtimeApp) << "Entering main loop. Close window to exit.";
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
        tick();
    }
}

void RuntimeApp::cleanup() {
    shutdown();

    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
    qCInfo(L::runtimeApp) << "Cleanup complete";
}

} // namespace nuff::runtime
