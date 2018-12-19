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

#include "genericpdfextractor.h"

#include <KItinerary/BarcodeDecoder>
#include <KItinerary/IataBcbpParser>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/PdfDocument>
#include <KItinerary/Rct2Ticket>
#include <KItinerary/Uic9183Parser>

#include <QDebug>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QTransform>

using namespace KItinerary;

enum {
    MaxPageCount = 10, // maximum in the current test set is 6
    MaxFileSize = 4000000, // maximum in the current test set is 980kB
    // unit is pixels, assuming landscape orientation
    MinSourceImageHeight = 10,
    MinSourceImageWidth = 30,
    MaxSourceImageHeight = 1000, // TODO what's a realisitic value here?
    MaxSourceImageWidth = 2000,
    // unit is 1/72 inch, assuming landscape orientation
    MinTargetImageHeight = 30,
    MinTargetImageWidth = 72,
    MaxTargetImageHeight = 252,
    MaxTargetImageWidth = 252,
};

GenericPdfExtractor::GenericPdfExtractor() = default;
GenericPdfExtractor::~GenericPdfExtractor() = default;

void GenericPdfExtractor::setContextDate(const QDateTime &dt)
{
    m_contextDate = dt;
}

void GenericPdfExtractor::extract(PdfDocument *doc, QJsonArray &result)
{
    m_unrecognizedBarcodes.clear();

    // stay away from documents that are atypically large for what we are looking for
    // that's just unecessarily eating up resources
    if (doc->pageCount() > MaxPageCount || doc->fileSize() > MaxFileSize) {
        return;
    }

    m_imageIds.clear();
    for (int i = 0; i < doc->pageCount(); ++i) {
        const auto page = doc->page(i);

        for (int j = 0; j < page.imageCount(); ++j) {
            const auto img = page.image(j);
            if (m_imageIds.find(img.objectId()) != m_imageIds.end()) {
                continue;
            }

            // image source size sanity checks
            if (std::min(img.sourceWidth(), img.sourceHeight()) < MinSourceImageHeight
             || std::max(img.sourceWidth(), img.sourceHeight()) < MinSourceImageWidth
             || img.sourceHeight() > MaxSourceImageHeight
             || img.sourceWidth() > MaxSourceImageWidth) {
                continue;
            }

            // image target size checks
            const auto targetRect = img.transform().map(QRectF(0, 0, 1, -1)).boundingRect();
            if (std::min(targetRect.width(), targetRect.height()) < MinTargetImageHeight
             || std::max(targetRect.width(), targetRect.height()) < MinTargetImageWidth
             || targetRect.height() > MaxTargetImageHeight
             || targetRect.width() > MaxTargetImageWidth) {
                continue;
            }

            extractImage(img, result);
            m_imageIds.insert(img.objectId());
        }
    }
}

QStringList GenericPdfExtractor::unrecognizedBarcodes() const
{
    return m_unrecognizedBarcodes;
}

void GenericPdfExtractor::extractImage(const PdfImage &img, QJsonArray &result)
{
    const auto aspectRatio = img.width() < img.height() ?
        (float)img.height() / (float)img.width() :
        (float)img.width() / (float)img.height();

    // almost square, assume Aztec (or QR, which we don't handle here yet)
    if (aspectRatio < 1.2f) {
        const auto b = BarcodeDecoder::decodeAztecBinary(img.image());
        if (Uic9183Parser::maybeUic9183(b)) {
            extractUic9183(b, result);
        } else {
            extractBarcode(QString::fromUtf8(b), result);
        }
    }

    // rectangular with medium aspect ratio, assume PDF 417
    if (aspectRatio > 1.5 && aspectRatio < 6) {
        const auto s = BarcodeDecoder::decodePdf417(img.image());
        extractBarcode(s, result);
    }
}

void GenericPdfExtractor::extractBarcode(const QString &code, QJsonArray &result)
{
    if (IataBcbpParser::maybeIataBcbp(code)) {
        const auto res = IataBcbpParser::parse(code, m_contextDate.date());
        const auto jsonLd = JsonLdDocument::toJson(res);
        std::copy(jsonLd.begin(), jsonLd.end(), std::back_inserter(result));
    }

    m_unrecognizedBarcodes.push_back(code);
}

void GenericPdfExtractor::extractUic9183(const QByteArray &data, QJsonArray &result)
{
    Uic9183Parser p;
    p.setContextDate(m_contextDate);
    p.parse(data);
    if (!p.isValid()) {
        return;
    }

    QJsonObject org;
    org.insert(QLatin1String("@type"), QLatin1String("Organization"));
    org.insert(QLatin1String("identifier"), QString(QLatin1String("uic:") + p.carrierId()));
    QJsonObject trip;
    trip.insert(QLatin1String("@type"), QLatin1String("TrainTrip"));
    trip.insert(QLatin1String("provider"), org);
    QJsonObject seat;
    seat.insert(QLatin1String("@type"), QLatin1String("Seat"));

    const auto rct2 = p.rct2Ticket();
    if (rct2.isValid()) {
        switch (rct2.type()) {
            case Rct2Ticket::Reservation:
            {
                QJsonObject dep;
                dep.insert(QLatin1String("@type"), QLatin1String("TrainStation"));
                dep.insert(QLatin1String("name"), rct2.outboundDepartureStation());
                trip.insert(QLatin1String("departureStation"), dep);
                trip.insert(QLatin1String("departureTime"), rct2.outboundDepartureTime().toString(Qt::ISODate));

                QJsonObject arr;
                arr.insert(QLatin1String("@type"), QLatin1String("TrainStation"));
                arr.insert(QLatin1String("name"), rct2.outboundArrivalStation());
                trip.insert(QLatin1String("arrivalStation"), arr);
                trip.insert(QLatin1String("arrivalTime"), rct2.outboundArrivalTime().toString(Qt::ISODate));

                trip.insert(QLatin1String("trainNumber"), rct2.trainNumber());
                seat.insert(QLatin1String("seatSection"), rct2.coachNumber());
                seat.insert(QLatin1String("seatNumber"), rct2.seatNumber());
                break;
            }
            default:
                break;
        }
    }

    QJsonObject ticket;
    ticket.insert(QLatin1String("@type"), QLatin1String("Ticket"));
    ticket.insert(QLatin1String("ticketedSeat"), seat);

    QJsonObject res;
    res.insert(QLatin1String("@type"), QLatin1String("TrainReservation"));
    res.insert(QLatin1String("reservationFor"), trip);
    res.insert(QLatin1String("reservationNumber"), p.pnr());
    res.insert(QLatin1String("reservedTicket"), ticket);

    result.push_back(res);
}
