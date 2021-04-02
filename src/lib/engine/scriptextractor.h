/*
   SPDX-FileCopyrightText: 2017-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "abstractextractor.h"

#include <memory>
#include <vector>

class QJsonObject;
class QString;

namespace KItinerary {
class ExtractorFilter;
class ScriptExtractorPrivate;

/** A single unstructured data extraction rule set.
 *
 * These rules are loaded from JSON meta-data files in a compiled-in qrc file,
 * or from $XDG_DATA_DIRS/kitinerary/extractors.
 *
 * @section extractor_metadata Meta Data Format
 *
 * The meta-data files either contain a single JSON object or an array of JSON objects
 * with the following content:
 * - \c mimeType: The MIME type of the extractor, \c text if not specified.
 * - \c filter: An array of filters that are used to select this extractor for a given input file.
 * - \c script: A JavaScript file to execute.
 * - \c function: The entry point in the above mentioned script, @c main if not specified.
 *
 * The following extractor types are supported:
 * - \c text/plain: plain text, the argument to the script function is a single string.
 * - \c text/html: HTML documents, the argument to the script function is a KItinerary::HtmlDocument instance.
 * - \c application/pdf: PDF documents, the argument to the script function is a KItinerary::PdfDocument instance.
 * - \c application/vnd.apple.pkpass: Apple Wallet passes, the argument to the script function is a KPkPass::Pass instance.
 * - \c internal/event: iCalendar events, the argument to the script function is a KCalendarCore::Event instance.
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
 *     "mimeType": "application/pdf",
 *     "filter": [ { "field": "From", "match": "@swiss.com", "mimeType": "message/rfc822", "scope": "Ancestors" } ],
 *     "script": "swiss.js",
 *     "function": "parsePdf"
 *   },
 *   {
 *     "mimeType": "application/vnd.apple.pkpass",
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
class KITINERARY_EXPORT ScriptExtractor : public AbstractExtractor
{
public:
    explicit ScriptExtractor();
    ~ScriptExtractor();

    QString name() const override;
    bool canHandle(const ExtractorDocumentNode &node) const override;
    ExtractorResult extract(const ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;

    /** The JS script containing the code of the extractor. */
    QString scriptFileName() const;
    /** The JS function entry point for this extractor, @c main if empty. */
    QString scriptFunction() const;
    /** Mime type this script extractor supports. */
    QString mimeType() const;
    /** Returns the filters deciding whether this extractor should be applied. */
    const std::vector<ExtractorFilter> &filters() const;

    ///@cond internal
    /** Load meta data from the given JSON object. */
    bool load(const QJsonObject &obj, const QString &fileName, int index = -1);
    /** Save extractor meta data to a JSON object. */
    QJsonObject toJson() const;

    /** Source file name. */
    QString fileName() const;

    void setMimeType(const QString &mimeType);
    void setScriptFileName(const QString &script);
    void setScriptFunction(const QString &func);
    void setFilters(std::vector<ExtractorFilter> &&filters);
    void setFilters(const std::vector<ExtractorFilter> &filters);
    ///@endcond

private:
    std::unique_ptr<ScriptExtractorPrivate> d;
};

}

