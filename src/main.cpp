#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQuickGraphicsConfiguration>
#include <qqmlcontext.h>

#include <vulkan/vulkan.h>

#include "bridge/render_engine.h"
#include "logging/colored_logger.h"

int main(int argc, char **argv)
{
    nuff::logging::installColoredLogger();

    QGuiApplication app(argc, argv);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Vulkan);

    auto renderEngine = std::make_unique<nuff::bridge::RenderEngine>();

    QQmlApplicationEngine qmlEngine;

    QObject::connect(&qmlEngine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    qmlEngine.rootContext()->setContextProperty("renderEngine", renderEngine.get());

    qmlEngine.addImportPath("./ui");
    qmlEngine.loadFromModule("MainWindow", "Main");

    for (auto* obj : qmlEngine.rootObjects()) {
        if (auto* win = qobject_cast<QQuickWindow*>(obj)) {
            QQuickGraphicsConfiguration config;
            config.setDeviceExtensions({VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME});
            win->setGraphicsConfiguration(config);
            win->show();
        }
    }

    return app.exec();
}
