#include "vk_ctx/builders/device_builder.h"

#include <set>
#include <sstream>
#include <stdexcept>

namespace {
std::string formatDeviceFeatures(const vk::PhysicalDeviceFeatures& features) {
    std::ostringstream oss;
    oss << "\n";

    // Helper macro to print feature status
    #define PRINT_FEATURE(name) \
        oss << "  " << #name << ": " << (features.name ? "YES" : "NO") << "\n"

    PRINT_FEATURE(robustBufferAccess);
    PRINT_FEATURE(fullDrawIndexUint32);
    PRINT_FEATURE(imageCubeArray);
    PRINT_FEATURE(independentBlend);
    PRINT_FEATURE(geometryShader);
    PRINT_FEATURE(tessellationShader);
    PRINT_FEATURE(sampleRateShading);
    PRINT_FEATURE(dualSrcBlend);
    PRINT_FEATURE(logicOp);
    PRINT_FEATURE(multiDrawIndirect);
    PRINT_FEATURE(drawIndirectFirstInstance);
    PRINT_FEATURE(depthClamp);
    PRINT_FEATURE(depthBiasClamp);
    PRINT_FEATURE(fillModeNonSolid);
    PRINT_FEATURE(depthBounds);
    PRINT_FEATURE(wideLines);
    PRINT_FEATURE(largePoints);
    PRINT_FEATURE(alphaToOne);
    PRINT_FEATURE(multiViewport);
    PRINT_FEATURE(samplerAnisotropy);
    PRINT_FEATURE(textureCompressionETC2);
    PRINT_FEATURE(textureCompressionASTC_LDR);
    PRINT_FEATURE(textureCompressionBC);
    PRINT_FEATURE(occlusionQueryPrecise);
    PRINT_FEATURE(pipelineStatisticsQuery);
    PRINT_FEATURE(vertexPipelineStoresAndAtomics);
    PRINT_FEATURE(fragmentStoresAndAtomics);
    PRINT_FEATURE(shaderTessellationAndGeometryPointSize);
    PRINT_FEATURE(shaderImageGatherExtended);
    PRINT_FEATURE(shaderStorageImageExtendedFormats);
    PRINT_FEATURE(shaderStorageImageMultisample);
    PRINT_FEATURE(shaderStorageImageReadWithoutFormat);
    PRINT_FEATURE(shaderStorageImageWriteWithoutFormat);
    PRINT_FEATURE(shaderUniformBufferArrayDynamicIndexing);
    PRINT_FEATURE(shaderSampledImageArrayDynamicIndexing);
    PRINT_FEATURE(shaderStorageBufferArrayDynamicIndexing);
    PRINT_FEATURE(shaderStorageImageArrayDynamicIndexing);
    PRINT_FEATURE(shaderClipDistance);
    PRINT_FEATURE(shaderCullDistance);
    PRINT_FEATURE(shaderFloat64);
    PRINT_FEATURE(shaderInt64);
    PRINT_FEATURE(shaderInt16);
    PRINT_FEATURE(shaderResourceResidency);
    PRINT_FEATURE(shaderResourceMinLod);
    PRINT_FEATURE(sparseBinding);
    PRINT_FEATURE(sparseResidencyBuffer);
    PRINT_FEATURE(sparseResidencyImage2D);
    PRINT_FEATURE(sparseResidencyImage3D);
    PRINT_FEATURE(sparseResidency2Samples);
    PRINT_FEATURE(sparseResidency4Samples);
    PRINT_FEATURE(sparseResidency8Samples);
    PRINT_FEATURE(sparseResidency16Samples);
    PRINT_FEATURE(sparseResidencyAliased);
    PRINT_FEATURE(variableMultisampleRate);
    PRINT_FEATURE(inheritedQueries);

    #undef PRINT_FEATURE

    return oss.str();
}

std::string formatSurfaceCapabilities(const vk::SurfaceCapabilitiesKHR& caps) {
    std::ostringstream oss;
    oss << "\n";
    oss << "  minImageCount: " << caps.minImageCount << "\n";
    oss << "  maxImageCount: " << (caps.maxImageCount == 0 ? "unlimited" : std::to_string(caps.maxImageCount)) << "\n";
    oss << "  currentExtent: " << caps.currentExtent.width << "x" << caps.currentExtent.height << "\n";
    oss << "  minImageExtent: " << caps.minImageExtent.width << "x" << caps.minImageExtent.height << "\n";
    oss << "  maxImageExtent: " << caps.maxImageExtent.width << "x" << caps.maxImageExtent.height << "\n";
    oss << "  maxImageArrayLayers: " << caps.maxImageArrayLayers << "\n";
    oss << "  supportedTransforms: " << vk::to_string(caps.supportedTransforms) << "\n";
    oss << "  currentTransform: " << vk::to_string(caps.currentTransform) << "\n";
    oss << "  supportedCompositeAlpha: " << vk::to_string(caps.supportedCompositeAlpha) << "\n";
    oss << "  supportedUsageFlags: " << vk::to_string(caps.supportedUsageFlags);
    return oss.str();
}
} // anonymous namespace

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
                                                const std::vector<const char *> &reqExtensions)
{
    auto availableExtensions = device.enumerateDeviceExtensionProperties();

    const bool supportsAllRequiredExtensions = std::ranges::all_of(
            reqExtensions, [&availableExtensions](const auto &requiredDeviceExtension) {
                return std::ranges::any_of(
                        availableExtensions,
                        [&requiredDeviceExtension](const auto &availableDeviceExtension) {
                            return strcmp(availableDeviceExtension.extensionName,
                                requiredDeviceExtension) == 0;
                        });
            });

    return supportsAllRequiredExtensions;
}

