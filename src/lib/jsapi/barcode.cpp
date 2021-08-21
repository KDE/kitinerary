/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "barcode.h"
#include "bitarray.h"
#include <era/ssbticketreader.h>
#include <pdf/pdfbarcodeutil_p.h>

#include <KItinerary/PdfDocument>
#include <KItinerary/Uic9183Parser>
#include <KItinerary/VdvTicket>
#include <KItinerary/VdvTicketParser>

#include <QImage>

using namespace KItinerary;

void JsApi::Barcode::setDecoder(BarcodeDecoder *decoder)
{
    m_decoder = decoder;
}

QString JsApi::Barcode::decodePdf417(const QVariant &img) const
{
    return decodeBarcode(img, BarcodeDecoder::PDF417);
}

QString JsApi::Barcode::decodeAztec(const QVariant &img) const
{
    return decodeBarcode(img, BarcodeDecoder::Aztec);
}

QVariant JsApi::Barcode::decodeAztecBinary(const QVariant &img) const
{
    if (img.userType() == qMetaTypeId<PdfImage>()) {
        const auto pdfImg = img.value<PdfImage>();
        if (!PdfBarcodeUtil::maybeBarcode(pdfImg, BarcodeDecoder::Aztec)) {
            return {};
        }
        const auto content = m_decoder->decodeBinary(pdfImg.image(), BarcodeDecoder::Aztec);
        if (content.isEmpty()) {
            return {};
        }
        return content;
    }
    return {};
}

QString JsApi::Barcode::decodeQR(const QVariant &img) const
{
    return decodeBarcode(img, BarcodeDecoder::QRCode);
}

QString JsApi::Barcode::decodeDataMatrix(const QVariant &img) const
{
    return decodeBarcode(img, BarcodeDecoder::DataMatrix);
}

QString JsApi::Barcode::decodeAnyBarcode(const QVariant& img) const
{
    if (img.userType() == qMetaTypeId<PdfImage>()) {
        const auto pdfImg = img.value<PdfImage>();
        return m_decoder->decodeString(pdfImg.image(), BarcodeDecoder::Any | BarcodeDecoder::IgnoreAspectRatio);
    }
    return {};
}

QString JsApi::Barcode::decodeBarcode(const QVariant &img, BarcodeDecoder::BarcodeTypes hints) const
{
    if (img.userType() == qMetaTypeId<PdfImage>()) {
        const auto pdfImg = img.value<PdfImage>();
        if (!PdfBarcodeUtil::maybeBarcode(pdfImg, hints)) {
            return {};
        }
        return m_decoder->decodeString(pdfImg.image(), hints);
    }
    return {};
}

QVariant JsApi::Barcode::decodeUic9183(const QVariant &s) const
{
    Uic9183Parser p;
    p.parse(s.toByteArray());
    if (!p.isValid()) {
        return {};
    }
    return QVariant::fromValue(p);
}

QVariant JsApi::Barcode::decodeVdvTicket(const QVariant &s) const
{
    VdvTicketParser p;
    if (!p.parse(s.toByteArray())) {
        return {};
    }
    return QVariant::fromValue(p.ticket());
}

QVariant JsApi::Barcode::decodeEraSsbTicket(const QVariant &s, int versionOverride) const
{
    return SSBTicketReader::read(s.toByteArray(), versionOverride);
}

QString JsApi::Barcode::toBase64(const QVariant &b) const
{
    return QString::fromUtf8(b.toByteArray().toBase64());
}

QVariant JsApi::Barcode::fromBase64(const QString &s) const
{
    return QByteArray::fromBase64(s.toUtf8());
}

QVariant JsApi::Barcode::toBitArray(const QVariant &b) const
{
    return QVariant::fromValue(JsApi::BitArray(b.toByteArray()));
}

void JsApi::Barcode::setContextDate(const QDateTime &dt)
{
    m_contextDate = dt;
}

QString JsApi::Barcode::byteArrayToString(const QVariant &b) const
{
    const auto ba = b.toByteArray();
    return QString::fromLatin1(ba.constData(), ba.size());
}

#include "moc_barcode.cpp"
