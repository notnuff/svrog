#include "vulkanviewport.h"
#include <QtCore/QRunnable>

VulkanViewport::VulkanViewport()
{
    connect(this, &QQuickItem::windowChanged, this, &VulkanViewport::handleWindowChanged);
}

void VulkanViewport::setRenderer(IVulkanRenderer *renderer)
{
    if (m_renderer == renderer)
        return;

    m_renderer = renderer;
    emit rendererChanged();
}

void VulkanViewport::handleWindowChanged(QQuickWindow *win)
{
    if (win) {
        connect(win, &QQuickWindow::beforeSynchronizing, this, &VulkanViewport::sync, Qt::DirectConnection);
        connect(win, &QQuickWindow::sceneGraphInvalidated, this, &VulkanViewport::cleanup, Qt::DirectConnection);
        win->setColor(Qt::black);
    }
}

void VulkanViewport::cleanup()
{
    if (m_renderer) {
        delete m_renderer;
        m_renderer = nullptr;
    }
}

class CleanupJob : public QRunnable
{
public:
    CleanupJob(IVulkanRenderer *renderer) : m_renderer(renderer) { }
    void run() override { delete m_renderer; }
private:
    IVulkanRenderer *m_renderer;
};

void VulkanViewport::releaseResources()
{
    if (m_renderer) {
        window()->scheduleRenderJob(new CleanupJob(m_renderer), QQuickWindow::BeforeSynchronizingStage);
        m_renderer = nullptr;
    }
}

void VulkanViewport::sync()
{
    if (!m_renderer)
        return;

    connect(window(), &QQuickWindow::beforeRendering, m_renderer, &IVulkanRenderer::frameStart, Qt::DirectConnection);
    connect(window(), &QQuickWindow::beforeRenderPassRecording, m_renderer, &IVulkanRenderer::mainPassRecordingStart, Qt::DirectConnection);

    auto viewportBegin = mapToScene(position());
    m_renderer->setViewport({
        viewportBegin.x(),
        viewportBegin.y(),
        width(),
        height()
    });
    m_renderer->setWindow(window());
}

#include "vulkanviewport.moc"
