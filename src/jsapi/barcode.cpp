/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "barcode.h"
#include "bitarray.h"
#include <generic/genericpdfextractor_p.h>

#include <KItinerary/BarcodeDecoder>
#include <KItinerary/IataBcbpParser>
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
    if (img.userType() == qMetaTypeId<PdfImage>()) {
        const auto pdfImg = img.value<PdfImage>();
        if (!GenericPdfExtractor::maybeBarcode(pdfImg, BarcodeDecoder::PDF417)) {
            return {};
        }
        return m_decoder->decodeString(pdfImg.image(), BarcodeDecoder::PDF417);
    }
    return {};
}

QString JsApi::Barcode::decodeAztec(const QVariant &img) const
{
    if (img.userType() == qMetaTypeId<PdfImage>()) {
        const auto pdfImg = img.value<PdfImage>();
        if (!GenericPdfExtractor::maybeBarcode(pdfImg, BarcodeDecoder::Aztec)) {
            return {};
        }
        return m_decoder->decodeString(pdfImg.image(), BarcodeDecoder::Aztec);
    }
    return {};
}

QVariant JsApi::Barcode::decodeAztecBinary(const QVariant &img) const
{
    if (img.userType() == qMetaTypeId<PdfImage>()) {
        const auto pdfImg = img.value<PdfImage>();
        if (!GenericPdfExtractor::maybeBarcode(pdfImg, BarcodeDecoder::Aztec)) {
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
    if (img.userType() == qMetaTypeId<PdfImage>()) {
        const auto pdfImg = img.value<PdfImage>();
        if (!GenericPdfExtractor::maybeBarcode(pdfImg, BarcodeDecoder::QRCode)) {
            return {};
        }
        return m_decoder->decodeString(pdfImg.image(), BarcodeDecoder::QRCode);
    }
    return {};
}

QString JsApi::Barcode::decodeDataMatrix(const QVariant &img) const
{
    if (img.userType() == qMetaTypeId<PdfImage>()) {
        const auto pdfImg = img.value<PdfImage>();
        if (!GenericPdfExtractor::maybeBarcode(pdfImg, BarcodeDecoder::DataMatrix)) {
            return {};
        }
        return m_decoder->decodeString(pdfImg.image(), BarcodeDecoder::DataMatrix);
    }
    return {};
}

QVariant JsApi::Barcode::decodeUic9183(const QVariant &s) const
{
    Uic9183Parser p;
    p.setContextDate(m_contextDate);
    p.parse(s.toByteArray());
    if (!p.isValid()) {
        return {};
    }
    return QVariant::fromValue(p);
}

QVariant JsApi::Barcode::decodeIataBcbp(const QString &s) const
{
    return QVariant::fromValue(IataBcbpParser::parse(s, m_contextDate.date()));
}

QVariant JsApi::Barcode::decodeVdvTicket(const QVariant &s) const
{
    VdvTicketParser p;
    if (!p.parse(s.toByteArray())) {
        return {};
    }
    return QVariant::fromValue(p.ticket());
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

#include "moc_barcode.cpp"
