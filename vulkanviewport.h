#ifndef VULKANVIEWPORT_H
#define VULKANVIEWPORT_H

#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickWindow>
#include "vulkanrenderer.h"

class VulkanViewport : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(IVulkanRenderer* renderer READ renderer WRITE setRenderer NOTIFY rendererChanged)

public:
    VulkanViewport();

    IVulkanRenderer* renderer() const { return m_renderer; }
    void setRenderer(IVulkanRenderer* renderer);

signals:
    void rendererChanged();

public slots:
    void sync();
    void cleanup();

private slots:
    void handleWindowChanged(QQuickWindow *win);

private:
    void releaseResources() override;

    IVulkanRenderer *m_renderer = nullptr;
};

#endif // VULKANVIEWPORT_H
