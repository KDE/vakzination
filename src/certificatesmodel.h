// SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef CERTIFICATESMODEL_H
#define CERTIFICATESMODEL_H

#include <QAbstractListModel>

#include <KConfig>
#include <KConfigGroup>

#include <KRecoveryCertificate>
#include <KTestCertificate>
#include <KVaccinationCertificate>

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
    enum Roles {
        CertificateRole = Qt::UserRole + 1,
        TypeRole,
    };

public:
    explicit CertificatesModel(bool testMode);
    QVariant data(const QModelIndex &index, int role) const override;

    int rowCount(const QModelIndex &parent = {}) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void importCertificate(const QUrl &path);
    Q_INVOKABLE void importCertificateFromClipboard();
    Q_INVOKABLE bool importCertificateFromData(const QByteArray &data);
    Q_INVOKABLE bool importCertificateFromText(const QString &text);

    Q_INVOKABLE bool removeRow(int row, const QModelIndex &parent = {}); // not exported to QML in Qt5 yet
    Q_INVOKABLE bool removeRows(int row, int count, const QModelIndex &parent = {}) override;

Q_SIGNALS:
    void importError(const QString &error);

private:
    QVector<AnyCertificate> fromStringList(const QStringList rawCertificates) const;
    QStringList toStringList(const QVector<AnyCertificate> certificates) const;

    int findRecursive(const KItinerary::ExtractorDocumentNode &node);
    tl::expected<int, QString> importPrivate(const QUrl &url);
    std::optional<AnyCertificate> parseCertificate(const QByteArray &data) const;
    void addCertificate(AnyCertificate cert);
    QString toLocalFile(const QUrl &url);

    QVector<AnyCertificate> m_certificates;

    KConfig m_config;
    KConfigGroup m_generalConfig;
    bool m_testMode;
};

#endif // CERTIFICATESMODEL_H
