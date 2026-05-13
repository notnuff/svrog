#pragma once

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

#include "engine/core/platform_config.h"

#include <string>

namespace nuff::runtime {

class GlfwPlatformConfig : public engine::PlatformConfig {
public:
    explicit GlfwPlatformConfig(GLFWwindow* window);

    void setShaderPath(std::string vertexPath, std::string fragmentPath);

    void onRecreate(renderer::CoreCtx& ctx,
                    renderer::IRenderTarget& target) override;

protected:
    std::unique_ptr<renderer::CoreInitializer> makeInitializer() override;
    std::unique_ptr<renderer::IRenderTarget>
        makeRenderTarget(renderer::CoreCtx& ctx) override;

private:
    GLFWwindow* m_window;
    std::string m_vertexShaderPath;
    std::string m_fragmentShaderPath;
};

} // namespace nuff::runtime
