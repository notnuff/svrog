#include <QGuiApplication>
#include <QtQuick/QQuickView>
#include <QQmlApplicationEngine>
#include <qqmlcontext.h>
#include <ranges>
#include <iostream>

#include "bridge/vulkandemorenderer.h"

int main(int argc, char **argv)
{

    QGuiApplication app(argc, argv);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Vulkan);

    QQmlApplicationEngine engine;
    const auto demo = std::make_unique<VulkanDemoRenderer>();

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.rootContext()->setContextProperty("demoRenderer", demo.get());
    qDebug() << "Import paths:";
    for (const auto &path : engine.importPathList())
        qDebug() << "  " << path;

    engine.addImportPath("./ui");
    engine.loadFromModule("MainWindow", "Main");
    return app.exec();
}
