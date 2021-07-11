/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
*/

#include <QApplication>
#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QUrl>
#include <QtQml>

#include <KLocalizedContext>
#include <KLocalizedString>

#include "certificatesmodel.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("KDE");
    QCoreApplication::setOrganizationDomain("kde.org");
    QCoreApplication::setApplicationName("vakzination");
    QGuiApplication::setApplicationDisplayName("Vakzination");

    QCommandLineParser parser;
    parser.addOption(QCommandLineOption(QStringLiteral("testmode"), i18n("Show with test data")));

    parser.process(app);

    const bool testMode = parser.isSet(QStringLiteral("testmode"));

    QQmlApplicationEngine engine;

    CertificatesModel model(testMode);
    qmlRegisterSingletonInstance("org.kde.vakzination", 1, 0, "CertificatesModel", &model);

    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
