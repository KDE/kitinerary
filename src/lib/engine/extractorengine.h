/*
   SPDX-FileCopyrightText: 2017-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include <QString>

#include <memory>
#include <vector>

class QByteArray;
class QDateTime;
class QJsonArray;
class QVariant;

namespace KItinerary {

class AbstractExtractor;
class BarcodeDecoder;
class ExtractorDocumentNode;
class ExtractorDocumentNodeFactory;
class ExtractorEnginePrivate;
class ExtractorRepository;
class ExtractorScriptEngine;

/**
 * Semantic data extraction engine.
 *
 * This will attempt to find travel itinerary data in the given input data
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
 * - PDF extractors are passed a PdfDocument instance allowing access to textual and
 *   image content.
 * - Apple Wallet pass extractors are passed a KPkPass::BoardingPass instance.
 * - iCalendar event extractors are passed KCalendarCore::Event instances.
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
 * (see https://invent.kde.org/pim/kitinerary-workbench).
 *
 * @subsection extractor_testing Automated Testing
 *
 * There are a few unit tests for extractors in the kitinerary repository (see autotests/extractordata),
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
 *   entirely empty (or non-existing) for structured data that does not need a custom extractor.
 *   This context information is applied to all tests in this folder.
 * - @c \<testname\>.[txt|html|pdf|pkpass|ics|eml|mbox]: The input test data.
 * - @c \<testname.extension\>.json: The expected JSON-LD output. If this file doesn't
 *   exists it is created by the test program.
 * - @c \<testname.extension\>.skip: If this file is present the corresponding test
 *   is skipped.
 */
class KITINERARY_EXPORT ExtractorEngine
{
public:
    ExtractorEngine();
    ExtractorEngine(ExtractorEngine &&) noexcept;
    ExtractorEngine(const ExtractorEngine &) = delete;
    ~ExtractorEngine();

    /** Resets the internal state, call before processing new input data. */
    void clear();

    /** Set raw data to extract from.
     *  @param data Raw data to extract from.
     *  @param fileName Used as a hint to determine the type, optional and used for MIME type auto-detection if needed.
     *  @param mimeType MIME type of @p data, auto-detected if empty.
     */
    void setData(const QByteArray &data, QStringView fileName = {}, QStringView mimeType = {});

    /** Already decoded data to extract from.
     *  @param data Has to contain a object of a supported data type matching @p mimeType.
     */
    void setContent(const QVariant &data, QStringView mimeType);

    /** Provide a document part that is only used to determine which extractor to use,
     *  but not for extraction itself.
     *  This can for example be the MIME message part wrapping a document to extract.
     *  Using this is not necessary when this document part is already included in
     *  what is passed to setContent() already anyway.
     */
    void setContext(const QVariant &data, QStringView mimeType);

    /** Set the date the extracted document has been issued at.
     *  This does not need to be perfectly accurate and is used to
     *  complete incomplete date information in the document (typically
     *  a missing year).
     *  This method does not need to be called when setContext is used.
     */
    void setContextDate(const QDateTime &dt);

    /** Perform extraction of "risky" content such as PDF files in a separate process.
     *  This is safer as it isolates the using application from crashes/hangs due to corrupt files.
     *  It is however slower, and not available on all platforms.
     *  This is off by default.
     */
    void setUseSeparateProcess(bool separateProcess);

    /** Sets additional extractors to run on the given data.
     *  Extractors are usually automatically selected, this is therefore most likely not needed to
     *  be called manually. This mainly exists for the external extractor process.
     */
    void setAdditionalExtractors(std::vector<const AbstractExtractor*> &&extractors);

    /** Hints about the document to extract based on application knowledge that
     *  can help the extractor.
     */
    enum Hint {
        NoHint = 0,
        ExtractFullPageRasterImages = 1, ///< perform expensive image processing on (PDF) documents containing full page raster images
        ExtractGenericIcalEvents = 2, ///< generate Event objects for generic ical events.
    };
    Q_DECLARE_FLAGS(Hints, Hint)

    /** The currently set extraction hints. */
    Hints hints() const;
    /** Set extraction hints. */
    void setHints(Hints hints);

    /** Perform the actual extraction, and return the JSON-LD data
     *  that has been found.
     */
    QJsonArray extract();

    /** Returns the extractor id used to obtain the result.
     *  Can be empty if generic extractors have been used.
     *  Not supposed to be used for normal operations, this is only needed for tooling.
     */
    QString usedCustomExtractor() const;

    /** Factory for creating new document nodes.
     *  This is only for use by KItinerary::ExtractorDocumentProcessor instances.
     */
    const ExtractorDocumentNodeFactory* documentNodeFactory() const;
    /** Barcode decoder for use by KItinerary::ExtractorDocumentProcessor.
     *  Use this rather than your own instance as it caches repeated attempts to
     *  decode the same image.
     */
    const BarcodeDecoder* barcodeDecoder() const;

    ///@cond internal
    /** Extractor repository instance used by this engine. */
    const ExtractorRepository* extractorRepository() const;
    /** JavaScript execution engine for script extractors. */
    const ExtractorScriptEngine* scriptEngine() const;
    /** Document root node.
     *  Only fully populated after extraction has been performed.
     *  Only exposed for tooling.
     */
    ExtractorDocumentNode rootDocumentNode() const;
    /** Process a single node.
     *  For use by the script engine, do not use manually.
     */
    void processNode(ExtractorDocumentNode &node) const;
    ///@endcond

private:
    std::unique_ptr<ExtractorEnginePrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ExtractorEngine::Hints)

}

