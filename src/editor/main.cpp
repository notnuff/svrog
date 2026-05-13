#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQuickGraphicsConfiguration>
#include <qqmlcontext.h>

#include <vulkan/vulkan.h>

#include "editor_app.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Vulkan);

    auto editorApp = std::make_unique<nuff::editor::EditorApp>();

    QQmlApplicationEngine qmlEngine;

    QObject::connect(&qmlEngine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    qmlEngine.rootContext()->setContextProperty("editorApp", editorApp.get());

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
