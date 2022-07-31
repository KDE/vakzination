/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.2
import org.kde.kirigami 2.15 as Kirigami
import org.kde.khealthcertificate 1.0
import org.kde.i18n.localeData 1.0

Kirigami.OverlaySheet {

    required property var certificate

    header: Kirigami.Heading {
        text: i18n("Details")
    }

    function daysTo(d1, d2) {
        return (d2.getTime() - d1.getTime()) / (1000 * 60 * 60 * 24);
    }

    Kirigami.FormLayout {

        Item {
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
            Kirigami.FormData.label: i18n("Vaccination")
        }

        QQC2.Label {
            text: certificate.date.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
            Kirigami.FormData.label: i18n("Date:")
            color: daysTo(certificate.date, new Date()) >= 14 ? Kirigami.Theme.textColor : Kirigami.Theme.neutralTextColor
            visible: !isNaN(certificate.date.getTime())
        }
        QQC2.Label {
            text: certificate.disease
            Kirigami.FormData.label: i18n("Disease:")
            visible: certificate.disease
        }
        QQC2.Label {
            text: certificate.vaccineType
            Kirigami.FormData.label: i18n("Type:")
            visible: certificate.vaccineType
        }
        QQC2.Label {
            text: certificate.vaccineUrl != "" ? '<a href="' + certificate.vaccineUrl + '">' + certificate.vaccine + '</a>' : certificate.vaccine
            Kirigami.FormData.label: i18n("Vaccine:")
            onLinkActivated: Qt.openUrlExternally(link)
            visible: text !== ""
        }
        QQC2.Label {
            text: certificate.manufacturer
            Kirigami.FormData.label: i18n("Manufacturer:")
            visible: text !== ""
        }
        QQC2.Label {
            text: certificate.totalDoses > 0 ? i18n("%1/%2", certificate.dose, certificate.totalDoses) : certificate.dose
            Kirigami.FormData.label: i18n("Dose:")
            color: certificate.dose < certificate.totalDoses ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.textColor
            visible: certificate.dose > 0
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
            visible: certificate.certificateId
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
        }
        Kirigami.Icon {
            source: {
                switch(certificate.signatureState) {
                    case HealthCertificate.ValidSignature: return "dialog-ok";
                    case HealthCertificate.UnknownSignature: return "question";
                    case HealthCertificate.InvalidSignature:
                    default:
                        return "dialog-error-symbolic";
                }
            }
            height: Kirigami.Units.gridUnit
            Kirigami.FormData.label: i18n("Signature:")
            color: {
                switch(certificate.signatureState) {
                    case HealthCertificate.ValidSignature: return Kirigami.Theme.positiveTextColor;
                    case HealthCertificate.UnknownSignature: return Kirigami.Theme.neutralTextColor;
                    case HealthCertificate.InvalidSignature:
                    default:
                        return Kirigami.Theme.negativeTextColor;
                }
            }
            visible: certificate.signatureState != HealthCertificate.UncheckedSignature
        }
    }
}

