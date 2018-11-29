/*
   Copyright (c) 2017 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#include "extractorrepository.h"
#include "extractor.h"
#include "extractorfilter.h"
#include "logging.h"

#include <KMime/Content>

#include <KPkPass/Pass>

#include <QDirIterator>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

using namespace KItinerary;

static void initResources() // must be outside of a namespace
{
    Q_INIT_RESOURCE(extractors);
}

namespace KItinerary {
class ExtractorRepositoryPrivate {
public:
    ExtractorRepositoryPrivate();
    void loadExtractors();

    std::vector<Extractor> m_extractors;
};
}

ExtractorRepositoryPrivate::ExtractorRepositoryPrivate()
{
    initResources();
    loadExtractors();
}


ExtractorRepository::ExtractorRepository()
{
    static ExtractorRepositoryPrivate repo;
    d = &repo;
}

ExtractorRepository::~ExtractorRepository() = default;
ExtractorRepository::ExtractorRepository(KItinerary::ExtractorRepository &&) noexcept = default;

std::vector<const Extractor *> ExtractorRepository::extractorsForMessage(KMime::Content *part) const
{
    std::vector<const Extractor *> v;
    if (!part) {
        return v;
    }

    for (auto it = d->m_extractors.begin(), end = d->m_extractors.end(); it != end; ++it) {
        for (const auto &filter : (*it).filters()) {
            if (filter.type() != ExtractorFilter::Mime) {
                continue;
            }
            auto header = part->headerByType(filter.fieldName());
            auto ancestor = part;
            while (!header && ancestor->parent()) {
                ancestor = ancestor->parent();
                header = ancestor->headerByType(filter.fieldName());
            }
            if (!header) {
                continue;
            }
            const auto headerData = header->asUnicodeString();
            if (filter.matches(headerData)) {
                v.push_back(&(*it));
                break;
            }
        }
    }

    return v;
}

std::vector<const Extractor *> ExtractorRepository::extractorsForPass(KPkPass::Pass *pass) const
{
    std::vector<const Extractor *> v;
    if (pass->type() != KPkPass::Pass::BoardingPass && pass->type() != KPkPass::Pass::EventTicket) {
        return v;
    }

    for (auto it = d->m_extractors.begin(), end = d->m_extractors.end(); it != end; ++it) {
        if ((*it).type() != Extractor::PkPass) {
            continue;
        }
        for (const auto &filter : (*it).filters()) {
            if (filter.type() != ExtractorFilter::PkPass) {
                continue;
            }

            QString value;
            if (strcmp(filter.fieldName(), "passTypeIdentifier") == 0) {
                value = pass->passTypeIdentifier();
            } else {
                continue;
            }
            if (filter.matches(value)) {
                v.push_back(&(*it));
                break;
            }
        }
    }

    return v;
}

static QString providerId(const QJsonObject &res)
{
    if (res.value(QLatin1String("@type")).toString() == QLatin1String("FlightReservation")) {
        return res.value(QLatin1String("reservationFor")).toObject().value(QLatin1String("airline")).toObject().value(QLatin1String("iataCode")).toString();
    }
    if (res.value(QLatin1String("@type")).toString() == QLatin1String("TrainReservation")) {
        return res.value(QLatin1String("reservationFor")).toObject().value(QLatin1String("provider")).toObject().value(QLatin1String("identifier")).toString();
    }

    return {};
}

std::vector<const Extractor *> ExtractorRepository::extractorsForJsonLd(const QJsonArray &data) const
{
    std::vector<const Extractor *> v;

    for (const auto &val : data) {
        const auto id = providerId(val.toObject());
        for (auto it = d->m_extractors.begin(), end = d->m_extractors.end(); it != end; ++it) {
            for (const auto &filter : (*it).filters()) {
                if (filter.type() != ExtractorFilter::JsonLd) {
                    continue;
                }
                if (strcmp(filter.fieldName(), "provider") != 0) {
                    continue;
                }
                if (filter.matches(id)) {
                    v.push_back(&(*it));
                    break;
                }
            }
        }
    }

    return v;
}

std::vector<const Extractor *> ExtractorRepository::extractorsForBarcode(const QString &code) const
{
    std::vector<const Extractor *> v;

    for (auto it = d->m_extractors.begin(), end = d->m_extractors.end(); it != end; ++it) {
        for (const auto &filter : (*it).filters()) {
            if (filter.type() == ExtractorFilter::Barcode && filter.matches(code)) {
                v.push_back(&(*it));
                break;
            }
        }
    }

    return v;
}

void ExtractorRepositoryPrivate::loadExtractors()
{
    auto searchDirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    searchDirs += QStringLiteral(":/org.kde.pim");

    for (const auto &dir : qAsConst(searchDirs)) {
        QDirIterator it(dir + QStringLiteral("/kitinerary/extractors"), {QStringLiteral("*.json")}, QDir::Files);
        while (it.hasNext()) {
            const auto fileName = it.next();
            QFile file(fileName);
            if (!file.open(QFile::ReadOnly)) {
                continue;
            }

            QJsonParseError error;
            const auto doc = QJsonDocument::fromJson(file.readAll(), &error);
            if (doc.isNull()) {
                qCWarning(Log) << "Extractor loading error:" << fileName << error.errorString();
                continue;
            }

            QFileInfo fi(fileName);

            if (doc.isObject()) {
                const auto obj = doc.object();
                Extractor e;
                if (e.load(obj, fi.absolutePath())) {
                    m_extractors.push_back(std::move(e));
                }
            } else if (doc.isArray()) {
                for (const auto &v : doc.array()) {
                Extractor e;
                    if (e.load(v.toObject(), fi.absolutePath())) {
                        m_extractors.push_back(std::move(e));
                    }
                }
            } else {
                qCWarning(Log) << "Invalid extractor meta-data:" << fileName;
                continue;
            }
        }
    }
}
