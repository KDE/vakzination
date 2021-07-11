// SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "certificatesmodel.h"

#include <QDebug>
#include <QFile>

#include <KHealthCertificate/KHealthCertificateParser>

const QByteArray sample =
    "HC1:6BF+70790T9WJWG.FKY*4GO0.O1CV2 O5 "
    "N2FBBRW1*70HS8WY04AC*WIFN0AHCD8KD97TK0F90KECTHGWJC0FDC:5AIA%G7X+AQB9746HS80:54IBQF60R6$A80X6S1BTYACG6M+9XG8KIAWNA91AY%67092L4WJCT3EHS8XJC$+"
    "DXJCCWENF6OF63W5NW6WF6%JC QE/IAYJC5LEW34U3ET7DXC9 QE-ED8%E.JCBECB1A-:8$96646AL60A60S6Q$D.UDRYA "
    "96NF6L/5QW6307KQEPD09WEQDD+Q6TW6FA7C466KCN9E%961A6DL6FA7D46JPCT3E5JDLA7$Q6E464W5TG6..DX%DZJC6/DTZ9 QE5$CB$DA/D JC1/D3Z8WED1ECW.CCWE.Y92OAGY8MY9L+9MPCG/D5 "
    "C5IA5N9$PC5$CUZCY$5Y$527B+A4KZNQG5TKOWWD9FL%I8U$F7O2IBM85CWOC%LEZU4R/BXHDAHN 11$CA5MRI:AONFN7091K9FKIGIY%VWSSSU9%01FO2*FTPQ3C3F";

CertificatesModel::CertificatesModel(bool testMode)
    : QAbstractListModel()
{
    if (testMode) {
        for (int i = 0; i < 4; i++) {
            m_vaccinations << KHealthCertificateParser::parse(sample).value<KVaccinationCertificate>();
        }
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
        qWarning() << "Could not parse certificate" << path;
        Q_EMIT importError();
        return;
    }

    KVaccinationCertificate cert = maybeCertificate.value<KVaccinationCertificate>();

    beginInsertRows({}, m_vaccinations.size(), m_vaccinations.size());
    m_vaccinations << cert;
    endInsertRows();
}