bool DeviceBuilder::isDeviceSuitable(vk::PhysicalDevice device, vk::SurfaceKHR surface,
                                     const std::vector<const char *> &extensions)
{
    auto indices = findQueueFamilies(device, surface);
    bool extensionsSupported = checkDeviceExtensionSupport(device, extensions);

    bool swapchainAdequate = false;
    if (extensionsSupported) {
        auto formats = device.getSurfaceFormatsKHR(surface);
        auto presentModes = device.getSurfacePresentModesKHR(surface);
        swapchainAdequate = !formats.empty() && !presentModes.empty();
    }

    // TODO: this should be passed from the creator, just like with extensions
    auto features = device.getFeatures2<vk::PhysicalDeviceFeatures2,
                                         vk::PhysicalDeviceVulkan13Features,
                                         vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

    bool supportsRequiredFeatures =
        features.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering
        && features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

    return indices.isComplete() && extensionsSupported && swapchainAdequate
            && supportsRequiredFeatures;
}

void DeviceBuilder::build(VkCtx& ctx) {
    auto physicalDevices = ctx.instance.enumeratePhysicalDevices();
    if (physicalDevices.empty()) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support");
    }

    for (const auto& device : physicalDevices) {
        if (isDeviceSuitable(*device, *ctx.surface, m_deviceExtensions)) {
            // TODO add device selection when we have more than one GPU
            ctx.physicalDevice = vk::raii::PhysicalDevice(device);
            break;
        }
    }

    if (!*ctx.physicalDevice) {
        throw std::runtime_error("Failed to find a suitable GPU");
    }

    const auto& props = ctx.physicalDevice.getProperties();
    qCInfo(logger()) << "Selected GPU:" << props.deviceName.data();

    const auto devVkVersion = props.apiVersion;
    qCInfo(logger())
        << "Supported Vulkan version:"
        << QString("%1.%2.%3")
            .arg(vk::versionMajor(devVkVersion))
            .arg(vk::versionMinor(devVkVersion))
            .arg(vk::versionPatch(devVkVersion));

    const auto& features = ctx.physicalDevice.getFeatures();
    qCInfo(logger())
        << "Device features:"
        << formatDeviceFeatures(features).c_str();

    auto capabilities = ctx.physicalDevice.getSurfaceCapabilitiesKHR(*ctx.surface);
    qCInfo(logger())
        << "Surface capabilities:"
        << formatSurfaceCapabilities(capabilities).c_str();

    ctx.queueFamilyIndices = findQueueFamilies(ctx.physicalDevice, *ctx.surface);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        ctx.queueFamilyIndices.graphicsFamily.value(),
        ctx.queueFamilyIndices.presentFamily.value()
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo{
            .queueFamilyIndex = queueFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::StructureChain<
        vk::PhysicalDeviceFeatures2,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
    > featureChain{
        vk::PhysicalDeviceFeatures2{},
        vk::PhysicalDeviceVulkan13Features{
            .dynamicRendering = vk::True
        },
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT{
            .extendedDynamicState = vk::True
        }
    };

    vk::DeviceCreateInfo createInfo{
        .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size()),
        .ppEnabledExtensionNames = m_deviceExtensions.data()
    };

    ctx.device = vk::raii::Device(ctx.physicalDevice, createInfo);
    ctx.graphicsQueue = ctx.device.getQueue(ctx.queueFamilyIndices.graphicsFamily.value(), 0);
    ctx.presentQueue = ctx.device.getQueue(ctx.queueFamilyIndices.presentFamily.value(), 0);

    qCInfo(logger()) << "Logical device created";
}

} // namespace nuff::renderer

