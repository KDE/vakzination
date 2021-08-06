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
    void testEmptyModel()
    {
        QStandardPaths::setTestModeEnabled(true);

        CertificatesModel model(false);
        QAbstractItemModelTester modelTest(&model);

        QCOMPARE(model.rowCount({}), 0);
    }

    void testBuiltinCerts()
    {
        QStandardPaths::setTestModeEnabled(true);

        CertificatesModel model(true);
        QAbstractItemModelTester modelTest(&model);

        QCOMPARE(model.rowCount({}), 4);

        QCOMPARE(model.data(model.index(0), CertificatesModel::TypeRole), KHealthCertificate::Vaccination);
    }

    void testImport()
    {
        QStandardPaths::setTestModeEnabled(true);

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

    void testNonexistantImport()
    {
        QStandardPaths::setTestModeEnabled(true);

        CertificatesModel model(true);
        QAbstractItemModelTester modelTest(&model);

        const QUrl testFile = QStringLiteral("/does/not/exist");

        QSignalSpy errorSpy(&model, &CertificatesModel::importError);
        model.importCertificate(testFile);

        QCOMPARE(errorSpy.count(), 1);
        QCOMPARE(errorSpy.first().first().toString(), i18n("Could not open file: "));

        QCOMPARE(model.rowCount({}), 4);
    }
};

QTEST_MAIN(CertificatesModelTest)

#include "certificatesmodeltest.moc"
