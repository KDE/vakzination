// SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "certificatesmodel.h"

#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorEngine>

#include <QDebug>
#include <QFile>
#include <QJsonArray>

#include <KHealthCertificate/KHealthCertificateParser>

const QByteArray sample =
    "HC1:6BF+70790T9WJWG.FKY*4GO0.O1CV2 O5 "
    "N2FBBRW1*70HS8WY04AC*WIFN0AHCD8KD97TK0F90KECTHGWJC0FDC:5AIA%G7X+AQB9746HS80:54IBQF60R6$A80X6S1BTYACG6M+9XG8KIAWNA91AY%67092L4WJCT3EHS8XJC$+"
    "DXJCCWENF6OF63W5NW6WF6%JC QE/IAYJC5LEW34U3ET7DXC9 QE-ED8%E.JCBECB1A-:8$96646AL60A60S6Q$D.UDRYA "
    "96NF6L/5QW6307KQEPD09WEQDD+Q6TW6FA7C466KCN9E%961A6DL6FA7D46JPCT3E5JDLA7$Q6E464W5TG6..DX%DZJC6/DTZ9 QE5$CB$DA/D JC1/D3Z8WED1ECW.CCWE.Y92OAGY8MY9L+9MPCG/D5 "
    "C5IA5N9$PC5$CUZCY$5Y$527B+A4KZNQG5TKOWWD9FL%I8U$F7O2IBM85CWOC%LEZU4R/BXHDAHN 11$CA5MRI:AONFN7091K9FKIGIY%VWSSSU9%01FO2*FTPQ3C3F";

CertificatesModel::CertificatesModel(bool testMode)
    : QAbstractListModel()
    , m_config(QStringLiteral("vakzinationrc"))
    , m_testMode(testMode)
{
    m_generalConfig = m_config.group(QStringLiteral("General"));

    if (m_testMode) {
        for (int i = 0; i < 4; i++) {
            m_vaccinations << KHealthCertificateParser::parse(sample).value<KVaccinationCertificate>();
        }
    } else {
        const QStringList certificates = m_generalConfig.readEntry(QStringLiteral("vaccinations"), QStringList{});

        m_vaccinations = fromStringList(certificates);
    }
}

QVariant CertificatesModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();

    switch (role) {
    case CertificateRole: {
        return m_vaccinations[row];
    }
    };
    return QStringLiteral("deadbeef");
}

int CertificatesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_vaccinations.size();
}

QHash<int, QByteArray> CertificatesModel::roleNames() const
{
    return {{CertificateRole, "certificate"}};
}

void CertificatesModel::importCertificate(const QUrl &path)
{
    QFile certFile(path.toLocalFile());

    bool ok = certFile.open(QFile::ReadOnly);

    if (!ok) {
        qWarning() << "Could not open certificate file" << path;
        Q_EMIT importError();
        return;
    }

    const QByteArray data = certFile.readAll();

    QVariant maybeCertificate = KHealthCertificateParser::parse(data);

    if (maybeCertificate.isNull()) {
        // let's see if this is a PDF containing barcodes instead
        KItinerary::ExtractorEngine engine;
        engine.setData(data, path.path());
        engine.extract();
        if (findRecursive(engine.rootDocumentNode())) {
            return;
        }

        qWarning() << "Could not find certificate in" << path;
        Q_EMIT importError();
        return;
    }

    importCertificate(maybeCertificate);
}

bool CertificatesModel::importCertificate(const QVariant &maybeCertificate)
{
    if (maybeCertificate.userType() != qMetaTypeId<KVaccinationCertificate>()) {
        return false;
    }

    KVaccinationCertificate cert = maybeCertificate.value<KVaccinationCertificate>();

    beginInsertRows({}, m_vaccinations.size(), m_vaccinations.size());
    m_vaccinations << cert;

    if (!m_testMode) {
        m_generalConfig.writeEntry(QStringLiteral("vaccinations"), toStringList(m_vaccinations));
    }
    endInsertRows();
    return true;
}

QVector<KVaccinationCertificate> CertificatesModel::fromStringList(const QStringList rawCertificates)
{
    QVector<KVaccinationCertificate> res;
    std::transform(rawCertificates.cbegin(), rawCertificates.cend(), std::back_inserter(res), [](const QString &raw) {
        return KHealthCertificateParser::parse(*QByteArray::fromBase64Encoding(raw.toUtf8())).value<KVaccinationCertificate>();
    });
    return res;
}

QStringList CertificatesModel::toStringList(const QVector<KVaccinationCertificate> certificates)
{
    QStringList res;
    std::transform(certificates.cbegin(), certificates.cend(), std::back_inserter(res), [](const KVaccinationCertificate &cert) {
        return QString::fromUtf8(cert.rawData().toBase64());
    });
    return res;
}

bool CertificatesModel::findRecursive(const KItinerary::ExtractorDocumentNode &node)
{
    // possibly a barcode
    if (node.childNodes().size() == 1 && node.mimeType() == QLatin1String("internal/qimage")) {
        const auto &child = node.childNodes()[0];
        if (child.isA<QString>()) {
            return importCertificate(KHealthCertificateParser::parse(child.content<QString>().toUtf8()));
        }
        if (child.isA<QByteArray>()) {
            return importCertificate(KHealthCertificateParser::parse(child.content<QByteArray>()));
        }
    }

    // recurse
    bool found = false;
    for (const auto &child : node.childNodes()) {
        if (findRecursive(child)) {
            found = true;
        }
    }
    return found;
}
