/*
   SPDX-FileCopyrightText: 2017-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include <QStringList>
#include <memory>
#include <vector>

namespace KCalendarCore {
class Calendar;
class Event;
}

namespace KMime {
class Content;
}

namespace KPkPass {
class Pass;
}

class QJsonArray;
class QJsonValue;
class QString;

namespace KItinerary {

class AbstractExtractor;
class Extractor;
class ExtractorDocumentNode;
class ExtractorRepositoryPrivate;
class ScriptExtractor;

/**
 *  Collection of all known data extractors.
 *  This class is usually not used directly, but as an implementation detail to KItinerary::ExtractorEngine.
 *
 *  See KItinerary::Extractor on where this loads its content from.
 *  @internal This API is only exported for developer tooling.
 *  @see KItinerary::ScriptExtractor.
 */
class KITINERARY_EXPORT ExtractorRepository
{
public:
    ExtractorRepository();
    ~ExtractorRepository();
    ExtractorRepository(ExtractorRepository &&) noexcept;
    ExtractorRepository(const ExtractorRepository &) = delete;

    /** Reload the extractor repository.
     *  Not needed during normal operations, this is mainly for tooling.
     */
    void reload();

    /** All known extractors. */
    const std::vector<std::unique_ptr<AbstractExtractor>>& extractors() const;
    [[deprecated("use extractors()")]] const std::vector<Extractor>& allExtractors() const;

    /** Finds matching extractors for the given document node. */
    void extractorsForNode(const ExtractorDocumentNode &node, std::vector<const AbstractExtractor*> &extractors) const;

    /** Finds matching extractors for the given message part. */
    [[deprecated("use extractorsForNode")]] void extractorsForMessage(KMime::Content *part, std::vector<Extractor> &extractors) const;
    /** Finds matching extractors for the given pkpass boarding pass. */
    [[deprecated("use extractorsForNode")]] void extractorsForPass(KPkPass::Pass *pass, std::vector<Extractor> &extractors) const;
    /** Finds matching extractors for the given JSON-LD data provided by generic extractors. */
    [[deprecated("use extractorsForNode")]] void extractorsForJsonLd(const QJsonArray &data, std::vector<Extractor> &extractors) const;
    /** Finds matching extractors for the given barcode string. */
    [[deprecated("use extractorsForNode")]] void extractorsForBarcode(const QString &code, std::vector<Extractor> &extractors) const;
    /** Find matching extractors for the given iCal calendar. */
    [[deprecated("use extractorsForNode")]] void extractorsForCalendar(const KCalendarCore::Calendar *cal, std::vector<Extractor> &extractors) const;
    /** Find matching extractors for the given iCal event. */
    [[deprecated("use extractorsForNode")]] void extractorsForEvent(const KCalendarCore::Event *event, std::vector<Extractor> &extractors) const;
    /** Find matching extractors for the given content. */
    [[deprecated("use extractorsForNode")]] void extractorsForContent(const QString &content, std::vector<Extractor> &extractors) const;
    /** Returns the extractor with the given identifier. */
    const AbstractExtractor* extractorByName(QStringView name) const;
    [[deprecated("use extractorByName")]] Extractor extractor(const QString &name) const;

    /** Returns the list of additional search paths for extractor scripts. */
    QStringList additionalSearchPaths() const;
    /** Sets additional search paths to look for extractors. */
    void setAdditionalSearchPaths(const QStringList &searchPaths);

    ///@cond internal
    /** JSON serialization of @p extractor, including all other Extractor definitions in the same file, if any.
     *  Only for tooling, do not use otherwise.
     */
    QJsonValue extractorToJson(const Extractor &extractor) const;
    QJsonValue extractorToJson(const ScriptExtractor *extractor) const;
    ///@endcond

private:
    ExtractorRepositoryPrivate* d;
};

}

