#include <QGuiApplication>
#include <QVulkanInstance>
#include <QtQuick/QQuickView>
#include <QQmlApplicationEngine>
#include <qqmlcontext.h>
#include <ranges>
#include <iostream>

#include "bridge/vulkandemorenderer.h"
#include "logging/colored_logger.h"

int main(int argc, char **argv)
{
    // Install custom colored message handler
    nuff::logging::installColoredLogger();

    QGuiApplication app(argc, argv);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Vulkan);

    QQmlApplicationEngine engine;
    const auto demo = std::make_unique<VulkanDemoRenderer>();

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    QVulkanInstance instance;
    engine.rootContext()->setContextProperty("demoRenderer", demo.get());

    engine.addImportPath("./ui");
    engine.loadFromModule("MainWindow", "Main");
    return app.exec();
}
