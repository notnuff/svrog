#ifndef SQUIRCLERENDERER_H
#define SQUIRCLERENDERER_H

#include "vulkanrenderer.h"

#include <QVulkanDeviceFunctions>
#include <QVulkanFunctions>

class VulkanDemoRenderer : public IVulkanRenderer
{
    Q_OBJECT
public:
    VulkanDemoRenderer();
    ~VulkanDemoRenderer() override;

    void setT(qreal t) { m_t = t; }

public slots:
    void frameStart() override;
    void mainPassRecordingStart() override;

public:
    void setViewport(const QRectF &viewport) override { m_viewport = viewport; }
    void setWindow(QQuickWindow *window) override { m_window = window; }

private:
    enum Stage {
        VertexStage,
        FragmentStage
    };
    void prepareShader(Stage stage);
    void init(int framesInFlight);

    QRectF m_viewport;
    qreal m_t = 0;
    QQuickWindow *m_window = nullptr;

    QByteArray m_vert;
    QByteArray m_frag;

    bool m_initialized = false;
    VkPhysicalDevice m_physDev = VK_NULL_HANDLE;
    VkDevice m_dev = VK_NULL_HANDLE;
    QVulkanDeviceFunctions *m_devFuncs = nullptr;
    QVulkanFunctions *m_funcs = nullptr;

    VkBuffer m_vbuf = VK_NULL_HANDLE;
    VkDeviceMemory m_vbufMem = VK_NULL_HANDLE;
    VkBuffer m_ubuf = VK_NULL_HANDLE;
    VkDeviceMemory m_ubufMem = VK_NULL_HANDLE;
    VkDeviceSize m_allocPerUbuf = 0;

    VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;

    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_resLayout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;

    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet m_ubufDescriptor = VK_NULL_HANDLE;
};

#endif // SQUIRCLERENDERER_H
