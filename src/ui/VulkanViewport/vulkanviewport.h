#ifndef VULKANVIEWPORT_H
#define VULKANVIEWPORT_H

#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickWindow>
#include <vulkan/vulkan.h>

#include "bridge/render_engine.h"

class VulkanViewport : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(nuff::bridge::RenderEngine* engine READ engine WRITE setEngine NOTIFY engineChanged)

public:
    VulkanViewport();

    nuff::bridge::RenderEngine* engine() const { return m_engine; }
    void setEngine(nuff::bridge::RenderEngine* engine);

signals:
    void engineChanged();

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;

private slots:
    void sync();
    void cleanup();
    void handleWindowChanged(QQuickWindow* win);
    void onBeforeRendering();

private:
    void initializeEngine();
    void importImage();
    void cleanupImportedImage();

    nuff::bridge::RenderEngine* m_engine = nullptr;
    bool m_initialized = false;
    bool m_needsImport = false;

    VkDevice m_qtDevice = VK_NULL_HANDLE;
    VkImage m_importedImage = VK_NULL_HANDLE;
    VkDeviceMemory m_importedMemory = VK_NULL_HANDLE;
    uint32_t m_importedWidth = 0;
    uint32_t m_importedHeight = 0;
};

#endif // VULKANVIEWPORT_H
