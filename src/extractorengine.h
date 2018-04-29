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

/** Code for executing an extractor rule set on a specific email part. */
class KITINERARY_EXPORT ExtractorEngine
{
public:
    ExtractorEngine();
    ~ExtractorEngine();
    ExtractorEngine(ExtractorEngine &&);
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
