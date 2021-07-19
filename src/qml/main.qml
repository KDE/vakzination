/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.2
import org.kde.kirigami 2.15 as Kirigami

import org.kde.vakzination 1.0

Kirigami.ApplicationWindow {
    id: root

    title: i18n("Your Certificates")

    minimumWidth: Kirigami.Units.gridUnit * 20
    minimumHeight: Kirigami.Units.gridUnit * 20

    pageStack.globalToolBar.toolbarActionAlignment: Qt.AlignLeft

    pageStack.initialPage: Qt.resolvedUrl("CertificatesPage.qml")
}
