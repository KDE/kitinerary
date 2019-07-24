/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "file.h"
#include "jsonlddocument.h"
#include "logging.h"

#include <KPkPass/Pass>

#include <KArchive/KZip>

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
    d->zipFile.reset(new KZip(d->fileName));
    if (!d->zipFile->open(mode == File::Write ? QIODevice::WriteOnly : QIODevice::ReadOnly)) {
        qCWarning(Log) << d->zipFile->errorString() << d->fileName;
        return false;
    }

    return true;
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
        return JsonLdDocument::fromJson(doc.object());
    }
    return {};
}

void File::addReservation(const QVariant &res)
{
    Q_ASSERT(d->zipFile);
    d->zipFile->writeFile(QLatin1String("reservations/") + QUuid::createUuid().toString() + QLatin1String(".json"),
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
            passIds.push_back(entry + QLatin1Char('/') + subEntry.leftRef(subEntry.size() - 7));
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
    Q_ASSERT(d->zipFile);
    const auto id = passId(pass);
    d->zipFile->writeFile(QLatin1String("passes/") + id + QLatin1String(".pkpass"), rawData);
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
