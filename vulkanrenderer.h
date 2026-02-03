#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include <QObject>
#include <QRectF>

class QQuickWindow;

class IVulkanRenderer : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    virtual ~IVulkanRenderer() = default;

public slots:
    // These will be called by the viewport item on the render thread.
    virtual void frameStart() = 0;
    virtual void mainPassRecordingStart() = 0;

public:
    // These are for passing data from the viewport item.
    virtual void setViewport(const QRectF &viewport) = 0;
    virtual void setWindow(QQuickWindow *window) = 0;
};

#endif // VULKANRENDERER_H
