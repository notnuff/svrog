#pragma once

#include <functional>
#include <memory>
#include <vector>

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

#include "core/initialization/build_type_initializer.h"
#include "core/builders/present_device_builder.h"
#include "presentation/surface_builder.h"
#include "presentation/swapchain_builder.h"
#include "presentation/swapchain_pipeline_config_builder.h"

namespace nuff::ui::glfw {

class GlfwInitializer : public CoreInitializerT {
public:
    GlfwInitializer() = default;
    ~GlfwInitializer() override = default;

    void setGlfwExtensions(const std::vector<const char*>& extensions) {
        m_glfwExtensions = extensions;
    }

    void setSurfaceCreator(renderer::SurfaceBuilder::SurfaceCreatorFn creator) {
        m_surfaceCreator = std::move(creator);
    }

    void setExtent(uint32_t width, uint32_t height) {
        m_width = width;
        m_height = height;
    }

protected:
    void prepareBuilders() override {
        CoreInitializerT::prepareBuilders();

        if (auto link = m_builders.get<renderer::InstanceBuilder>(); link.has_value()) {
            auto& surface = link->insert_after<renderer::SurfaceBuilder>()
                .as<renderer::SurfaceBuilder>();
            surface.setSurfaceCreator(m_surfaceCreator);
        }

        if (auto link = m_builders.get<renderer::DeviceBuilder>(); link.has_value()) {
            link->insert_before<renderer::PresentDeviceBuilder>();

            auto swapchainLink = link->insert_after<renderer::SwapchainBuilder>();
            swapchainLink.as<renderer::SwapchainBuilder>().setExtent(m_width, m_height);
            swapchainLink.insert_after<renderer::SwapchainPipelineConfigBuilder>();
        }
    }

    void configureCtx(renderer::CoreCtx& ctx) override {
        auto& ext = ctx.extension<renderer::InstanceExtensionsMixin>();
        ext.instanceExtensions.insert(
            ext.instanceExtensions.end(),
            m_glfwExtensions.begin(),
            m_glfwExtensions.end());
    }

private:
    std::vector<const char*> m_glfwExtensions;
    renderer::SurfaceBuilder::SurfaceCreatorFn m_surfaceCreator;
    uint32_t m_width = 800;
    uint32_t m_height = 600;
};

} // namespace nuff::ui::glfw
