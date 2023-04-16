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
#include <khealthcertificate/khealthcertificate_version.h>

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

static QByteArray certRawData(const AnyCertificate &cert)
{
    return std::visit(
        [](auto &&c) {
            return c.rawData();
        },
        cert);
}

QStringList CertificatesModel::toStringList(const QVector<AnyCertificate> certificates) const
{
    QStringList res;
    std::transform(certificates.cbegin(), certificates.cend(), std::back_inserter(res), [](const AnyCertificate &cert) {
        return QString::fromUtf8(certRawData(cert).toBase64());
    });
    return res;
}

static QDateTime certRelevantUntil(const AnyCertificate &cert)
{
    return std::visit(
        [](auto &&c) {
            return KHealthCertificate::relevantUntil(c);
        },
        cert);
}

static bool certLessThan(const AnyCertificate &lhs, const AnyCertificate &rhs)
{
    const auto lhsDt = certRelevantUntil(lhs);
    const auto rhsDt = certRelevantUntil(rhs);
    if (lhsDt == rhsDt) {
        return certRawData(lhs) < certRawData(rhs); // ensure a stable sorting in all cases
    }
    if (!lhsDt.isValid()) {
        return false;
    }
    return !rhsDt.isValid() || lhsDt > rhsDt;
}

const QByteArray sample =
    "HC1:NCF570.90T9WTWGVLKG99.+VKV9NT3RH1X*4%AB3XK4F36:G$MB2F3F*K+UR3JCYHAY50.FK6ZK7:EDOLFVCPD0B$D% "
    "D3IA4W5646946%96X476KCN9E%961A69L6QW6B46XJCCWENF6OF63W5Y96B46WJCT3E2+8WJC0FD4:473DSDDF+ANG7ZHAFM89A6A1A71B/M8RY971BS1BAC9$+ADB8ZCAM%6//6.JCP9EJY8L/5M/"
    "5546.96D46%JCIQE1C93KC.SC4KCD3DX47B46IL6646I*6..DX%DLPCG/DZ-CFZA71A1T8W.CZ-C4%E-3E4VCI3D7WEMY95IAWY8I3DD "
    "CGECQED$PC5$CUZCY$5Y$5JPCT3E5JDLA7KF6D463W5WA6%78%VIKQS*9OE.U37WGJG.1J5PF9WOASFU3UI69PKJEH2F:SY2SCYKFOMVGP OLGW31.J5OVSAFBGON19H+HCSIA7P:65P0F-QR/GS:2";

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

    std::sort(m_certificates.begin(), m_certificates.end(), certLessThan);
}

QVariant CertificatesModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    if (!checkIndex(index) || row < 0) {
        return {};
    }

    switch (role) {
    case Qt::DisplayRole: {
        if (std::holds_alternative<KVaccinationCertificate>(m_certificates[row])) {
            const auto cert = std::get<KVaccinationCertificate>(m_certificates[row]);
            if (cert.dose() > 0 && cert.totalDoses() > 0) {
                return i18n("Vaccination %1/%2 (%3)", cert.dose(), cert.totalDoses(), cert.name());
            }
            return i18n("Vaccination (%1)", cert.name());
        }
        if (std::holds_alternative<KTestCertificate>(m_certificates[row])) {
            const auto cert = std::get<KTestCertificate>(m_certificates[row]);
            return i18n("Test %1 (%2)",
                        QLocale().toString(cert.date().isValid() ? cert.date() : cert.certificateIssueDate().date(), QLocale::NarrowFormat),
                        cert.name());
        }
        if (std::holds_alternative<KRecoveryCertificate>(m_certificates[row])) {
            const auto cert = std::get<KRecoveryCertificate>(m_certificates[row]);
            return i18n("Recovery (%1)", cert.name());
        }
        return {};
    }
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
    auto n = QAbstractListModel::roleNames();
    n.insert(CertificateRole, "certificate");
    n.insert(TypeRole, "type");
    return n;
}

void CertificatesModel::addCertificate(AnyCertificate cert)
{
#if KHEALTHCERTIFICATE_VERSION >= QT_VERSION_CHECK(22, 11, 40)
    auto it = std::find(m_certificates.begin(), m_certificates.end(), cert);
    if (it != m_certificates.end()) { // certificate is already known
        return;
    }
#endif

    it = std::lower_bound(m_certificates.begin(), m_certificates.end(), cert, certLessThan);
    const auto row = std::distance(m_certificates.begin(), it);
    beginInsertRows({}, row, row);
    m_certificates.insert(it, cert);

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
    bool result = false;
    if (md->hasText()) {
        result = importCertificateFromText(md->text());
    } else if (md->hasFormat(QLatin1String("application/octet-stream"))) {
        result = importCertificateFromData(md->data(QLatin1String("application/octet-stream")));
    }

    if (!result) {
        Q_EMIT importError(i18n("No certificate in clipboard"));
    }
}

bool CertificatesModel::importCertificateFromText(const QString &text)
{
    return importCertificateFromData(text.toUtf8());
}

bool CertificatesModel::importCertificateFromData(const QByteArray &data)
{
    auto maybeCert = parseCertificate(data);
    if (maybeCert) {
        addCertificate(*maybeCert);
        return true;
    }

    return false;
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
