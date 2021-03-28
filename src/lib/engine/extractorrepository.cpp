/*
   SPDX-FileCopyrightText: 2017-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-kitinerary.h"
#include "extractorrepository.h"

#include "extractor.h"
#include "logging.h"
#include "extractors/iatabcbpextractor.h"

#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorDocumentProcessor>
#include <KItinerary/ExtractorFilter>
#include <KItinerary/ScriptExtractor>

#ifdef HAVE_KCAL
#include <KCalendarCore/Calendar>
#include <KCalendarCore/Event>
#endif

#include <KMime/Content>

#include <KPkPass/Pass>

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
    void addExtractor(Extractor &&e);
    void addExtractor(std::unique_ptr<AbstractExtractor> &&e);
    void extractorForTypeAndContent(ExtractorInput::Type type, const QString &content, std::vector<Extractor> &extractors) const;
    static void insertExtractor(const Extractor &ext, std::vector<Extractor> &extractors);

    std::vector<Extractor> m_extractors;
    std::vector<std::unique_ptr<AbstractExtractor>> m_extractorsNew;
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
    addExtractor(std::make_unique<IataBcbpExtractor>());
}

void ExtractorRepositoryPrivate::extractorForTypeAndContent(ExtractorInput::Type type, const QString &content, std::vector<Extractor> &extractors) const
{
    for (auto it = m_extractors.begin(), end = m_extractors.end(); it != end; ++it) {
        for (const auto &filter : (*it).filters()) {
            if (filter.type() == type && filter.matches(content)) {
                insertExtractor(*it, extractors);
                break;
            }
        }
    }
}

// approximate set behavior on extractors, using the d pointers as a quick way to ensure uniqueness
void ExtractorRepositoryPrivate::insertExtractor(const Extractor &ext, std::vector<Extractor> &extractors)
{
    const auto it = std::lower_bound(extractors.begin(), extractors.end(), ext, [](const auto &lhs, const auto &rhs) {
        return lhs.d.constData() < rhs.d.constData();
    });
    if (it != extractors.end() && (*it).d.constData() == ext.d.constData()) {
        return;
    }
    extractors.insert(it, ext);
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
    d->m_extractorsNew.clear();
    d->loadAll();
}

const std::vector<std::unique_ptr<AbstractExtractor>>& ExtractorRepository::extractors() const
{
    return d->m_extractorsNew;
}

const std::vector<Extractor>& ExtractorRepository::allExtractors() const
{
    return d->m_extractors;
}

void ExtractorRepository::extractorsForMessage(KMime::Content *part, std::vector<Extractor> &extractors) const
{
    if (!part) {
        return;
    }

    for (auto it = d->m_extractors.begin(), end = d->m_extractors.end(); it != end; ++it) {
        for (const auto &filter : (*it).filters()) {
            if (filter.type() != ExtractorInput::Email) {
                continue;
            }
            auto header = part->headerByType(filter.fieldName().toUtf8().constData());
            auto ancestor = part;
            while (!header && ancestor->parent()) {
                ancestor = ancestor->parent();
                header = ancestor->headerByType(filter.fieldName().toUtf8().constData());
            }
            if (!header) {
                continue;
            }
            const auto headerData = header->asUnicodeString();
            if (filter.matches(headerData)) {
                ExtractorRepositoryPrivate::insertExtractor(*it, extractors);
                break;
            }
        }
    }
}

void ExtractorRepository::extractorsForPass(KPkPass::Pass *pass, std::vector<Extractor> &extractors) const
{
    if (pass->type() != KPkPass::Pass::BoardingPass && pass->type() != KPkPass::Pass::EventTicket) {
        return;
    }

    for (auto it = d->m_extractors.begin(), end = d->m_extractors.end(); it != end; ++it) {
        if ((*it).type() != ExtractorInput::PkPass) {
            continue;
        }
        for (const auto &filter : (*it).filters()) {
            if (filter.type() != ExtractorInput::PkPass) {
                continue;
            }

            QString value;
            if (filter.fieldName() == QLatin1String("passTypeIdentifier")) {
                value = pass->passTypeIdentifier();
            } else {
                continue;
            }
            if (filter.matches(value)) {
                ExtractorRepositoryPrivate::insertExtractor(*it, extractors);
                break;
            }
        }
    }
}

static QString valueForJsonPath(const QJsonObject &obj, const QString &path)
{
    const auto pathSections = path.splitRef(QLatin1Char('.'));
    QJsonValue v(obj);
    for (const auto &pathSection : pathSections) {
        if (!v.isObject()) {
            return {};
        }
        v = v.toObject().value(pathSection.toString());
    }
    return v.toString();
}

void ExtractorRepository::extractorsForJsonLd(const QJsonArray &data, std::vector<Extractor> &extractors) const
{
    for (const auto &val : data) {
        for (auto it = d->m_extractors.begin(), end = d->m_extractors.end(); it != end; ++it) {
            for (const auto &filter : (*it).filters()) {
                if (filter.type() != ExtractorInput::JsonLd) {
                    continue;
                }
                const auto value = valueForJsonPath(val.toObject(), filter.fieldName());
                if (value.isEmpty()) {
                    continue;
                }
                if (filter.matches(value)) {
                    ExtractorRepositoryPrivate::insertExtractor(*it, extractors);
                    break;
                }
            }
        }
    }
}

void ExtractorRepository::extractorsForBarcode(const QString &code, std::vector<Extractor> &extractors) const
{
    d->extractorForTypeAndContent(ExtractorInput::Barcode, code, extractors);
}

#ifdef HAVE_KCAL
void ExtractorRepository::extractorsForCalendar(const KCalendarCore::Calendar *cal, std::vector<Extractor> &extractors) const
{
    for (auto it = d->m_extractors.begin(), end = d->m_extractors.end(); it != end; ++it) {
        for (const auto &filter : (*it).filters()) {
            if (filter.type() != ExtractorInput::ICal) {
                continue;
            }

            const auto value = cal->property(filter.fieldName().toUtf8().constData());
            if (filter.matches(value.toString())) {
                ExtractorRepositoryPrivate::insertExtractor(*it, extractors);
                break;
            }
        }
    }
}

void ExtractorRepository::extractorsForEvent(const KCalendarCore::Event *event, std::vector<Extractor> &extractors) const
{
    for (auto it = d->m_extractors.begin(), end = d->m_extractors.end(); it != end; ++it) {
        for (const auto &filter : (*it).filters()) {
            if (filter.type() != ExtractorInput::ICal) {
                continue;
            }

            const auto propIdx = KCalendarCore::Event::staticMetaObject.indexOfProperty(filter.fieldName().toUtf8().constData());
            if (propIdx < 0) {
                continue;
            }
            const auto prop = KCalendarCore::Event::staticMetaObject.property(propIdx);
            const auto value = prop.readOnGadget(event);
            if (filter.matches(value.toString())) {
                ExtractorRepositoryPrivate::insertExtractor(*it, extractors);
                break;
            }
        }
    }
}
#endif

void ExtractorRepository::extractorsForNode(const ExtractorDocumentNode &node, std::vector<const AbstractExtractor*> &extractors) const
{
    if (node.isNull()) {
        return;
    }

    for (const auto &extractor : d->m_extractorsNew) {
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

void ExtractorRepository::extractorsForContent(const QString &content, std::vector<Extractor> &extractors) const
{
    d->extractorForTypeAndContent(ExtractorInput::Text, content, extractors);
}

const AbstractExtractor* ExtractorRepository::extractorByName(QStringView name) const
{
    auto it = std::lower_bound(d->m_extractorsNew.begin(), d->m_extractorsNew.end(), name, [](const auto &lhs, auto rhs) {
        return lhs->name() < rhs;
    });
    if (it != d->m_extractorsNew.end() && (*it)->name() == name) {
        return (*it).get();
    }
    return {};
}

Extractor ExtractorRepository::extractor(const QString &name) const
{
    auto it = std::lower_bound(d->m_extractors.begin(), d->m_extractors.end(), name, [](const auto &lhs, const auto &rhs) {
        return lhs.name() < rhs;
    });
    if (it != d->m_extractors.end() && (*it).name() == name) {
        return *it;
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

    for (const auto &dir : qAsConst(searchDirs)) {
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
                Extractor e;
                if (e.load(obj, fi.canonicalFilePath())) {
                    addExtractor(std::move(e));
                }

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
                    Extractor e;
                    if (e.load(v.toObject(), fi.canonicalFilePath(), extractorArray.size() == 1 ? -1 : i)) {
                        addExtractor(std::move(e));
                    }

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

void ExtractorRepositoryPrivate::addExtractor(Extractor &&e)
{
    auto it = std::lower_bound(m_extractors.begin(), m_extractors.end(), e, [](const auto &lhs, const auto &rhs) {
        return lhs.name() < rhs.name();
    });
    if (it == m_extractors.end() || (*it).name() != e.name()) {
        m_extractors.insert(it, std::move(e));
    }
}

void ExtractorRepositoryPrivate::addExtractor(std::unique_ptr<AbstractExtractor> &&e)
{
    auto it = std::lower_bound(m_extractorsNew.begin(), m_extractorsNew.end(), e, [](const auto &lhs, const auto &rhs) {
        return lhs->name() < rhs->name();
    });
    if (it == m_extractorsNew.end() || (*it)->name() != e->name()) {
        m_extractorsNew.insert(it, std::move(e));
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

QJsonValue ExtractorRepository::extractorToJson(const Extractor &extractor) const
{
    QJsonArray a;
    bool added = false;
    for (const auto &e : d->m_extractors) {
        if (e.fileName() != extractor.fileName()) {
            continue;
        }
        if (extractor.name() == e.name()) {
            a.push_back(extractor.toJson());
            added = true;
        } else {
            a.push_back(e.toJson());
        }
    }
    if (!added) {
        a.push_back(extractor.toJson());
    }

    if (a.size() == 1) {
        return a.at(0);
    }
    return a;
}

QJsonValue ExtractorRepository::extractorToJson(const ScriptExtractor *extractor) const
{
    QJsonArray a;
    bool added = false;
    for (const auto &ext : d->m_extractorsNew) {
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
