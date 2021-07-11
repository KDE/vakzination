// SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef CERTIFICATESMODEL_H
#define CERTIFICATESMODEL_H

#include <QAbstractListModel>

#include <KHealthCertificate/KVaccinationCertificate>

class CertificatesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles { CertificateRole = Qt::UserRole + 1 };

public:
    explicit CertificatesModel(QObject *parent = nullptr);
    QVariant data(const QModelIndex &index, int role) const override;

    int rowCount(const QModelIndex &parent) const override;

    QHash<int, QByteArray> roleNames() const override;

private:
    QVector<KVaccinationCertificate> m_vaccinations;
};

#endif // CERTIFICATESMODEL_H
