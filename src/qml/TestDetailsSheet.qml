/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.khealthcertificate as KHC
import org.kde.i18n.localeData

ColumnLayout {

    required property var certificate

    function daysTo(d1, d2) {
        return (d2.getTime() - d1.getTime()) / (1000 * 60 * 60 * 24);
    }

    Kirigami.FormLayout {

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Person")
        }

        QQC2.Label {
            text: certificate.name
            Kirigami.FormData.label: i18n("Name:")
        }
        QQC2.Label {
            text: certificate.dateOfBirth.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
            visible: !isNaN(certificate.dateOfBirth.getTime())
            Kirigami.FormData.label: i18n("Date of birth:")
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Test")
        }

        QQC2.Label {
            text: certificate.date.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
            Kirigami.FormData.label: i18n("Date:")
            color: daysTo(certificate.date, new Date()) >= 2 ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.textColor
            visible: !isNaN(certificate.date.getTime())
        }
        QQC2.Label {
            text: certificate.disease
            Kirigami.FormData.label: i18n("Disease:")
            visible: certificate.disease
        }
        QQC2.Label {
            text: certificate.testType
            Kirigami.FormData.label: i18n("Type:")
            visible: certificate.testType
        }
        QQC2.Label {
            text: certificate.testUrl != "" ? '<a href="' + certificate.testUrl + '">' + certificate.testName + '</a>' : certificate.testName
            visible: certificate.testName.length > 0
            Kirigami.FormData.label: i18n("Test:")
            onLinkActivated: Qt.openUrlExternally(link)
        }
        QQC2.Label {
            text: {
                if (certificate.resultString !== "") {
                    return certificate.resultString;
                }
                switch (certificate.result) {
                    case KHC.TestCertificate.Positive:
                        return i18nc('test result', 'Positive');
                    case KHC.TestCertificate.Negative:
                        return i18nc('test result', 'Negative');
                }
            }
            Kirigami.FormData.label: i18n("Result:")
            color: certificate.result == KHC.TestCertificate.Positive ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
        }
        QQC2.Label {
            text: certificate.testCenter
            Kirigami.FormData.label: i18n("Test Center:")
            visible: text !== ""
        }
        QQC2.Label {
            text: i18nc("%1 a flag emoji, %2 is a country name", "%1 %2", Country.fromAlpha2(certificate.country).emojiFlag, Country.fromAlpha2(certificate.country).name)
            Kirigami.FormData.label: i18n("Country:")
            visible: certificate.country
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Certificate")
        }

        QQC2.Label {
            text: certificate.certificateIssuer
            Kirigami.FormData.label: i18n("Issuer:")
            visible: text !== ""
        }
        QQC2.Label {
            text: certificate.certificateId
            Kirigami.FormData.label: i18n("Identifier:")
            wrapMode: Text.Wrap
            visible: text !== ""
        }
        QQC2.Label {
            text: certificate.certificateIssueDate.toLocaleString(Qt.locale(), Locale.ShortFormat)
            Kirigami.FormData.label: i18n("Issued:")
            visible: !isNaN(certificate.certificateIssueDate.getTime())
        }
        QQC2.Label {
            text: certificate.certificateExpiryDate.toLocaleString(Qt.locale(), Locale.ShortFormat)
            Kirigami.FormData.label: i18n("Expires:")
            visible: !isNaN(certificate.certificateExpiryDate.getTime())
            color: certificate.certificateExpiryDate.getTime() < Date.now() ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
        }
        Kirigami.Icon {
            source: {
                switch(certificate.signatureState) {
                    case KHC.HealthCertificate.ValidSignature: return "dialog-ok";
                    case KHC.HealthCertificate.UnknownSignature: return "question";
                    case KHC.HealthCertificate.InvalidSignature:
                    default:
                        return "dialog-error-symbolic";
                }
            }
            height: Kirigami.Units.gridUnit
            Kirigami.FormData.label: i18n("Signature:")
            color: {
                switch(certificate.signatureState) {
                    case KHC.HealthCertificate.ValidSignature: return Kirigami.Theme.positiveTextColor;
                    case KHC.HealthCertificate.UnknownSignature: return Kirigami.Theme.neutralTextColor;
                    case KHC.HealthCertificate.InvalidSignature:
                    default:
                        return Kirigami.Theme.negativeTextColor;
                }
            }
            visible: certificate.signatureState != KHC.HealthCertificate.UncheckedSignature
        }
    }
}

