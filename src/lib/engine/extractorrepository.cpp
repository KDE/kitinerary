/*
   SPDX-FileCopyrightText: 2017-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-kitinerary.h"
#include "extractorrepository.h"

#include "logging.h"
#include "extractors/genericboardingpassextractor.h"

#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorDocumentProcessor>
#include <KItinerary/ExtractorFilter>
#include <KItinerary/ScriptExtractor>

#include <QDirIterator>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaProperty>
#include <QStandardPaths>

using namespace KItinerary;

static void initResources() // must be outside of a namespace
{
    Q_INIT_RESOURCE(extractors);
    Q_INIT_RESOURCE(vdv_certs);
}

namespace KItinerary {
class ExtractorRepositoryPrivate {
public:
    ExtractorRepositoryPrivate();
    void loadAll();
    void initBuiltInExtractors();
    void loadScriptExtractors();
    void addExtractor(std::unique_ptr<AbstractExtractor> &&e);

    std::vector<std::unique_ptr<AbstractExtractor>> m_extractors;
    QStringList m_extraSearchPaths;
};
}

ExtractorRepositoryPrivate::ExtractorRepositoryPrivate()
{
    initResources();
    loadAll();
}

void ExtractorRepositoryPrivate::loadAll()
{
    initBuiltInExtractors();
    loadScriptExtractors();
}

void ExtractorRepositoryPrivate::initBuiltInExtractors()
{
    addExtractor(std::make_unique<GenericBoardingPassExtractor>());
}

ExtractorRepository::ExtractorRepository()
{
    static ExtractorRepositoryPrivate repo;
    d = &repo;
}

ExtractorRepository::~ExtractorRepository() = default;
ExtractorRepository::ExtractorRepository(KItinerary::ExtractorRepository &&) noexcept = default;

void ExtractorRepository::reload()
{
    d->m_extractors.clear();
    d->loadAll();
}

const std::vector<std::unique_ptr<AbstractExtractor>>& ExtractorRepository::extractors() const
{
    return d->m_extractors;
}

void ExtractorRepository::extractorsForNode(const ExtractorDocumentNode &node, std::vector<const AbstractExtractor*> &extractors) const
{
    if (node.isNull()) {
        return;
    }

    for (const auto &extractor : d->m_extractors) {
        if (extractor->canHandle(node)) {
            // while we only would add each extractor at most once, some of them might already be in the list, so de-duplicate
            const auto it = std::lower_bound(extractors.begin(), extractors.end(), extractor.get(), [](auto lhs, auto rhs) {
                return lhs < rhs;
            });
            if (it == extractors.end() || (*it) != extractor.get()) {
                extractors.insert(it, extractor.get());
            }
        }
    }
}

const AbstractExtractor* ExtractorRepository::extractorByName(QStringView name) const
{
    auto it = std::lower_bound(d->m_extractors.begin(), d->m_extractors.end(), name, [](const auto &lhs, auto rhs) {
        return lhs->name() < rhs;
    });
    if (it != d->m_extractors.end() && (*it)->name() == name) {
        return (*it).get();
    }
    return {};
}

void ExtractorRepositoryPrivate::loadScriptExtractors()
{
    auto searchDirs = m_extraSearchPaths;
    const auto qsp = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const auto &p : qsp) {
        searchDirs.push_back(p + QLatin1String("/kitinerary/extractors"));
    }
    searchDirs += QStringLiteral(":/org.kde.pim/kitinerary/extractors");

    for (const auto &dir : std::as_const(searchDirs)) {
        QDirIterator it(dir, QDir::Files);
        while (it.hasNext()) {
            const auto fileName = it.next();
            if (!fileName.endsWith(QLatin1String(".json"))) {
                continue;
            }

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
            const auto name = fi.fileName().left(fi.fileName().size() - 5);

            if (doc.isObject()) {
                const auto obj = doc.object();
                auto ext = std::make_unique<ScriptExtractor>();
                if (ext->load(obj, fi.canonicalFilePath())) {
                    addExtractor(std::move(ext));
                } else {
                    qCWarning(Log) << "failed to load extractor:" << fi.canonicalFilePath();
                }
            } else if (doc.isArray()) {
                const auto extractorArray = doc.array();
                int i = 0;
                for (const auto &v : extractorArray) {
                    auto ext = std::make_unique<ScriptExtractor>();
                    if (ext->load(v.toObject(), fi.canonicalFilePath(), extractorArray.size() == 1 ? -1 : i)) {
                        addExtractor(std::move(ext));
                    } else {
                        qCWarning(Log) << "failed to load extractor:" << fi.canonicalFilePath();
                    }
                    ++i;
                }
            } else {
                qCWarning(Log) << "Invalid extractor meta-data:" << fileName;
                continue;
            }
        }
    }
}

void ExtractorRepositoryPrivate::addExtractor(std::unique_ptr<AbstractExtractor> &&e)
{
    auto it = std::lower_bound(m_extractors.begin(), m_extractors.end(), e, [](const auto &lhs, const auto &rhs) {
        return lhs->name() < rhs->name();
    });
    if (it == m_extractors.end() || (*it)->name() != e->name()) {
        m_extractors.insert(it, std::move(e));
    }
}

QStringList ExtractorRepository::additionalSearchPaths() const
{
    return d->m_extraSearchPaths;
}

void ExtractorRepository::setAdditionalSearchPaths(const QStringList& searchPaths)
{
    d->m_extraSearchPaths = searchPaths;
}

QJsonValue ExtractorRepository::extractorToJson(const ScriptExtractor *extractor) const
{
    QJsonArray a;
    bool added = false;
    for (const auto &ext : d->m_extractors) {
        auto e = dynamic_cast<ScriptExtractor*>(ext.get());
        if (!e || e->fileName() != extractor->fileName()) {
            continue;
        }
        if (extractor->name() == e->name()) {
            a.push_back(extractor->toJson());
            added = true;
        } else {
            a.push_back(e->toJson());
        }
    }
    if (!added) {
        a.push_back(extractor->toJson());
    }

    if (a.size() == 1) {
        return a.at(0);
    }
    return a;
}
