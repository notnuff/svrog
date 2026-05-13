#pragma once

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

#include "engine/core/app.h"

namespace nuff::runtime {

class RuntimeApp : public app::App {
public:
    RuntimeApp() = default;
    ~RuntimeApp() override;

    void run();

protected:
    std::unique_ptr<engine::PlatformConfig> makePlatformConfig() override;
    std::string                             defaultScenePath() const override;

private:
    static constexpr uint32_t DEFAULT_WIDTH = 800;
    static constexpr uint32_t DEFAULT_HEIGHT = 600;

    void initWindow();
    void mainLoop();
    void cleanup();

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    GLFWwindow* m_window = nullptr;
};

} // namespace nuff::runtime
