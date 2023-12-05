/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import QtMultimedia
import org.kde.kirigami 2.17 as Kirigami
import org.kde.prison.scanner 1.0 as Prison
import org.kde.vakzination 1.0

Kirigami.Page {
    id: root
    title: i18n("Scan QR code")

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    actions: [
        Kirigami.Action {
            icon.name: checked ? "flashlight-off" : "flashlight-on"
            text: i18n("Light")
            checkable: true
            checked: camera.torchMode == Camera.TorchOn
            visible: camera.isTorchModeSupported
            onTriggered: camera.torchMode = (camera.torchMode == Camera.TorchOn ? Camera.TorchOff : Camera.TorchOn)
        }
    ]

    VideoOutput {
        id: viewFinder
        anchors.fill: parent
        fillMode: VideoOutput.PreserveAspectCrop
    }

    Prison.VideoScanner {
        id: scanner
        formats: Prison.Format.QRCode | Prison.Format.Aztec | Prison.Format.DataMatrix | Prison.Format.PDF417
        videoSink: viewFinder.videoSink
        onResultContentChanged: {
            if (result.hasText && CertificatesModel.importCertificateFromText(result.text)) {
                applicationWindow().pageStack.goBack();
            }
            if (result.hasBinaryData && CertificatesModel.importCertificateFromData(result.binaryData)) {
                applicationWindow().pageStack.goBack();
            }
        }
    }

    CaptureSession {
        id: captureSession
        camera: Camera {
            id: camera
            active: true
        }
        videoOutput: viewFinder
    }

    Rectangle {
        border {
            color: Kirigami.Theme.focusColor
            width: 2
        }
        color: Qt.rgba(Kirigami.Theme.focusColor.r, Kirigami.Theme.focusColor.g, Kirigami.Theme.focusColor.b, 0.2);
        radius: Kirigami.Units.smallSpacing

        x: viewFinder.contentRect.x + scanner.result.boundingRect.x / viewFinder.sourceRect.width * viewFinder.contentRect.width
        y: viewFinder.contentRect.y + scanner.result.boundingRect.y / viewFinder.sourceRect.height * viewFinder.contentRect.height
        width: scanner.result.boundingRect.width / viewFinder.sourceRect.width * viewFinder.contentRect.width
        height: scanner.result.boundingRect.height / viewFinder.sourceRect.height * viewFinder.contentRect.height
    }

    Kirigami.PlaceholderMessage {
        anchors.fill: parent
        text: i18n("No camera available.")
        visible: camera.error != Camera.NoError
    }
}
