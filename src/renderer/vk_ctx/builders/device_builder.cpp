#include "vk_ctx/builders/device_builder.h"

#include <set>
#include <stdexcept>

namespace nuff::renderer {

DeviceBuilder& DeviceBuilder::requireGraphicsQueue(bool require) {
    m_requireGraphics = require;
    return *this;
}

DeviceBuilder& DeviceBuilder::requirePresentQueue(bool require) {
    m_requirePresent = require;
    return *this;
}

DeviceBuilder& DeviceBuilder::addDeviceExtensions(const std::vector<const char*>& extensions) {
    m_deviceExtensions.insert(m_deviceExtensions.end(), extensions.begin(), extensions.end());
    return *this;
}

QueueFamilyIndices DeviceBuilder::findQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
    QueueFamilyIndices indices;
    auto queueFamilies = device.getQueueFamilyProperties();

    uint32_t i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = i;
        }

        if (device.getSurfaceSupportKHR(i, surface)) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) break;
        i++;
    }

    return indices;
}

bool DeviceBuilder::checkDeviceExtensionSupport(vk::PhysicalDevice device,
                                                 const std::vector<const char*>& extensions) {
    auto availableExtensions = device.enumerateDeviceExtensionProperties();
    std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool DeviceBuilder::isDeviceSuitable(vk::PhysicalDevice device, vk::SurfaceKHR surface,
                                      const std::vector<const char*>& extensions) {
    auto indices = findQueueFamilies(device, surface);
    bool extensionsSupported = checkDeviceExtensionSupport(device, extensions);

    bool swapchainAdequate = false;
    if (extensionsSupported) {
        auto formats = device.getSurfaceFormatsKHR(surface);
        auto presentModes = device.getSurfacePresentModesKHR(surface);
        swapchainAdequate = !formats.empty() && !presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapchainAdequate;
}

void DeviceBuilder::build(VkCtx& ctx) {
    auto physicalDevices = ctx.instance.enumeratePhysicalDevices();
    if (physicalDevices.empty()) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support");
    }

    for (const auto& device : physicalDevices) {
        if (isDeviceSuitable(device, ctx.surface, m_deviceExtensions)) {
            ctx.physicalDevice = device;
            break;
        }
    }

    if (!ctx.physicalDevice) {
        throw std::runtime_error("Failed to find a suitable GPU");
    }

    auto props = ctx.physicalDevice.getProperties();
    qCInfo(logger()) << "Selected GPU:" << props.deviceName.data();

    ctx.queueFamilyIndices = findQueueFamilies(ctx.physicalDevice, ctx.surface);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        ctx.queueFamilyIndices.graphicsFamily.value(),
        ctx.queueFamilyIndices.presentFamily.value()
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo{{}, queueFamily, 1, &queuePriority};
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::PhysicalDeviceFeatures deviceFeatures{};

    vk::DeviceCreateInfo createInfo{
        {},
        static_cast<uint32_t>(queueCreateInfos.size()),
        queueCreateInfos.data(),
        0, nullptr,
        static_cast<uint32_t>(m_deviceExtensions.size()),
        m_deviceExtensions.data(),
        &deviceFeatures
    };

    ctx.device = ctx.physicalDevice.createDevice(createInfo);
    ctx.graphicsQueue = ctx.device.getQueue(ctx.queueFamilyIndices.graphicsFamily.value(), 0);
    ctx.presentQueue = ctx.device.getQueue(ctx.queueFamilyIndices.presentFamily.value(), 0);

    qCInfo(logger()) << "Logical device created";
}

} // namespace nuff::renderer

