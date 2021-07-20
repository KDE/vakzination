/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.2
import Qt.labs.platform 1.1
import org.kde.kirigami 2.15 as Kirigami

import org.kde.vakzination 1.0

Kirigami.ScrollablePage {

    title: Kirigami.Settings.isMobile ? i18n("Your certificates") : ""

    property var importFileAction: Kirigami.Action {
        text: i18n("Import from file")
        icon.name: "folder-open"
        onTriggered: {
            fileDialog.open()
        }
    }

    property var importClipboardAction: Kirigami.Action {
        text: i18n("Import from clipboard")
        icon.name: "edit-paste"
        onTriggered: {
            CertificatesModel.importCertificateFromClipboard()
        }
    }

    FileDialog {
        id: fileDialog
        folder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)

        onAccepted: {
            CertificatesModel.importCertificate(currentFile)
        }
    }

    header: Kirigami.InlineMessage {
        id: importError

        property string error

        type: Kirigami.MessageType.Error
        text: i18n("Certificate could not be imported: %1", error)
        showCloseButton: true
    }

    footer: Controls.ToolBar {

        visible: Kirigami.Settings.isMobile
        height: visible ? implicitHeight : 0

        Kirigami.ActionToolBar {
            anchors.fill: parent

            actions: [importFileAction, importClipboardAction]
        }
    }

    actions.main: !Kirigami.Settings.isMobile ? importFileAction : undefined
    actions.right: !Kirigami.Settings.isMobile ? importClipboardAction : undefined

    Connections {
        target: CertificatesModel

        function onImportError(error) {
            importError.error = error
            importError.visible = true
        }
    }

    ListView {
        id: listView
        model: CertificatesModel

        delegate: CertificateDelegate {
            width: parent.width
            showSeparator: index  !== listView.count - 1
        }

        Kirigami.PlaceholderMessage {
            text: i18n("No certificates saved")
            visible: listView.count === 0
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
        }
    }
}
