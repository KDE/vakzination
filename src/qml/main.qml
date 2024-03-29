/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
*/

import QtCore
import QtQuick 2.15
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.2
import QtQuick.Dialogs
import org.kde.kirigami 2.15 as Kirigami

import org.kde.vakzination 1.0

Kirigami.ApplicationWindow {
    id: root

    title: i18n("Your Certificates")

    globalDrawer: Kirigami.GlobalDrawer {
        isMenu: true
        actions: [
            Kirigami.Action {
                text: i18n("Import from file")
                icon.name: "folder-open"
                onTriggered: fileDialog.open()
            },
            Kirigami.Action {
                text: i18n("Import from clipboard")
                icon.name: "edit-paste"
                onTriggered: CertificatesModel.importCertificateFromClipboard()
            },
            Kirigami.Action {
                text: i18n("Scan QR code")
                icon.name: "view-barcode-qr"
                onTriggered: applicationWindow().pageStack.push(scanBarcodePage);
            }
        ]
    }

    minimumWidth: Kirigami.Units.gridUnit * 20
    minimumHeight: Kirigami.Units.gridUnit * 20

    pageStack.globalToolBar.toolbarActionAlignment: Qt.AlignLeft

    pageStack.initialPage: Qt.resolvedUrl("CertificatesPage.qml")

    FileDialog {
        id: fileDialog
        currentFolder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        onAccepted: CertificatesModel.importCertificate(selectedFile)
    }

    Component {
        id: scanBarcodePage
        ScanBarcodePage {}
    }

    DropArea {
        onEntered: {
            if (!drag.hasUrls) {
                drag.accepted = false;
            }
        }
        onDropped: {
            for (const i in drop.urls) {
                CertificatesModel.importCertificate(drop.urls[i]);
            }
        }
        anchors.fill: parent
    }
}
