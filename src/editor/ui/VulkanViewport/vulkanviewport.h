#ifndef VULKANVIEWPORT_H
#define VULKANVIEWPORT_H

#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickWindow>
#include <vulkan/vulkan.h>

#include "editor/editor_app.h"

class VulkanViewport : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(nuff::editor::EditorApp* engine READ engine WRITE setEngine NOTIFY engineChanged)

public:
    VulkanViewport();

    nuff::editor::EditorApp* engine() const { return m_engine; }
    void setEngine(nuff::editor::EditorApp* engine);

signals:
    void engineChanged();

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;

private slots:
    void cleanup();
    void handleWindowChanged(QQuickWindow* win);
    void onSceneGraphInitialized();
    void tryInitializeEngine();
    void onFrameSwapped();

private:
    void importImage();
    void cleanupImportedImage();

    nuff::editor::EditorApp* m_engine = nullptr;
    bool m_initialized = false;
    bool m_needsImport = false;

    uint32_t m_vendorId = 0;
    uint32_t m_deviceId = 0;

    VkDevice m_qtDevice = VK_NULL_HANDLE;
    VkImage m_importedImage = VK_NULL_HANDLE;
    VkDeviceMemory m_importedMemory = VK_NULL_HANDLE;
    uint32_t m_importedWidth = 0;
    uint32_t m_importedHeight = 0;
};

#endif // VULKANVIEWPORT_H
