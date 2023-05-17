/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "file.h"
#include "jsonlddocument.h"
#include "logging.h"

#include <KItinerary/CreativeWork>

#include <KPkPass/Pass>

#include <KZip>

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QUuid>

using namespace KItinerary;

namespace KItinerary {
class FilePrivate
{
public:
    QString fileName;
    QIODevice *device = nullptr;
    std::unique_ptr<KZip> zipFile;
};
}

File::File()
    : d(new FilePrivate)
{
}

File::File(const QString &fileName)
    : d(new FilePrivate)
{
    d->fileName = fileName;
}

File::File(QIODevice* device)
    : d(new FilePrivate)
{
    d->device = device;
}

File::File(KItinerary::File &&) = default;

File::~File()
{
    close();
}

File& KItinerary::File::operator=(KItinerary::File &&) = default;

void File::setFileName(const QString &fileName)
{
    d->fileName = fileName;
}

bool File::open(File::OpenMode mode) const
{
    if (d->device) {
        d->zipFile = std::make_unique<KZip>(d->device);
    } else {
        d->zipFile = std::make_unique<KZip>(d->fileName);
    }

    if (!d->zipFile->open(mode == File::Write ? QIODevice::WriteOnly : QIODevice::ReadOnly)) {
        qCWarning(Log) << d->zipFile->errorString() << d->fileName;
        return false;
    }

    return true;
}

QString File::errorString() const
{
    if (d->zipFile && !d->zipFile->isOpen()) {
        return d->zipFile->errorString();
    }
    return {};
}

void File::close()
{
    if (d->zipFile) {
        d->zipFile->close();
    }
    d->zipFile.reset();
}

QVector<QString> File::reservations() const
{
    Q_ASSERT(d->zipFile);
    const auto resDir = dynamic_cast<const KArchiveDirectory*>(d->zipFile->directory()->entry(QLatin1String("reservations")));
    if (!resDir) {
        return {};
    }

    const auto entries = resDir->entries();
    QVector<QString> res;
    res.reserve(entries.size());
    for (const auto &entry : entries) {
        if (!entry.endsWith(QLatin1String(".json"))) {
            continue;
        }
        res.push_back(entry.left(entry.size() - 5));
    }

    return res;
}

QVariant File::reservation(const QString &resId) const
{
    Q_ASSERT(d->zipFile);
    const auto resDir = dynamic_cast<const KArchiveDirectory*>(d->zipFile->directory()->entry(QLatin1String("reservations")));
    if (!resDir) {
        return {};
    }

    const auto file = resDir->file(resId + QLatin1String(".json"));
    if (!file) {
        qCDebug(Log) << "reservation not found" << resId;
        return {};
    }

    const auto doc = QJsonDocument::fromJson(file->data());
    if (doc.isArray()) {
        const auto array = JsonLdDocument::fromJson(doc.array());
        if (array.size() != 1) {
            qCWarning(Log) << "reservation file for" << resId << "contains" << array.size() << "elements!";
            return {};
        }
        return array.at(0);
    } else if (doc.isObject()) {
        return JsonLdDocument::fromJsonSingular(doc.object());
    }
    return {};
}

void File::addReservation(const QVariant &res)
{
    addReservation(QUuid::createUuid().toString(QUuid::WithoutBraces), res);
}

void File::addReservation(const QString &id, const QVariant &res)
{
    Q_ASSERT(d->zipFile);
    d->zipFile->writeFile(QLatin1String("reservations/") + id + QLatin1String(".json"),
                          QJsonDocument(JsonLdDocument::toJson(res)).toJson());
}

QString File::passId(const KPkPass::Pass *pass)
{
    return passId(pass->passTypeIdentifier(), pass->serialNumber());
}

QString File::passId(const QString &passTypeIdenfier, const QString &serialNumber)
{
    if (passTypeIdenfier.isEmpty() || serialNumber.isEmpty()) {
        return {};
    }
    // serialNumber can contain percent-encoding or slashes, ie stuff we don't want to have in file names
    return passTypeIdenfier + QLatin1Char('/') + QString::fromUtf8(serialNumber.toUtf8().toBase64(QByteArray::Base64UrlEncoding));
}

QVector<QString> File::passes() const
{
    Q_ASSERT(d->zipFile);
    const auto passDir = dynamic_cast<const KArchiveDirectory*>(d->zipFile->directory()->entry(QLatin1String("passes")));
    if (!passDir) {
        return {};
    }

    const auto entries = passDir->entries();
    QVector<QString> passIds;
    for (const auto &entry : entries) {
        const auto subdir = dynamic_cast<const KArchiveDirectory*>(passDir->entry(entry));
        if (!subdir) {
            continue;
        }

        const auto subEntries = subdir->entries();
        for (const auto &subEntry : subEntries) {
            if (!subEntry.endsWith(QLatin1String(".pkpass"))) {
                continue;
            }
            passIds.push_back(entry + QLatin1Char('/') + QStringView(subEntry).left(subEntry.size() - 7));
        }
    }
    return passIds;
}

QByteArray File::passData(const QString& passId) const
{
    Q_ASSERT(d->zipFile);
    const auto passDir = dynamic_cast<const KArchiveDirectory*>(d->zipFile->directory()->entry(QLatin1String("passes")));
    if (!passDir) {
        return {};
    }

    const auto file = passDir->file(passId + QLatin1String(".pkpass"));
    if (!file) {
        qCDebug(Log) << "pass not found" << passId;
        return {};
    }
    return file->data();
}

