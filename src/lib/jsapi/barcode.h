/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

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

    /** Decode an UIC 918.3 message from a train ticket Aztec code.
     *  @param s A QByteArray containing the raw data from the barcode.
     *  @returns An instance of Uic9183Parser.
     */
    Q_INVOKABLE QVariant decodeUic9183(const QVariant &s) const;
    /** Decode an IATA BCBP message from a flight boarding pass barcode.
     *  @returns A JSON-LD structure representing the boarding pass.
     */
    Q_INVOKABLE QVariant decodeIataBcbp(const QString &s) const;
    /** Decode an VDV ticket barcode.
     *  @param s A QByteArray containing the raw VDV barcode data.
     *  @returns An instance of VdvTicket.
     */
    Q_INVOKABLE QVariant decodeVdvTicket(const QVariant &s) const;
    /** Decode an ERA SSB ticket barcode.
     *  @param s A QByteArray containing the raw ERA SSB barcode data.
     *  @returns An instance of SSBTicket.
     */
    Q_INVOKABLE QVariant decodeEraSsbTicket(const QVariant &s) const;

    /** Converts the given QByteArray into an base64 encoded string. */
    Q_INVOKABLE QString toBase64(const QVariant &b) const;
    /** Converts a given Base64 encoded string to a QByteArray. */
    Q_INVOKABLE QVariant fromBase64(const QString &s) const;
    /** Converts the given QByteArray into a BitArray. */
    Q_INVOKABLE QVariant toBitArray(const QVariant &b) const;
    /** Converts the given QByteArray into a string for processing in JS.
     *  This only works if there isn't "too much" binary content in the byte array.
     */
    Q_INVOKABLE QString byteArrayToString(const QVariant &b) const;

    ///@cond internal
    void setContextDate(const QDateTime &dt);
    void setDecoder(BarcodeDecoder *decoder);
    ///@endcond

private:
    QDateTime m_contextDate;
    BarcodeDecoder *m_decoder = nullptr;
};

}
}

