/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
*/

#include <QCommandLineParser>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QUrl>
#include <QtQml>

#ifdef Q_OS_ANDROID
#include <QGuiApplication>
#else
#include <QApplication>
#endif

#include <KAboutData>
#include <KLocalizedContext>
#include <KLocalizedString>

#include "certificatesmodel.h"
#include "version.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("org.kde.breeze"));
#else
    QIcon::setFallbackThemeName(QStringLiteral("breeze"));
    QApplication app(argc, argv);
    // Default to org.kde.desktop style unless the user forces another style
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }
#endif

    KLocalizedString::setApplicationDomain("vakzination");

    KAboutData about(QStringLiteral("vakzination"),
                     i18n("Vakzination"),
                     QStringLiteral(VAKZINATION_VERSION_STRING),
                     i18n("Vakzination manages your health certificates like vaccination, test, and recovery certificates."),
                     KAboutLicense::GPL_V2,
                     i18n("© 2021-2022 KDE Community"));

    about.addAuthor(i18n("Nicolas Fella"), i18n("Maintainer"), QStringLiteral("nicolas.fella@gmx.de"));
    about.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"), i18nc("EMAIL OF TRANSLATORS", "Your emails"));
    about.setOrganizationDomain("kde.org");
    KAboutData::setApplicationData(about);

    QCommandLineParser parser;
    about.setupCommandLine(&parser);
    parser.addOption(QCommandLineOption(QStringLiteral("testmode"), i18n("Show with test data")));
    QCommandLineOption isTemporaryOpt(QStringLiteral("tempfile"), QStringLiteral("Input file is a temporary file and will be deleted after importing."));
    parser.addOption(isTemporaryOpt);
    parser.addPositionalArgument(QStringLiteral("file"), i18n("File to import."));
    parser.process(app);
    about.processCommandLine(&parser);

    const bool testMode = parser.isSet(QStringLiteral("testmode"));

    QQmlApplicationEngine engine;

    CertificatesModel model(testMode);
    for (const auto &file : parser.positionalArguments()) {
        const auto url = QUrl::fromLocalFile(file);
        model.importCertificate(url);
        if (parser.isSet(isTemporaryOpt)) {
            QFile::remove(url.toLocalFile());
        }
    }

    qmlRegisterSingletonInstance("org.kde.vakzination", 1, 0, "CertificatesModel", &model);

    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
