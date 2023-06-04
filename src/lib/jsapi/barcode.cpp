/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "barcode.h"
#include <pdf/pdfbarcodeutil_p.h>

#include <KItinerary/PdfDocument>

#include <QImage>
#include <QQmlEngine>

using namespace KItinerary;

void JsApi::Barcode::setDecoder(const BarcodeDecoder *decoder)
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

QJSValue JsApi::Barcode::decodeAztecBinary(const QVariant &img) const
{
    if (img.userType() == qMetaTypeId<PdfImage>()) {
        const auto pdfImg = img.value<PdfImage>();
        if (PdfBarcodeUtil::maybeBarcode(pdfImg, BarcodeDecoder::Aztec) == BarcodeDecoder::None) {
            return {};
        }
        const auto content = m_decoder->decodeBinary(pdfImg.image(), BarcodeDecoder::Aztec);
        if (content.isEmpty()) {
            return {};
        }
        const auto engine = qjsEngine(this);
        return QJSValue(QJSManagedValue(QVariant(content), engine));
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
        if (PdfBarcodeUtil::maybeBarcode(pdfImg, hints) == BarcodeDecoder::None) {
            return {};
        }
        return m_decoder->decodeString(pdfImg.image(), hints);
    }
    return {};
}

#include "moc_barcode.cpp"
