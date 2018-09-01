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
#include <vector>

namespace KPkPass {
class Pass;
}

class QDateTime;
class QJsonArray;
class QString;

namespace KItinerary {

class Extractor;
class ExtractorEnginePrivate;
class HtmlDocument;
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
 * @subsection extractor_api Extractor API
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
 *   If input is HTML or PDF, the string will be the text of the document stripped
 *   of all formatting etc.
 * - HTML extractors are passed a HtmlDocument instance allowing DOM-like access to
 *   the document structure.
 * - PDF extractors are passed a PDFDocument instance allowing access to textual and
 *   image content.
 * - Apple Wallet pass extractors are passed a KPkPass::BoardingPass instance.
 *
 * These functions should return an object or an array of objects following the JSON-LD
 * format defined on schema.org. JsApi::JsonLd provides helper functions to build such
 * objects. If @c null or an empty array is returned, the next applicable extractor is
 * run.
 *
 * Returned objects are then passed through ExtractorPostprocessor which will normalize,
 * augment and validate the data. This can greatly simplify the extraction, as for example
 * the expansion of an IATA BCBP ticket token already fills most key properties of a flight
 * reservation automatically.
 *
 * @subsection extractor_tools Development Tools
 *
 * For interactive testing during development of new extractors, it is recommended to
 * link (or copy) the JSON meta data and JavaScript code files to the search path for
 * Extractor meta data.
 *
 * Additionally, there's an interactive testing and inspection tool called @c kitinerary-workbench
 * (see https://phabricator.kde.org/source/kitinerary-workbench/).
 *
 * @subsection extractor_testing Automated Testing
 *
 * There are a few unit tests for extractors in the kitinerary repository (see unstructureddataextractortest.cpp),
 * however the majority of real-world test data cannot be shared this way, due to privacy
 * and copyright issues (e.g. PDFs containing copyrighted vendor logos and user credit card details).
 * Therefore there is also support for testing against external data (see extractortest.cpp).
 *
 * External test data is assumed to be in a folder named @c kitinerary-tests next to the @c kitinerary
 * source folder. The test program searches this folder recursively for folders with the following content
 * and attempts to extract data from each test file in there.
 *
 * - @c context.eml: MIME message header data specifying the context in which the test data
 *   was received. This typically only needs a @c From: and @c Date: line, but can even be
 *   entirely empty (but existing) for structured data that does not need a custom extractor.
 *   This context information is applied to all tests in this folder.
 * - @c <testname>.[txt|html|pdf|pkpass]: The input test data.
 * - @c <testname.extension>.json: The expected JSON-LD output. If this file doesn't
 *   exists it is created by the test program.
 * - @c <testname.extension>.skip: If this file is present the corresponding test
 *   is skipped.
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

    /** Set the extractors to be run on the current data. */
    void setExtractors(std::vector<const Extractor*> &&extractors);
    [[deprecated("use setExtractors instead")]] void setExtractor(const Extractor *extractor);

    /** The text to extract data from.
     *  Only considered for text extractors.
     */
    void setText(const QString &text);
    /** A HTML document to extract data from.
     *  Only considered for HTML and text extractors.
     */
    void setHtmlDocument(HtmlDocument *htmlDoc);
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

    /** Perform the actual extration, and return the JSON-LD data
     *  that has been found.
     */
    QJsonArray extract();

private:
    std::unique_ptr<ExtractorEnginePrivate> d;
};

}

#endif // EXTRACTORENGINE_H
