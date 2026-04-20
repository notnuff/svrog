import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Controls
import VulkanViewport

ApplicationWindow {
    id: root

    visible: false  // main.cpp shows window after graphics configuration
    width: 828
    height: 512

    color: "black"
    flags: Qt.Window | Qt.FramelessWindowHint

    header: Rectangle {
        id: header
        width: root.width
        height: 40
        color: "#222"

        DragHandler {
            target: null
            onActiveChanged: if (active) root.startSystemMove()
        }

        RowLayout {
            anchors.fill: parent
            anchors.margins: 5

            Text {
                text: "svrog 0.0.0"
                color: "white"
                Layout.leftMargin: 10
            }

            Item { Layout.fillWidth: true }

            Row {
                spacing: 10
                Text {
                    text: "—"; color: "white"; font.pixelSize: 20
                    TapHandler { onTapped: root.showMinimized() }
                }
                Text {
                    text: "✕"; color: "white"; font.pixelSize: 20
                    Layout.rightMargin: 10
                    TapHandler { onTapped: root.close() }
                }
            }
        }
    }

    Item {
        id: container
        anchors.fill: parent
        anchors.margins: 5

        // --- Main Content Area ---
        RowLayout {
            anchors.fill: parent

            spacing: 10

            Rectangle {
                color: "transparent"

                radius: 10
                border.width: 1
                border.color: "#333"

                implicitWidth: 2 * parent.width / 3
                Layout.fillHeight: true

                VulkanViewport {
                    id: viewport
                    engine: renderEngine
                    anchors.fill: parent
                }
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.fillWidth: true

                color: "#111"

                radius: 10
                border.width: 1
                border.color: "#333"

                Text {
                    width: parent.width
                    padding: 10
                    text: "here will be right panel and stuff"
                    font.pixelSize: 56
                    wrapMode: Text.Wrap
                    color: "white"
                }
            }
        }
    }
}