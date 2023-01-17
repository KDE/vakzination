// SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "certificatesmodel.h"

#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <QTest>

#include <KLocalizedString>

#include <KItinerary/ExtractorCapabilities>

class CertificatesModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);

        // cleanup persisted content from previous runs
        CertificatesModel model(false);
        QAbstractItemModelTester modelTest(&model);
        if (model.rowCount()) {
            model.removeRows(0, model.rowCount());
        }
        QCOMPARE(model.rowCount(), 0);
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

        QCOMPARE(errorSpy.count(), 0);
        QCOMPARE(model.rowCount({}), 5);
        QCOMPARE(model.data(model.index(4), CertificatesModel::TypeRole), KHealthCertificate::Test);
    }

    void testImportPng()
    {
        if (KItinerary::ExtractorCapabilities::capabilitiesString().contains(QLatin1String("zxing (1.4.0)"))) {
            QSKIP("Using ZXing version incapable of decoding test data - skipping");
        }

        CertificatesModel model(true);
        QAbstractItemModelTester modelTest(&model);

        const QUrl testFile = QUrl::fromLocalFile(QFINDTESTDATA("full-vaccination.png"));

        QSignalSpy errorSpy(&model, &CertificatesModel::importError);
        model.importCertificate(testFile);

        QCOMPARE(errorSpy.count(), 0);
        QCOMPARE(model.rowCount({}), 5);
        QCOMPARE(model.data(model.index(4), CertificatesModel::TypeRole), KHealthCertificate::Vaccination);
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

        const QUrl testFile(QStringLiteral("/does/not/exist"));

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
            model.removeRow(0);
            QCOMPARE(model.rowCount({}), 1);
        }

        {
            CertificatesModel model(false);
            QAbstractItemModelTester modelTest(&model);
            QCOMPARE(model.rowCount({}), 1);
        }
    }
};

QTEST_MAIN(CertificatesModelTest)

#include "certificatesmodeltest.moc"
