#include <QGuiApplication>
#include <QtQuick/QQuickView>
#include <QQmlApplicationEngine>
#include <qqmlcontext.h>
#include <ranges>
#include <iostream>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "vulkandemorenderer.h"

class Test {
public:
    void initVulkan() {
        createInstance();
    }
private:
    void createInstance() {
        const std::array validationLayers {
            "VK_LAYER_KHRONOS_validation"
        };

        std::vector<const char*> requiredLayers;
#ifdef NDEBUG
        constexpr bool enableValidationLayers = false;
#else
        constexpr bool enableValidationLayers = true;
#endif

        if (enableValidationLayers) {
            requiredLayers.assign(validationLayers.begin(), validationLayers.end());
        }

        // Check if the required layers are supported by the Vulkan implementation.
        auto layerProperties = context.enumerateInstanceLayerProperties();
        if (std::ranges::any_of(requiredLayers, [&layerProperties](auto const& requiredLayer) {
            return std::ranges::none_of(layerProperties,
                                       [requiredLayer](auto const& layerProperty)
                                       { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
        }))
        {
            throw std::runtime_error("One or more required layers are not supported!");
        }

        constexpr std::array extensions {"VK_KHR_wayland_surface"};

        constexpr vk::ApplicationInfo appInfo{ .pApplicationName   = "Hello Triangle",
                .applicationVersion = VK_MAKE_VERSION( 1, 0, 0 ),
                .pEngineName        = "No Engine",
                .engineVersion      = VK_MAKE_VERSION( 1, 0, 0 ),
                .apiVersion         = vk::ApiVersion14 };

        vk::InstanceCreateInfo createInfo{
            .pApplicationInfo = &appInfo,

            .enabledLayerCount = (uint32_t) requiredLayers.size(),
            .ppEnabledLayerNames = requiredLayers.data(),

            .enabledExtensionCount = (uint32_t) extensions.size(),
            .ppEnabledExtensionNames = extensions.data(),

        };

        try {
            instance = vk::raii::Instance(context, createInfo);
        } catch (const vk::SystemError& err) {
            std::cerr << "Vulkan error: " << err.what() << std::endl;
        } catch (const std::exception& err) {
            std::cerr << "Error: " << err.what() << std::endl;
        }

    }

private:
    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;
};


int main(int argc, char **argv)
{
    Test t;
    t.initVulkan();

    QGuiApplication app(argc, argv);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Vulkan);

    QQmlApplicationEngine engine;
    auto demo = std::make_unique<VulkanDemoRenderer>();

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.rootContext()->setContextProperty("demoRenderer", demo.get());
    engine.load(QUrl("qrc:///scenegraph/demo/main.qml"));

    return app.exec();
}
