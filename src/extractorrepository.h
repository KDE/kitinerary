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

#ifndef EXTRACTORREPOSITORY_H
#define EXTRACTORREPOSITORY_H

#include "kitinerary_export.h"

#include <QSharedPointer>

#include <memory>
#include <vector>

namespace KCalendarCore {
class Calendar;
}

namespace KMime {
class Content;
}

namespace KPkPass {
class Pass;
}

class QJsonArray;
class QString;

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
    std::vector<Extractor> extractorsForMessage(KMime::Content *part) const;
    /** Finds matching extractors for the given pkpass boarding pass. */
    std::vector<Extractor> extractorsForPass(KPkPass::Pass *pass) const;
    /** Finds matching extractors for the given JSON-LD data provided by generic extractors. */
    std::vector<Extractor> extractorsForJsonLd(const QJsonArray &data) const;
    /** Finds matching extractors for the given barcode string. */
    std::vector<Extractor> extractorsForBarcode(const QString &code) const;
    /** Find matching extractors for the given iCal calendar. */
    std::vector<Extractor> extractorsForCalendar(const QSharedPointer<KCalendarCore::Calendar> &cal) const;
    /** Returns the extractor with the given identifier. */
    Extractor extractor(const QString &name) const;

private:
    ExtractorRepositoryPrivate* d;
};

}

#endif // EXTRACTORREPOSITORY_H
