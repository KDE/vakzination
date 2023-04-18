/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.2
import Qt.labs.settings 1.0
import org.kde.kirigami 2.19 as Kirigami
import org.kde.khealthcertificate 1.0 as KHC

import org.kde.vakzination 1.0

Kirigami.Page {
    id: root
    title: Kirigami.Settings.isMobile ? i18n("Your certificates") : ""

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    readonly property bool hasValidCertificate: {
        if (certSelector.count === 0 || !certSelector.currentValue) {
            return false;
        }

        switch (certSelector.currentValue.type) {
            case KHC.HealthCertificate.Vaccination:
            case KHC.HealthCertificate.Test:
            case KHC.HealthCertificate.Recovery:
                return true;
            default:
                return false;
        }
    }

    actions {
        contextualActions: [
            Kirigami.Action {
                iconName: "edit-delete"
                text: i18n("Delete")
                onTriggered: deleteWarningDialog.open()
                enabled: root.hasValidCertificate
            }
        ]
    }

    Kirigami.PromptDialog {
        id: deleteWarningDialog
        title: i18n("Delete Certificate")
        subtitle: i18n("Do you really want to delete this certificate?")
        standardButtons: QQC2.Dialog.Cancel
        customFooterActions: [
            Kirigami.Action {
                text: i18n("Delete")
                icon.name: "edit-delete"
                onTriggered: {
                    CertificatesModel.removeRow(certSelector.currentIndex);
                    deleteWarningDialog.close();
                }
            }
        ]
    }

    header: ColumnLayout {
        spacing: 0
        Kirigami.InlineMessage {
            id: importError

            property string error

            type: Kirigami.MessageType.Error
            text: i18n("Certificate could not be imported: %1", error)
            showCloseButton: true
        }

        QQC2.ComboBox {
            id: certSelector
            Layout.margins: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            model: CertificatesModel
            valueRole: "certificate"
            textRole: "display"
            visible: certSelector.count > 0
        }
        Kirigami.Separator {
            Layout.fillWidth: true
            visible: certSelector.visible
        }
    }

    Settings {
        property alias currentCertificateIndex: certSelector.currentIndex
    }

    Component {
        id: vaccinationDetails
        VaccinationDetailsSheet {
            certificate: certSelector.currentValue
        }
    }
    Component {
        id: testDetails
        TestDetailsSheet {
            certificate: certSelector.currentValue
        }
    }
    Component {
        id: recoveryDetails
        RecoveryDetailsSheet {
            certificate: certSelector.currentValue
        }
    }

    Connections {
        target: CertificatesModel

        function onImportError(error) {
            importError.error = error
            importError.visible = true
        }
    }

    QQC2.SwipeView {
        id: swipeView
        visible: hasValidCertificate
        anchors.fill: parent

        CertificateDelegate {
            certificate: certSelector.currentValue
            implicitWidth: parent.width
        }

        Kirigami.ScrollablePage {
            padding: Kirigami.Units.largeSpacing
            Loader {
                id: loader
                width: parent.width
                sourceComponent: {
                    if (!hasValidCertificate) {
                        return undefined;
                    }
                    switch (certSelector.currentValue.type) {
                        case KHC.HealthCertificate.Vaccination:
                            return vaccinationDetails;
                        case KHC.HealthCertificate.Test:
                            return testDetails;
                        case KHC.HealthCertificate.Recovery:
                            return recoveryDetails;
                        default:
                            return undefined;
                    }
                }
            }
        }
    }
    Kirigami.PlaceholderMessage {
        text: i18n("No certificates saved")
        visible: certSelector.count === 0
        anchors.centerIn: swipeView
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
    }

    footer: Kirigami.NavigationTabBar {
        visible: hasValidCertificate
        actions: [
            Kirigami.Action {
                text: i18n('Certificate')
                icon.name: 'view-barcode-qr'
                onTriggered: swipeView.currentIndex = 0
                checked: swipeView.currentIndex === 0
            },
            Kirigami.Action {
                text: i18n('Detail')
                icon.name: 'view-list-details'
                onTriggered: swipeView.currentIndex = 1;
                checked: swipeView.currentIndex === 1
            }
        ]
    }
}
