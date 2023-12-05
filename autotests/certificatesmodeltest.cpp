// SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "certificatesmodel.h"

#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <QTest>

#include <KLocalizedString>

#include <KItinerary/ExtractorCapabilities>

#include <khealthcertificate_version.h>

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

    void testImportFromText()
    {
        CertificatesModel model(true);
        QAbstractItemModelTester modelTest(&model);
        QSignalSpy errorSpy(&model, &CertificatesModel::importError);
        QVERIFY(!model.importCertificateFromText(QStringLiteral("Not a certificate")));
        QCOMPARE(model.rowCount(), 4);

        QVERIFY(model.importCertificateFromText(
            QStringLiteral("HC1:6BF+70790T9WJWG.FKY*4GO0.O1CV2 O5 "
                           "N2FBBRW1*70HS8WY04AC*WIFN0AHCD8KD97TK0F90KECTHGWJC0FDC:5AIA%G7X+AQB9746HS80:54IBQF60R6$A80X6S1BTYACG6M+9XG8KIAWNA91AY%"
                           "67092L4WJCT3EHS8XJC$+DXJCCWENF6OF63W5NW6WF6%JC QE/IAYJC5LEW34U3ET7DXC9 QE-ED8%E.JCBECB1A-:8$96646AL60A60S6Q$D.UDRYA "
                           "96NF6L/5QW6307KQEPD09WEQDD+Q6TW6FA7C466KCN9E%961A6DL6FA7D46JPCT3E5JDLA7$Q6E464W5TG6..DX%DZJC6/DTZ9 QE5$CB$DA/D "
                           "JC1/D3Z8WED1ECW.CCWE.Y92OAGY8MY9L+9MPCG/D5 C5IA5N9$PC5$CUZCY$5Y$527B+A4KZNQG5TKOWWD9FL%I8U$F7O2IBM85CWOC%LEZU4R/BXHDAHN "
                           "11$CA5MRI:AONFN7091K9FKIGIY%VWSSSU9%01FO2*FTPQ3C3F")));
        QCOMPARE(model.rowCount(), 5);
        QCOMPARE(errorSpy.count(), 0);
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
    void testDeduplication()
    {
        CertificatesModel model(true);
        QAbstractItemModelTester modelTest(&model);
        model.importCertificate(QUrl::fromLocalFile(QFINDTESTDATA("full-vaccination.txt")));
        QCOMPARE(model.rowCount({}), 5);
        model.importCertificate(QUrl::fromLocalFile(QFINDTESTDATA("full-vaccination.txt")));
        QCOMPARE(model.rowCount({}), 5);
    }
};

QTEST_MAIN(CertificatesModelTest)

#include "certificatesmodeltest.moc"
