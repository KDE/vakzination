// SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef CERTIFICATESMODEL_H
#define CERTIFICATESMODEL_H

#include <QAbstractListModel>

#include <KConfig>
#include <KConfigGroup>

#include <KHealthCertificate/KRecoveryCertificate>
#include <KHealthCertificate/KTestCertificate>
#include <KHealthCertificate/KVaccinationCertificate>

#include "expected.h"

#include <optional>

using AnyCertificate = std::variant<KVaccinationCertificate, KTestCertificate, KRecoveryCertificate>;

namespace KItinerary
{
class ExtractorDocumentNode;
}

class CertificatesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles { CertificateRole = Qt::UserRole + 1 };

public:
    explicit CertificatesModel(bool testMode);
    QVariant data(const QModelIndex &index, int role) const override;

    int rowCount(const QModelIndex &parent) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void importCertificate(const QUrl &path);

Q_SIGNALS:
    void importError(const QString &error);

private:
    QVector<KVaccinationCertificate> fromStringList(const QStringList rawCertificates);
    QStringList toStringList(const QVector<KVaccinationCertificate> certificates);
    std::optional<AnyCertificate> findRecursive(const KItinerary::ExtractorDocumentNode &node);
    tl::expected<AnyCertificate, QString> importPrivate(const QUrl &url);
    std::optional<AnyCertificate> parseCertificate(const QByteArray &data);

    QVector<KVaccinationCertificate> m_vaccinations;
    KConfig m_config;
    KConfigGroup m_generalConfig;
    bool m_testMode;
};

#endif // CERTIFICATESMODEL_H
