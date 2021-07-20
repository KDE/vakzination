/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.2
import org.kde.kirigami 2.15 as Kirigami
import org.kde.prison 1.0 as Prison
import org.kde.khealthcertificate 1.0 as KHC

ColumnLayout {
    id: root

    required property var certificate
    required property var type
    required property var showSeparator
    required property var index

    Kirigami.Heading {

        Layout.alignment: Qt.AlignHCenter

        text: {
            if (root.type === KHC.HealthCertificate.Vaccination) {
                return i18n("Vaccination %1 (%2/%3)", certificate.disease ? certificate.disease : certificate.vaccine, certificate.dose, certificate.totalDoses)
            }

            if (root.type === KHC.HealthCertificate.Test) {
                return i18n("Test %1", certificate.disease, certificate.dose)
            }

            if (root.type === KHC.HealthCertificate.Recovery) {
                return i18n("Recovery %1", certificate.disease)
            }
        }
    }

    Controls.Label {
        Layout.alignment: Qt.AlignHCenter
        text: certificate.name
    }

    Prison.Barcode {
        id: barcode

        Layout.maximumWidth: implicitWidth
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter

        barcodeType: Prison.Barcode.QRCode
        content: certificate.rawData
    }

    Controls.Button {
        Layout.alignment: Qt.AlignHCenter

        text: i18n("Details")
        onClicked: {

            var sheet

            if (root.type === KHC.HealthCertificate.Vaccination) {
                sheet = vaccinationSheet.createObject(this, {certificate: root.certificate})
            }

            if (root.type === KHC.HealthCertificate.Test) {
                sheet = testSheet.createObject(this, {certificate: root.certificate})
            }

            if (root.type === KHC.HealthCertificate.Recovery) {
                sheet = recoverySheet.createObject(this, {certificate: root.certificate})
            }

            sheet.open()
        }
    }

    Item {
        Layout.fillWidth: true
        height: sep.height + Kirigami.Units.largeSpacing

        visible: root.showSeparator

        Kirigami.Separator {
            id: sep
            anchors {
                left: parent.left
                leftMargin: Kirigami.Units.largeSpacing
                right: parent.right
                rightMargin: Kirigami.Units.largeSpacing
                top: parent.top
                topMargin: Kirigami.Units.largeSpacing
            }
        }
    }

    Component {
        id: vaccinationSheet

        VaccinationDetailsSheet {}
    }

    Component {
        id: testSheet

        TestDetailsSheet {}
    }

    Component {
        id: recoverySheet

        RecoveryDetailsSheet{}
    }
}
