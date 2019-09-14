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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "barcode.h"
#include "bitarray.h"
#include <genericpdfextractor_p.h>

#include <KItinerary/BarcodeDecoder>
#include <KItinerary/IataBcbpParser>
#include <KItinerary/PdfDocument>
#include <KItinerary/Uic9183Parser>

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

QVariant JsApi::Barcode::decodeUic9183(const QVariant &s) const
{
    Uic9183Parser p;
    p.setContextDate(m_contextDate);
    p.parse(s.toByteArray());
    return QVariant::fromValue(p);
}

QVariant JsApi::Barcode::decodeIataBcbp(const QString &s) const
{
    return QVariant::fromValue(IataBcbpParser::parse(s, m_contextDate.date()));
}

QString JsApi::Barcode::toBase64(const QVariant &b) const
{
    return QString::fromUtf8(b.toByteArray().toBase64());
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
