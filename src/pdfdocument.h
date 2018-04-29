/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KITINERARY_PDFDOCUMENT_H
#define KITINERARY_PDFDOCUMENT_H

#include "kitinerary_export.h"

#include <QObject>

#include <memory>

class QImage;

namespace KItinerary {

class PdfDocumentPrivate;

/** PDF document for extraction.
 *  This is used as input for ExtractorEngine and the JS extractor scripts.
 *  @note This class is only functional if Poppler is available as a dependency,
 *  otherwise all methods return empty values.
 */
class KITINERARY_EXPORT PdfDocument : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text CONSTANT)
    Q_PROPERTY(int imageCount READ imageCount CONSTANT)
public:
    explicit PdfDocument(QObject *parent = nullptr);
    ~PdfDocument();

    /** The entire text extracted from the PDF document. */
    QString text() const;

    /** The number of images found in this document. */
    int imageCount() const;

    /** The n-th image found in this document. */
    QImage image(int index) const;

    /** Creates a PdfDocument from the given raw data.
     *  @returns @c nullptr if loading fails or Poppler was not found.
     */
    static PdfDocument* fromData(const QByteArray &data, QObject *parent = nullptr);

private:
    std::unique_ptr<PdfDocumentPrivate> d;
};

}

#endif // KITINERARY_PDFDOCUMENT_H
