/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KItinerary/BarcodeDecoder>

#include <QDateTime>
#include <QJSValue>
#include <QObject>

namespace KItinerary {

class BarcodeDecoder;

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
     *  @return a JS ArrayBuffer
     */
    Q_INVOKABLE QJSValue decodeAztecBinary(const QVariant &img) const;
    /** Decode as QR barcode image.
     *  @param img An image containing the barcode, e.g. a PdfImage instance.
     */
    Q_INVOKABLE QString decodeQR(const QVariant &img) const;
    /** Decode a DataMatrix barcode image.
     *  @param img An image containing the barcode, e.g. a PdfImage instance.
     */
    Q_INVOKABLE QString decodeDataMatrix(const QVariant &img) const;
    /** Attempts to decode any barcode found in the given image.
     *  This is the most expensive of the above option, and should only be
     *  used if no other alternative is available.
     *  @param img An image containing the barcode, e.g. a PdfImage instance.
     */
    Q_INVOKABLE QString decodeAnyBarcode(const QVariant &img) const;

    /** Decode an ERA SSB ticket barcode.
     *  @param s A QByteArray containing the raw ERA SSB barcode data.
     *  @param versionOverride Override version auto-detection. Useful for tickets that are known to
     *  fill their version field incorrectly.
     *  @returns An instance of SSBTicket.
     */
    Q_INVOKABLE QVariant decodeEraSsbTicket(const QByteArray &s, int versionOverride = 0) const;

    ///@cond internal
    void setDecoder(const BarcodeDecoder *decoder);
    ///@endcond

private:
    QString decodeBarcode(const QVariant &img, BarcodeDecoder::BarcodeTypes hints) const;

    const BarcodeDecoder *m_decoder = nullptr;
};

}
}

