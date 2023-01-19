// SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "certificatesmodel.h"

#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorEngine>
#include <kitinerary_version.h>

#include <QClipboard>
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QMimeData>

#include <KLocalizedString>

#include <KHealthCertificate/KHealthCertificateParser>

QVector<AnyCertificate> CertificatesModel::fromStringList(const QStringList rawCertificates) const
{
    QVector<AnyCertificate> res;
    res.reserve(rawCertificates.size());
    for (const auto &rawCertificate : rawCertificates) {
        const auto cert = parseCertificate(*QByteArray::fromBase64Encoding(rawCertificate.toUtf8()));
        if (cert) {
            res.push_back(*cert);
        }
    }
    return res;
}

QStringList CertificatesModel::toStringList(const QVector<AnyCertificate> certificates) const
{
    QStringList res;
    std::transform(certificates.cbegin(), certificates.cend(), std::back_inserter(res), [](const AnyCertificate &cert) {
        return std::visit(
            [](auto &&arg) {
                return QString::fromUtf8(arg.rawData().toBase64());
            },
            cert);
    });
    return res;
}

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
            m_certificates << KHealthCertificateParser::parse(sample).value<KVaccinationCertificate>();
        }
    } else {
        const QStringList certs = m_generalConfig.readEntry(QStringLiteral("certificates"), QStringList{});
        m_certificates = fromStringList(certs);
    }
}

QVariant CertificatesModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();

    switch (role) {
    case CertificateRole: {
        return std::visit(
            [](auto &&arg) -> QVariant {
                return arg;
            },
            m_certificates[row]);
    }
    case TypeRole: {
        if (std::holds_alternative<KVaccinationCertificate>(m_certificates[row])) {
            return KHealthCertificate::Vaccination;
        }
        if (std::holds_alternative<KTestCertificate>(m_certificates[row])) {
            return KHealthCertificate::Test;
        }
        if (std::holds_alternative<KRecoveryCertificate>(m_certificates[row])) {
            return KHealthCertificate::Recovery;
        }
    }
    };
    return {};
}

int CertificatesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_certificates.size();
}

QHash<int, QByteArray> CertificatesModel::roleNames() const
{
    return {
        {CertificateRole, "certificate"},
        {TypeRole, "type"},
    };
}

void CertificatesModel::addCertificate(AnyCertificate cert)
{
    beginInsertRows({}, m_certificates.size(), m_certificates.size());
    m_certificates << cert;

    if (!m_testMode) {
        m_generalConfig.writeEntry(QStringLiteral("certificates"), toStringList(m_certificates));
    }

    endInsertRows();
}

void CertificatesModel::importCertificate(const QUrl &path)
{
    tl::expected<int, QString> maybeResult = importPrivate(path);

    if (!maybeResult) {
        qWarning() << "Failed to import" << maybeResult.error();
        Q_EMIT importError(maybeResult.error());
    }
}

tl::expected<int, QString> CertificatesModel::importPrivate(const QUrl &url)
{
    if (url.isEmpty()) {
        return tl::make_unexpected(i18n("Empty file url"));
    }

    if (!url.isValid()) {
        return tl::make_unexpected(i18n("File URL not valid: %1", url.toDisplayString()));
    }

    QFile certFile(toLocalFile(url));

    bool ok = certFile.open(QFile::ReadOnly);

    if (!ok) {
        return tl::make_unexpected(i18n("Could not open file: %1", toLocalFile(url)));
    }

    const QByteArray data = certFile.readAll();

    std::optional<AnyCertificate> maybeCertificate = parseCertificate(data);
    if (maybeCertificate) {
        addCertificate(*maybeCertificate);
        return 1;
    } else {
        // let's see if this is a PDF containing barcodes instead
        KItinerary::ExtractorEngine engine;
#if KITINERARY_VERSION >= QT_VERSION_CHECK(5, 19, 41)
        // user opened the file, so we can be reasonably sure they assume it contains
        // relevant content, so try expensive extraction methods too
        engine.setHints(KItinerary::ExtractorEngine::ExtractFullPageRasterImages);
#endif
        engine.setData(data, url.path());
        engine.extract();
        if (auto count = findRecursive(engine.rootDocumentNode())) {
            return count;
        } else {
            return tl::make_unexpected(i18n("No certificate found in %1", toLocalFile(url)));
        }
    }
}

std::optional<AnyCertificate> CertificatesModel::parseCertificate(const QByteArray &data) const
{
    const QVariant maybeCertificate = KHealthCertificateParser::parse(data);

    if (maybeCertificate.userType() == qMetaTypeId<KVaccinationCertificate>()) {
        return maybeCertificate.value<KVaccinationCertificate>();
    }

    if (maybeCertificate.userType() == qMetaTypeId<KTestCertificate>()) {
        return maybeCertificate.value<KTestCertificate>();
    }

    if (maybeCertificate.userType() == qMetaTypeId<KRecoveryCertificate>()) {
        return maybeCertificate.value<KRecoveryCertificate>();
    }

    return {};
}

int CertificatesModel::findRecursive(const KItinerary::ExtractorDocumentNode &node)
{
    // can possibly contain a barcode
    if (node.childNodes().size() == 1
        && (node.mimeType() == QLatin1String("internal/qimage") || node.mimeType() == QLatin1String("application/vnd.apple.pkpass"))) {
        const auto &child = node.childNodes()[0];
        std::optional<AnyCertificate> cert;
        if (child.isA<QString>()) {
            cert = parseCertificate(child.content<QString>().toUtf8());
        } else if (child.isA<QByteArray>()) {
            cert = parseCertificate(child.content<QByteArray>());
        }

        if (cert) {
            addCertificate(*cert);
            return 1;
        }
    }

    // recurse
    int count = 0;
    for (const auto &child : node.childNodes()) {
        count += findRecursive(child);
    }
    return count;
}

void CertificatesModel::importCertificateFromClipboard()
{
    const auto md = QGuiApplication::clipboard()->mimeData();
    if (md->hasText()) {
        importCertificateFromText(md->text());
    } else if (md->hasFormat(QLatin1String("application/octet-stream"))) {
        importCertificateFromData(md->data(QLatin1String("application/octet-stream")));
    }
}

void CertificatesModel::importCertificateFromText(const QString &text)
{
    importCertificateFromData(text.toUtf8());
}

void CertificatesModel::importCertificateFromData(const QByteArray &data)
{
    auto maybeCert = parseCertificate(data);
    if (maybeCert) {
        addCertificate(*maybeCert);
    } else {
        Q_EMIT importError(i18n("No certificate in clipboard"));
    }
}

QString CertificatesModel::toLocalFile(const QUrl &url)
{
#ifdef Q_OS_ANDROID
    // toLocalFile makes content:/ URLs kaputt
    return url.toString();
#else
    return url.toLocalFile();
#endif
}

bool CertificatesModel::removeRow(int row, const QModelIndex &parent)
{
    return QAbstractListModel::removeRow(row, parent);
}

bool CertificatesModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid()) {
        return false;
    }

    beginRemoveRows({}, row, row + count - 1);
    m_certificates.erase(m_certificates.begin() + row, m_certificates.begin() + row + count);
    if (!m_testMode) {
        m_generalConfig.writeEntry(QStringLiteral("certificates"), toStringList(m_certificates));
    }
    endRemoveRows();
    return true;
}
