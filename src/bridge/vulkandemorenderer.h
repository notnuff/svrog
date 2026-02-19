#ifndef SQUIRCLERENDERER_H
#define SQUIRCLERENDERER_H

#include <QObject>
#include "./ivulkanrenderer.h"

class VulkanDemoRenderer : public IVulkanRenderer {
    Q_OBJECT

public:
    VulkanDemoRenderer();

    ~VulkanDemoRenderer() override;

public slots:
    void frameStart() override;

    void mainPassRecordingStart() override;

public:
    void setViewport(const QRectF &viewport) override;

    void setWindow(QQuickWindow *window) override;
};

#endif // SQUIRCLERENDERER_H
