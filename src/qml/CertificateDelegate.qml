/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
*/

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.prison as Prison
import org.kde.khealthcertificate as KHC

Controls.Page {
    id: root
    required property var certificate

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    header: ColumnLayout {
        Kirigami.Heading {

            Layout.alignment: Qt.AlignHCenter

            text: {
                if (root.certificate.type === KHC.HealthCertificate.Vaccination) {
                    return i18n("Vaccination %1 (%2/%3)", certificate.disease ? certificate.disease : certificate.vaccine, certificate.dose, certificate.totalDoses)
                }

                if (root.certificate.type === KHC.HealthCertificate.Test) {
                    return i18n("Test %1", certificate.disease, certificate.dose)
                }

                if (root.certificate.type === KHC.HealthCertificate.Recovery) {
                    return i18n("Recovery %1", certificate.disease)
                }
            }
        }

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: certificate.name
        }
    }

    contentItem: Prison.Barcode {
        id: barcode

        Layout.maximumWidth: implicitWidth
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter

        barcodeType: Prison.Barcode.QRCode
        content: certificate.rawData
    }
}
