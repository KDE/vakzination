// SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "certificatesmodel.h"

#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <QTest>

#include <KLocalizedString>

#include "config-vakzination.h"

class CertificatesModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);

        // cleanup persisted content from previous runs
        KConfig config(QStringLiteral("vakzinationrc"));
        auto general = config.group(QStringLiteral("General"));
        general.deleteEntry(QStringLiteral("certificates"));
    }

    void testEmptyModel()
    {
        CertificatesModel model(false);
        QAbstractItemModelTester modelTest(&model);

        QCOMPARE(model.rowCount({}), 0);
    }

    void testBuiltinCerts()
    {
        CertificatesModel model(true);
        QAbstractItemModelTester modelTest(&model);

        QCOMPARE(model.rowCount({}), 4);

        QCOMPARE(model.data(model.index(0), CertificatesModel::TypeRole), KHealthCertificate::Vaccination);
    }

    void testImportPdf()
    {
        CertificatesModel model(true);
        QAbstractItemModelTester modelTest(&model);

        const QUrl testFile = QUrl::fromLocalFile(QFINDTESTDATA("negative-pcr-test-fr.pdf"));

        QSignalSpy errorSpy(&model, &CertificatesModel::importError);
        model.importCertificate(testFile);

#if HAVE_KITINERARY
        QCOMPARE(errorSpy.count(), 0);
        QCOMPARE(model.rowCount({}), 5);
        QCOMPARE(model.data(model.index(4), CertificatesModel::TypeRole), KHealthCertificate::Test);
#else
        QCOMPARE(errorSpy.count(), 1);
        QCOMPARE(errorSpy.first().first().toString(), i18n("Importing certificates from PDF is not supported in this build"));
#endif
    }

    void testImportPng()
    {
        CertificatesModel model(true);
        QAbstractItemModelTester modelTest(&model);

        const QUrl testFile = QUrl::fromLocalFile(QFINDTESTDATA("full-vaccination.png"));

        QSignalSpy errorSpy(&model, &CertificatesModel::importError);
        model.importCertificate(testFile);

#if HAVE_KITINERARY
        QCOMPARE(errorSpy.count(), 0);
        QCOMPARE(model.rowCount({}), 5);
        QCOMPARE(model.data(model.index(4), CertificatesModel::TypeRole), KHealthCertificate::Vaccination);
#else
        QCOMPARE(errorSpy.count(), 1);
        QCOMPARE(errorSpy.first().first().toString(), i18n("Importing certificates from images is not supported in this build"));
#endif
    }

    void testImportPlain()
    {
        CertificatesModel model(true);
        QAbstractItemModelTester modelTest(&model);

        const QUrl testFile = QUrl::fromLocalFile(QFINDTESTDATA("full-vaccination.txt"));

        QSignalSpy errorSpy(&model, &CertificatesModel::importError);
        model.importCertificate(testFile);

        QCOMPARE(errorSpy.count(), 0);
        QCOMPARE(model.rowCount({}), 5);
        QCOMPARE(model.data(model.index(4), CertificatesModel::TypeRole), KHealthCertificate::Vaccination);
    }

    void testNonexistantImport()
    {
        CertificatesModel model(true);
        QAbstractItemModelTester modelTest(&model);

        const QUrl testFile = QStringLiteral("/does/not/exist");

        QSignalSpy errorSpy(&model, &CertificatesModel::importError);
        model.importCertificate(testFile);

        QCOMPARE(errorSpy.count(), 1);
        QCOMPARE(errorSpy.first().first().toString(), i18n("Could not open file: "));

        QCOMPARE(model.rowCount({}), 4);
    }

    void testNonsenseFile()
    {
        CertificatesModel model(true);
        QAbstractItemModelTester modelTest(&model);

        const QUrl testFile = QUrl::fromLocalFile(QFINDTESTDATA("nonsense.txt"));

        QSignalSpy errorSpy(&model, &CertificatesModel::importError);
        model.importCertificate(testFile);

        QCOMPARE(errorSpy.count(), 1);
        QCOMPARE(errorSpy.first().first().toString(), i18n("No certificate found in %1", testFile.toLocalFile()));

        QCOMPARE(model.rowCount({}), 4);
    }

    void testPersistence()
    {
        {
            CertificatesModel model(false);
            QAbstractItemModelTester modelTest(&model);
            model.importCertificate(QUrl::fromLocalFile(QFINDTESTDATA("full-vaccination.txt")));
            model.importCertificate(QUrl::fromLocalFile(QFINDTESTDATA("partial-vaccination.divoc")));
            QCOMPARE(model.rowCount({}), 2);
        }

        {
            KConfig config(QStringLiteral("vakzinationrc"));
            auto general = config.group(QStringLiteral("General"));
            auto certs = general.readEntry(QStringLiteral("certificates"), QStringList{});
            QCOMPARE(certs.size(), 2);
            certs.push_back(QStringLiteral("garbage"));
            general.writeEntry(QStringLiteral("certificates"), certs);
        }

        {
            CertificatesModel model(false);
            QAbstractItemModelTester modelTest(&model);
            QCOMPARE(model.rowCount({}), 2);
        }
    }
};

QTEST_MAIN(CertificatesModelTest)

#include "certificatesmodeltest.moc"
