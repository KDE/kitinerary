/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KItinerary/BarcodeDecoder>

#include <QDateTime>
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
     *  @return a QByteArray, which from the JS perspective is essentially an opque handle.
     */
    Q_INVOKABLE QVariant decodeAztecBinary(const QVariant &img) const;
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
    Q_INVOKABLE QVariant decodeEraSsbTicket(const QVariant &s, int versionOverride = 0) const;

    /** Converts the given QByteArray into a BitArray. */
    Q_INVOKABLE QVariant toBitArray(const QVariant &b) const;

    ///@cond internal
    void setDecoder(BarcodeDecoder *decoder);
    ///@endcond

private:
    QString decodeBarcode(const QVariant &img, BarcodeDecoder::BarcodeTypes hints) const;

    BarcodeDecoder *m_decoder = nullptr;
};

}
}

