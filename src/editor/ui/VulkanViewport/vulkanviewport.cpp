#include "vulkanviewport.h"

#include <unistd.h>

#include <QLoggingCategory>
#include <QSGSimpleTextureNode>
#include <QSGRendererInterface>

#include "editor/editor_app.h"

namespace L {
Q_LOGGING_CATEGORY(vkViewport, "nuff.ui.vulkan_viewport")
}

VulkanViewport::VulkanViewport()
{
    setFlag(ItemHasContents, true);
    connect(this, &QQuickItem::windowChanged, this, &VulkanViewport::handleWindowChanged);
}

void VulkanViewport::setEngine(nuff::editor::EditorApp* engine)
{
    if (m_engine == engine) return;
    m_engine = engine;
    emit engineChanged();
    QMetaObject::invokeMethod(this, &VulkanViewport::tryInitializeEngine,
                              Qt::QueuedConnection);
}

void VulkanViewport::handleWindowChanged(QQuickWindow* win)
{
    if (win) {
        connect(win, &QQuickWindow::sceneGraphInitialized,
                this, &VulkanViewport::onSceneGraphInitialized, Qt::DirectConnection);
        connect(win, &QQuickWindow::sceneGraphInvalidated,
                this, &VulkanViewport::cleanup, Qt::DirectConnection);
        connect(win, &QQuickWindow::frameSwapped,
                this, &VulkanViewport::onFrameSwapped, Qt::QueuedConnection);
        win->setColor(Qt::black);
    }
}

void VulkanViewport::onSceneGraphInitialized()
{
    auto* rif = window()->rendererInterface();
    auto* physDev = static_cast<VkPhysicalDevice*>(
        rif->getResource(window(), QSGRendererInterface::PhysicalDeviceResource));
    if (!physDev) return;

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(*physDev, &props);

    m_vendorId = props.vendorID;
    m_deviceId = props.deviceID;
    m_qtDevice = *static_cast<VkDevice*>(
        rif->getResource(window(), QSGRendererInterface::DeviceResource));

    qCInfo(L::vkViewport) << "Scene graph ready, Qt GPU:" << props.deviceName;

    QMetaObject::invokeMethod(this, &VulkanViewport::tryInitializeEngine,
                              Qt::QueuedConnection);
}

void VulkanViewport::tryInitializeEngine()
{
    if (m_initialized) return;
    if (!m_engine || !window()) return;
    if (m_qtDevice == VK_NULL_HANDLE) return;

    const auto w = static_cast<uint32_t>(std::max(1.0, width()));
    const auto h = static_cast<uint32_t>(std::max(1.0, height()));

    m_engine->initialize(m_vendorId, m_deviceId, w, h);
    if (!m_engine->isInitialized()) {
        qCWarning(L::vkViewport) << "Engine failed to initialize";
        return;
    }

    m_engine->renderFrame();
    m_needsImport = true;
    m_initialized = true;
    update();
}

void VulkanViewport::onFrameSwapped()
{
    if (!m_initialized || !m_engine || !m_engine->isInitialized()) return;
    m_engine->renderFrame();
    update();
}

void VulkanViewport::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    if (!m_initialized || !m_engine || !m_engine->isInitialized()) return;

    const auto w = static_cast<uint32_t>(std::max(1.0, newGeometry.width()));
    const auto h = static_cast<uint32_t>(std::max(1.0, newGeometry.height()));
    const auto extent = m_engine->imageExtent();
    if (extent.width == w && extent.height == h) return;

    m_engine->resize(w, h);
    m_needsImport = true;
    update();
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
