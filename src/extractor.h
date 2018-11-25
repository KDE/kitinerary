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

#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include "kitinerary_export.h"

#include <memory>
#include <vector>

class QJsonObject;
class QString;

namespace KItinerary {

class ExtractorFilter;
class ExtractorPrivate;

/** A single unstructured data extraction rule set.
 *
 * These rules are loaded from JSON meta-data files in a compiled-in qrc file,
 * or from $XDG_DATA_DIRS/kitinerary/extractors.
 *
 * @section extractor_metadata Meta Data Format
 *
 * The meta-data files either contain a single JSON object or an array of JSON objects
 * with the following content:
 * - \c type: The type of the extractor, \c text if not specified.
 * - \c filter: An array of filters that are used to select this extractor for a given input file.
 * - \c script: A JavaScript file to execute.
 * - \c function: The entry point in the above mentioned script, @c main if not specified.
 *
 * The following extractor types are supported:
 * - \c text: plain text, the argument to the script function is a single string.
 * - \c html: HTML documents, the argument to the script function is a HtmlDocument instance.
 * - \c pdf: PDF documents, the argument to the script function is a PdfDocument instance.
 * - \c pkpass: Apple Wallet passes, the argument to the script function is a KPkPass::BoardingPass instance.
 * - \c ical: iCalendar events, the argument to the script function is a KCalCore::Event instance.
 *
 * Filter definitions have the following field:
 * - \c type: The type of data this filter applies to, one of: @c Mime, @c PkPass, @c JsonLd, @c Barcode.
 *            Can often be omitted as it's auto-detected based on the following fields.
 * - \c header: A MIME message header name (valid and mandatory for type @c Mime).
 * - \c field: A field id in a Apple Wallet pass (valid and mandatory for type @c PkPass).
 * - \c property: A property on a Json-LD object (valid and mandatory for type @c JsonLd).
 * - \c match: A regular expression that is matched against the specified value (see QRegularExpression).
 *
 * Example:
 * @code
 * [
 *   {
 *     "type": "pdf",
 *     "filter": [ { "header": "From", "match": "@swiss.com" } ],
 *     "script": "swiss.js",
 *     "function": "parsePdf"
 *   },
 *   {
 *     "type": "pkpass",
 *     "filter": [ { "field": "passTypeIdentifier", "match": "pass.booking.swiss.com" } ],
 *     "script": "swiss.js",
 *     "function": "parsePkPass"
 *   }
 * ]
 * @endcode
 *
 * @section extractor_development Development
 *
 * For development it's convenient to symlink the extractors source folder to
 * $XDG_DATA_DIRS/kitinerary/extractors, so you can re-run a changed extractor
 * script without recompiling or restarting the application.
 *
 */
class KITINERARY_EXPORT Extractor
{
public:
    ///@cond internal
    Extractor();
    Extractor(const Extractor &) = delete;
    Extractor(Extractor &&);
    ~Extractor();
    ///@endcond

    /** Load meta data from the given JSON object. */
    bool load(const QJsonObject &obj, const QString &baseDir);

    /** Type of data this extractor can process. */
    enum Type {
        Text, ///< A plain-text extractor.
        Html, ///< A HTML document extractor.
        Pdf, ///< A PDF document extractor.
        PkPass, ///< A Apple Wallet pass extractor.
        ICal ///< iCalendar events extractor.
    };
    /** Returns the type of this extractor. */
    Type type() const;

    /** The JS script containing the code of the extractor. */
    QString scriptFileName() const;
    /** The JS function entry point for this extractor, @c main if empty. */
    QString scriptFunction() const;
    /** Returns the filters deciding whether this extractor should be applied. */
    const std::vector<ExtractorFilter> &filters() const;

private:
    std::unique_ptr<ExtractorPrivate> d;
};

}

#endif // EXTRACTOR_H
