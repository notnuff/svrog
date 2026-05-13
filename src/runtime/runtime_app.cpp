#include "runtime_app.h"

#include <stdexcept>
#include <string>

#include <QLoggingCategory>

#include "engine/input/input_system.h"
#include "glfw_input_translator.h"
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
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
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

void RuntimeApp::keyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int mods) {
    auto* self = reinterpret_cast<RuntimeApp*>(glfwGetWindowUserPointer(window));
    if (!self->isInitialized()) return;
    self->input().postKey(input::translateKey(key),
                          input::translateAction(action),
                          input::translateModifiers(mods));
}

void RuntimeApp::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto* self = reinterpret_cast<RuntimeApp*>(glfwGetWindowUserPointer(window));
    if (!self->isInitialized()) return;
    double x = 0.0, y = 0.0;
    glfwGetCursorPos(window, &x, &y);
    self->input().postMouseButton(input::translateMouseButton(button),
                                  input::translateAction(action),
                                  x, y,
                                  input::translateModifiers(mods));
}

void RuntimeApp::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    auto* self = reinterpret_cast<RuntimeApp*>(glfwGetWindowUserPointer(window));
    if (!self->isInitialized()) return;
    self->input().postMouseMove(xpos, ypos);
}

void RuntimeApp::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto* self = reinterpret_cast<RuntimeApp*>(glfwGetWindowUserPointer(window));
    if (!self->isInitialized()) return;
    self->input().postScroll(xoffset, yoffset);
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
