#include "vulkanviewport.h"

#include <unistd.h>

#include <QLoggingCategory>
#include <QSGSimpleTextureNode>
#include <QSGRendererInterface>

#include "bridge/render_engine.h"

namespace L {
Q_LOGGING_CATEGORY(vkViewport, "nuff.ui.vulkan_viewport")
}

VulkanViewport::VulkanViewport()
{
    setFlag(ItemHasContents, true);
    connect(this, &QQuickItem::windowChanged, this, &VulkanViewport::handleWindowChanged);
}

void VulkanViewport::setEngine(nuff::bridge::RenderEngine* engine)
{
    if (m_engine == engine) return;
    m_engine = engine;
    emit engineChanged();
}

void VulkanViewport::handleWindowChanged(QQuickWindow* win)
{
    if (win) {
        connect(win, &QQuickWindow::beforeSynchronizing,
                this, &VulkanViewport::sync, Qt::DirectConnection);
        connect(win, &QQuickWindow::beforeRendering,
                this, &VulkanViewport::onBeforeRendering, Qt::DirectConnection);
        connect(win, &QQuickWindow::sceneGraphInvalidated,
                this, &VulkanViewport::cleanup, Qt::DirectConnection);
        win->setColor(Qt::black);
    }
}

void VulkanViewport::initializeEngine()
{
    auto* rif = window()->rendererInterface();
    auto* physDev = static_cast<VkPhysicalDevice*>(
        rif->getResource(window(), QSGRendererInterface::PhysicalDeviceResource));
    if (!physDev) return;

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(*physDev, &props);

    int w = std::max(1, static_cast<int>(width()));
    int h = std::max(1, static_cast<int>(height()));

    m_engine->initialize(props.vendorID, props.deviceID,
                         static_cast<uint32_t>(w), static_cast<uint32_t>(h));

    m_qtDevice = *static_cast<VkDevice*>(
        rif->getResource(window(), QSGRendererInterface::DeviceResource));

    m_needsImport = true;
    m_initialized = true;
    qCInfo(L::vkViewport) << "Engine initialized, Qt GPU:" << props.deviceName;
}

void VulkanViewport::sync()
{
    if (!m_engine || !window()) return;

    if (!m_initialized) {
        initializeEngine();
    }

    if (!m_engine->isInitialized()) return;

    auto w = static_cast<uint32_t>(std::max(1.0, width()));
    auto h = static_cast<uint32_t>(std::max(1.0, height()));
    auto extent = m_engine->imageExtent();
    if (extent.width != w || extent.height != h) {
        m_engine->resize(w, h);
        m_needsImport = true;
    }

    update();
}

void VulkanViewport::onBeforeRendering()
{
    if (!m_engine || !m_engine->isInitialized()) return;
    m_engine->renderFrame();
}

void VulkanViewport::importImage()
{
    if (!m_engine || !m_engine->isInitialized() || m_qtDevice == VK_NULL_HANDLE) return;

    int fd = m_engine->exportMemoryFd();
    if (fd < 0) {
        qCWarning(L::vkViewport) << "Failed to export memory fd";
        return;
    }

    auto extent = m_engine->imageExtent();
    auto format = static_cast<VkFormat>(m_engine->imageFormat());

    VkExternalMemoryImageCreateInfo extMemInfo{
        .sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
        .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT
    };
    VkImageCreateInfo imageInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = &extMemInfo,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = {extent.width, extent.height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };
    if (vkCreateImage(m_qtDevice, &imageInfo, nullptr, &m_importedImage) != VK_SUCCESS) {
        qCWarning(L::vkViewport) << "Failed to create imported image";
        close(fd);
        return;
    }

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(m_qtDevice, m_importedImage, &memReqs);

    auto* rif = window()->rendererInterface();
    auto* physDev = static_cast<VkPhysicalDevice*>(
        rif->getResource(window(), QSGRendererInterface::PhysicalDeviceResource));

    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(*physDev, &memProps);

    uint32_t memTypeIndex = UINT32_MAX;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if ((memReqs.memoryTypeBits & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
            memTypeIndex = i;
            break;
        }
    }
    if (memTypeIndex == UINT32_MAX) {
        for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
            if (memReqs.memoryTypeBits & (1 << i)) {
                memTypeIndex = i;
                break;
            }
        }
    }
    if (memTypeIndex == UINT32_MAX) {
        qCWarning(L::vkViewport) << "Failed to find suitable memory type for import";
        vkDestroyImage(m_qtDevice, m_importedImage, nullptr);
        m_importedImage = VK_NULL_HANDLE;
        close(fd);
        return;
    }

    VkImportMemoryFdInfoKHR importInfo{
        .sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR,
        .handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT,
        .fd = fd
    };
    VkMemoryAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = &importInfo,
        .allocationSize = m_engine->memorySize(),
        .memoryTypeIndex = memTypeIndex
    };

    if (vkAllocateMemory(m_qtDevice, &allocInfo, nullptr, &m_importedMemory) != VK_SUCCESS) {
        qCWarning(L::vkViewport) << "Failed to import memory";
        vkDestroyImage(m_qtDevice, m_importedImage, nullptr);
        m_importedImage = VK_NULL_HANDLE;
        return;
    }
    vkBindImageMemory(m_qtDevice, m_importedImage, m_importedMemory, 0);

    m_importedWidth = extent.width;
    m_importedHeight = extent.height;
    qCInfo(L::vkViewport) << "Imported engine image:" << m_importedWidth << "x" << m_importedHeight;
}

void VulkanViewport::cleanupImportedImage()
{
    if (m_importedImage != VK_NULL_HANDLE) {
        vkDestroyImage(m_qtDevice, m_importedImage, nullptr);
        m_importedImage = VK_NULL_HANDLE;
    }
    if (m_importedMemory != VK_NULL_HANDLE) {
        vkFreeMemory(m_qtDevice, m_importedMemory, nullptr);
        m_importedMemory = VK_NULL_HANDLE;
    }
}

QSGNode* VulkanViewport::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
{
    if (!m_engine || !m_engine->isInitialized()) return oldNode;

    if (m_needsImport) {
        cleanupImportedImage();
        importImage();
        m_needsImport = false;
    }

    if (m_importedImage == VK_NULL_HANDLE) return oldNode;

    auto* node = static_cast<QSGSimpleTextureNode*>(oldNode);
    if (!node) {
        node = new QSGSimpleTextureNode();
    }

    auto* texture = QNativeInterface::QSGVulkanTexture::fromNative(
        m_importedImage,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        window(),
        QSize(static_cast<int>(m_importedWidth), static_cast<int>(m_importedHeight)));

    node->setTexture(texture);
    node->setOwnsTexture(true);
    node->setRect(boundingRect());
    return node;
}

void VulkanViewport::cleanup()
{
    cleanupImportedImage();
}