void File::addPass(KPkPass::Pass* pass, const QByteArray& rawData)
{
    addPass(passId(pass), rawData);
}

void File::addPass(const QString &passId, const QByteArray& rawData)
{
    Q_ASSERT(d->zipFile);
    d->zipFile->writeFile(QLatin1String("passes/") + passId + QLatin1String(".pkpass"), rawData);
}

QVector<QString> File::documents() const
{
    const auto docDir = dynamic_cast<const KArchiveDirectory*>(d->zipFile->directory()->entry(QLatin1String("documents")));
    if (!docDir) {
        return {};
    }

    const auto entries = docDir->entries();
    QVector<QString> res;
    res.reserve(entries.size());
    for (const auto &entry : entries) {
        if (docDir->entry(entry)->isDirectory()) {
            res.push_back(entry);
        }
    }

    return res;
}

QVariant File::documentInfo(const QString &id) const
{
    Q_ASSERT(d->zipFile);
    const auto dir = dynamic_cast<const KArchiveDirectory*>(d->zipFile->directory()->entry(QLatin1String("documents/") + id));
    if (!dir) {
        return {};
    }

    const auto file = dir->file(QStringLiteral("meta.json"));
    if (!file) {
        qCDebug(Log) << "document meta data not found" << id;
        return {};
    }

    const auto doc = QJsonDocument::fromJson(file->data());
    if (doc.isArray()) {
        const auto array = JsonLdDocument::fromJson(doc.array());
        if (array.size() != 1) {
            qCWarning(Log) << "document meta data for" << id << "contains" << array.size() << "elements!";
            return {};
        }
        return array.at(0);
    } else if (doc.isObject()) {
        return JsonLdDocument::fromJsonSingular(doc.object());
    }
    return {};
}

QByteArray File::documentData(const QString &id) const
{
    const auto meta = documentInfo(id);
    if (!JsonLd::canConvert<CreativeWork>(meta)) {
        return {};
    }
    const auto fileName = JsonLd::convert<CreativeWork>(meta).name();

    const auto dir = dynamic_cast<const KArchiveDirectory*>(d->zipFile->directory()->entry(QLatin1String("documents/") + id));
    Q_ASSERT(dir); // checked by documentInfo already
    const auto file = dir->file(fileName);
    if (!file) {
        qCWarning(Log) << "document data not found" << id << fileName;
        return {};
    }
    return file->data();
}

QString File::normalizeDocumentFileName(const QString &name)
{
    auto fileName = name;
    // normalize the filename to something we can safely deal with
    auto idx = fileName.lastIndexOf(QLatin1Char('/'));
    if (idx >= 0) {
        fileName = fileName.mid(idx + 1);
    }
    fileName.replace(QLatin1Char('?'), QLatin1Char('_'));
    fileName.replace(QLatin1Char('*'), QLatin1Char('_'));
    fileName.replace(QLatin1Char(' '), QLatin1Char('_'));
    fileName.replace(QLatin1Char('\\'), QLatin1Char('_'));
    if (fileName.isEmpty() || fileName == QLatin1String("meta.json")) {
        fileName = QStringLiteral("file");
    }
    return fileName;
}

void File::addDocument(const QString &id, const QVariant &docInfo, const QByteArray &docData)
{
    Q_ASSERT(d->zipFile);
    if (!JsonLd::canConvert<CreativeWork>(docInfo)) {
        qCWarning(Log) << "Invalid document meta data" << docInfo;
        return;
    }
    if (id.isEmpty()) {
        qCWarning(Log) << "Trying to add a document with an empty identifier!";
        return;
    }

    const auto fileName = normalizeDocumentFileName(JsonLdDocument::readProperty(docInfo, "name").toString());
    auto normalizedDocInfo = docInfo;
    JsonLdDocument::writeProperty(normalizedDocInfo, "name", fileName);

    d->zipFile->writeFile(QLatin1String("documents/") + id + QLatin1String("/meta.json"),
                          QJsonDocument(JsonLdDocument::toJson(normalizedDocInfo)).toJson());
    d->zipFile->writeFile(QLatin1String("documents/") + id + QLatin1Char('/') + fileName, docData);
}

QVector<QString> File::listCustomData(const QString &scope) const
{
    Q_ASSERT(d->zipFile);
    const auto dir = dynamic_cast<const KArchiveDirectory*>(d->zipFile->directory()->entry(QLatin1String("custom/") + scope));
    if (!dir) {
        return {};
    }

    const auto entries = dir->entries();
    QVector<QString> res;
    res.reserve(entries.size());
    std::copy(entries.begin(), entries.end(), std::back_inserter(res));
    return res;
}

QByteArray File::customData(const QString& scope, const QString &id) const
{
    Q_ASSERT(d->zipFile);
    const auto dir = dynamic_cast<const KArchiveDirectory*>(d->zipFile->directory()->entry(QLatin1String("custom/") + scope));
    if (!dir) {
        return {};
    }

    const auto file = dir->file(id);
    if (!file) {
        qCDebug(Log) << "custom data not found" << scope << id;
        return {};
    }
    return file->data();
}

void File::addCustomData(const QString &scope, const QString &id, const QByteArray &data)
{
    Q_ASSERT(d->zipFile);
    d->zipFile->writeFile(QLatin1String("custom/") + scope + QLatin1Char('/') + id, data);
}
