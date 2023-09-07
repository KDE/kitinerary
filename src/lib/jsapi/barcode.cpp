/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "barcode.h"
#include <pdf/pdfbarcodeutil_p.h>

#include <KItinerary/PdfDocument>

#include <QImage>
#include <QQmlEngine>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <private/qv4arraybuffer_p.h>
#include <private/qv4engine_p.h>
#endif

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
        const auto content = m_decoder->decode(pdfImg.image(), BarcodeDecoder::Aztec).toByteArray();
        if (content.isEmpty()) {
            return {};
        }
        const auto engine = qjsEngine(this);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        return QJSValue(engine->handle(), engine->handle()->newArrayBuffer(content)->asReturnedValue());
#else
        return QJSValue(QJSManagedValue(QVariant(content), engine));
#endif
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
        return m_decoder->decode(pdfImg.image(), BarcodeDecoder::Any | BarcodeDecoder::IgnoreAspectRatio).toString();
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
        return m_decoder->decode(pdfImg.image(), hints).toString();
    }
    return {};
}

#include "moc_barcode.cpp"
