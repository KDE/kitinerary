/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QDateTime>
#include <QJSValue>
#include <QObject>
#include <QVariant>

namespace KItinerary {

/** JavaScript API exposed to extractor scripts.
 *  @see ExtractorEngine
 */
namespace JsApi {

/** The extraction context.
 *  This object contains information about what is being extracted,
 *  or where the extracted information is coming from.
 */
class Context : public QObject
{
    Q_OBJECT
    /** The time the email containing the extracted data was sent.
     *  This can be useful if the extracted data only contains dates without
     *  specifying a year. The year can then be inferred out of this context.
     */
    Q_PROPERTY(QDateTime senderDate MEMBER m_senderDate)

    /** The PDF page number (0-based) if the extractor was triggered by
     *  content found in a PDF file (typically a barcode).
     */
    Q_PROPERTY(int pdfPageNumber MEMBER m_pdfPageNum)

    /** If the extractor was triggered by a barcode, this contains the
     *  corresponding barcode content (either as string or as byte array).
     */
    Q_PROPERTY(QVariant barcode MEMBER m_barcode)

    /** If the extractor was triggered by results from a preceding generic
     *  extractor run, this property contains the generic extraction results.
     *  A typical example are e.g. information retrieved from an IATA barcode.
     *  This can be used by the extractor script to augment generic extraction results.
     */
    Q_PROPERTY(QJSValue data READ data)

public:
    ///@cond internal
    /** Reset context information that are only relevant for a single run. */
    QJSValue data() const;
    void reset();

    QDateTime m_senderDate;
    QVariant m_barcode;
    QJSValue m_data;
    int m_pdfPageNum = -1;
    ///@endcond
};

}
}

