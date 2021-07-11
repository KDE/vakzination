/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.2
import org.kde.kirigami 2.15 as Kirigami
import org.kde.prison 1.0 as Prison

Column {
    id: root

    required property var certificate

    height: childredRect.height

    Kirigami.Heading {

        anchors.left: barcode.left

        text: i18n("%1 (%2/%3)", certificate.disease, certificate.dose, certificate.totalDoses)
    }

    Controls.Label {
        anchors.left: barcode.left
        text: certificate.name
    }

    Prison.Barcode {
        id: barcode

        anchors.horizontalCenter: parent.horizontalCenter

        barcodeType: Prison.Barcode.QRCode
        content: certificate.rawData
    }

    Controls.Button {
        anchors.left: barcode.left

        text: i18n("Details")
        onClicked: pageStack.push(Qt.resolvedUrl("CertificateDetailsPage.qml"), {cert: root.certificate})
    }

}
