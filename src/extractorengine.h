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

#ifndef EXTRACTORENGINE_H
#define EXTRACTORENGINE_H

#include "kitinerary_export.h"

#include <memory>

namespace KPkPass {
class Pass;
}

class QDateTime;
class QJsonArray;
class QString;

namespace KItinerary {

class Extractor;
class ExtractorEnginePrivate;
class PdfDocument;

/**
 * Unstructured data extraction engine.
 *
 * This will apply the given Extractor instance to the given input data
 * (plain text, HTML text, PDF documents, etc), and return the extracted
 * JSON-LD data.
 *
 * @section create_extractors Creating Extractors
 *
 * For adding custom extractors, two parts are needed:
 * - JSON meta-data describing the extractor and when to apply it, as described
 *   in the Extractor documentation.
 * - An extractor JavaScript file, compatible with QJSEngine.
 *
 * The extractor script will have access to API defined in the JsApi namespace:
 * - JsApi::Context: information about the input data being processed.
 * - JsApi::JsonLd: functions for generating JSON-LD data.
 * - JsApi::Barcode: barcode decoding functions.
 *
 * The entry point to the script is specified in the meta-data, its argument depends
 * on the extractor type:
 * - Plain text extractors are passed a string.
 * - PDF extractors are passed a PDFDocument instance allowing access to textual and
 *   image content.
 * - Apple Wallet pass extractors are passed a KPkPass::BoardingPass instance.
 */
class KITINERARY_EXPORT ExtractorEngine
{
public:
    ExtractorEngine();
    ~ExtractorEngine();
    ExtractorEngine(ExtractorEngine &&) noexcept;
    ExtractorEngine(const ExtractorEngine &) = delete;

    /** Resets the internal state, call before processing new input data. */
    void clear();

    /** Set the extractor to be run on the current data. */
    void setExtractor(const Extractor *extractor);

    /** The text to extract data from.
     *  Only considered for text extractors.
     */
    void setText(const QString &text);
    /** A PDF document to extract data from.
     *  Only considered for PDF or text extractors.
     */
    void setPdfDocument(PdfDocument *pdfDoc);
    /** The pkpass boarding pass to extract data from.
     *  Only considered for pkpass extractors.
     */
    void setPass(KPkPass::Pass *pass);

    /** The date the email containing the processed text was sent. */
    void setSenderDate(const QDateTime &dt);

    QJsonArray extract();

private:
    std::unique_ptr<ExtractorEnginePrivate> d;
};

}

#endif // EXTRACTORENGINE_H
