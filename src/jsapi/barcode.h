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

#ifndef KITINERARY_JSAPI_BARCODE_H
#define KITINERARY_JSAPI_BARCODE_H

#include <QDate>
#include <QObject>

namespace KItinerary {
namespace JsApi {

/** Barcode decoding functions. */
class Barcode : public QObject
{
    Q_OBJECT
public:
    /** Decode a PDF417 barcode image.
     *  @param img An image containing the barcode, e.g. a PdfImage instance.
     */
    Q_INVOKABLE QString decodePdf417(const QVariant &img) const;
    /** Decode a Aztec barcode image.
     *  @param img An image containing the barcode, e.g. a PdfImage instance.
     */
    Q_INVOKABLE QString decodeAztec(const QVariant &img) const;
    /** Decode a Aztec barcode image containing binary data.
     *  @param img An image containing the barcode, e.g. a PdfImage instance.
     *  @return a QByteArray, which from the JS perspective is essentially an opque handle.
     */
    Q_INVOKABLE QVariant decodeAztecBinary(const QVariant &img) const;
    /** Decode an UIC 918.3 message from a train ticket Aztec code.
     *  @param s A QByteArray containing the raw data from the barcode.
     *  @returns An instance of Uic9183Parser.
     */
    Q_INVOKABLE QVariant decodeUic9183(const QVariant &s) const;
    /** Decode an IATA BCBP message from a flight boarding pass barcode.
     *  @returns A JSON-LD structure representing the boarding pass.
     */
    Q_INVOKABLE QVariant decodeIataBcbp(const QString &s) const;
    /** Converts the given QByteArray into an base64 encoded string. */
    Q_INVOKABLE QString toBase64(const QVariant &b) const;

    ///@cond internal
    void setContextDate(const QDate &date);
    ///@endcond

private:
    QDate m_contextDate;
};

}
}

#endif // KITINERARY_JSAPI_BARCODE_H
