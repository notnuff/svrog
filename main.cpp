#include <QGuiApplication>
#include <QtQuick/QQuickView>
#include <QQmlApplicationEngine>
#include <qqmlcontext.h>

#include "vulkandemorenderer.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Vulkan);

    QQmlApplicationEngine engine;
    auto demo = std::make_unique<VulkanDemoRenderer>();

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.rootContext()->setContextProperty("demoRenderer", demo.get());
    engine.load(QUrl("qrc:///scenegraph/demo/main.qml"));

    return app.exec();
}
