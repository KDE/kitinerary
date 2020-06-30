/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef EXTRACTORREPOSITORY_H
#define EXTRACTORREPOSITORY_H

#include "kitinerary_export.h"

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
class QStringList;

namespace KItinerary {

class Extractor;
class ExtractorRepositoryPrivate;

/** Collection of all unstructured data extractor rule sets.
 *
 *  See KItinerary::Extractor on where this loads its content from.
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
    const std::vector<Extractor>& allExtractors() const;

    /** Finds matching extractors for the given message part. */
    void extractorsForMessage(KMime::Content *part, std::vector<Extractor> &extractors) const;
    /** Finds matching extractors for the given pkpass boarding pass. */
    void extractorsForPass(KPkPass::Pass *pass, std::vector<Extractor> &extractors) const;
    /** Finds matching extractors for the given JSON-LD data provided by generic extractors. */
    void extractorsForJsonLd(const QJsonArray &data, std::vector<Extractor> &extractors) const;
    /** Finds matching extractors for the given barcode string. */
    void extractorsForBarcode(const QString &code, std::vector<Extractor> &extractors) const;
    /** Find matching extractors for the given iCal calendar. */
    void extractorsForCalendar(const KCalendarCore::Calendar *cal, std::vector<Extractor> &extractors) const;
    /** Find matching extractors for the given iCal event. */
    void extractorsForEvent(const KCalendarCore::Event *event, std::vector<Extractor> &extractors) const;
    /** Find matching extractors for the given content. */
    void extractorsForContent(const QString &content, std::vector<Extractor> &extractors) const;
    /** Returns the extractor with the given identifier. */
    Extractor extractor(const QString &name) const;

    /** Returns the list of additional search paths for extractor scripts. */
    QStringList additionalSearchPaths() const;
    /** Sets additional search paths to look for extractors. */
    void setAdditionalSearchPaths(const QStringList &searchPaths);

    ///@cond internal
    /** JSON serialization of @p extractor, including all other Extractor definitions in the same file, if any.
     *  Only for tooling, do not use otherwise.
     */
    QJsonValue extractorToJson(const Extractor &extractor) const;
    ///@endcond

private:
    ExtractorRepositoryPrivate* d;
};

}

#endif // EXTRACTORREPOSITORY_H
