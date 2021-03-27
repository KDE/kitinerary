/*
   SPDX-FileCopyrightText: 2017-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_EXTRACTOR_H
#define KITINERARY_EXTRACTOR_H

#include "kitinerary_export.h"
#include "extractorinput.h"

#include <QExplicitlySharedDataPointer>

#include <vector>

class QJsonObject;
class QString;

namespace KItinerary {

class ExtractorFilter;
class ExtractorPrivate;
class ExtractorRepositoryPrivate;

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
 * The following extractor types are supported (see also ExtractorInput::Type):
 * - \c Text: plain text, the argument to the script function is a single string.
 * - \c Html: HTML documents, the argument to the script function is a HtmlDocument instance.
 * - \c Pdf: PDF documents, the argument to the script function is a PdfDocument instance.
 * - \c PkPass: Apple Wallet passes, the argument to the script function is a KPkPass::BoardingPass instance.
 * - \c ICal: iCalendar events, the argument to the script function is a KCalendarCore::Event instance.
 *
 * Filter definitions have the following field:
 * - \c mimeType: The MIME type of the document part this filter can match against.
 * - \c field: The name of the field to match against. This can be a field id in a Apple Wallet pass,
 *      A MIME message header name, a property on a Json-LD object or an iCal calendar or event.
 *      For plain text or binary content, this is ignored.
 * - \c match: A regular expression that is matched against the specified value (see QRegularExpression).
 * - \c scope: Specifies how the filter should be applied relative to the document node that is being extracted.
 *      One of @c Current, @c Parent, @c Children, @c Ancestors, @c Descendants (@c Current is the default).
 *
 * Example:
 * @code
 * [
 *   {
 *     "type": "Pdf",
 *     "filter": [ { "field": "From", "match": "@swiss.com", "mimeType": "message/rfc822", "scope": "Ancestors" } ],
 *     "script": "swiss.js",
 *     "function": "parsePdf"
 *   },
 *   {
 *     "type": "PkPass",
 *     "filter": [ { "field": "passTypeIdentifier", "match": "pass.booking.swiss.com", "mimeType": "application/vnd.apple.pkpass", "scope": "Current" } ],
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
class KITINERARY_DEPRECATED_EXPORT Extractor
{
public:
    Extractor();
    Extractor(const Extractor&);
    Extractor(Extractor&&) noexcept;
    ~Extractor();
    Extractor& operator=(const Extractor&);
    Extractor& operator=(Extractor&&);

    /** Identifier for this extractor. */
    QString name() const;

    /** Data type this extractor can process. */
    [[deprecated("use mimeType()")]] ExtractorInput::Type type() const;
    /** The MIME type this script extractor supports. */
    QString mimeType() const;

    /** The JS script containing the code of the extractor. */
    QString scriptFileName() const;
    /** The JS function entry point for this extractor, @c main if empty. */
    QString scriptFunction() const;
    /** Returns the filters deciding whether this extractor should be applied. */
    const std::vector<ExtractorFilter> &filters() const;

    ///@cond internal
    /** Load meta data from the given JSON object. */
    bool load(const QJsonObject &obj, const QString &fileName, int index = -1);
    /** Save extractor meta data to a JSON object. */
    QJsonObject toJson() const;

    /** Source file name. */
    QString fileName() const;

    [[deprecated("use setMimeType()")]] void setType(ExtractorInput::Type type);
    void setMimeType(const QString &mimeType);
    void setScriptFileName(const QString &script);
    void setScriptFunction(const QString &func);
    void setFilters(std::vector<ExtractorFilter> filters);
    ///@endcond

private:
    QExplicitlySharedDataPointer<ExtractorPrivate> d;
    friend class ExtractorRepositoryPrivate;
};

}

#endif // KITINERARY_EXTRACTOR_H
