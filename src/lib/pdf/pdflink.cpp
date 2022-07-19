/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pdflink.h"

#include <QDebug>

#include <Page.h>

using namespace KItinerary;

namespace KItinerary {
class PdfLinkPrivate : public QSharedData
{
public:
    QString url;
    QRectF area;
};
}

PdfLink::PdfLink()
    : d(new PdfLinkPrivate)
{
}

PdfLink::PdfLink(const QString& url, const QRectF& area)
    : d(new PdfLinkPrivate)
{
    d->url = url;
    d->area = area;
}

PdfLink::PdfLink(const PdfLink&) = default;
PdfLink::~PdfLink() = default;

PdfLink& PdfLink::operator=(const PdfLink&) = default;

QString PdfLink::url() const
{
    return d->url;
}

QRectF PdfLink::area() const
{
    return d->area;
}

static double toRatio(double low, double high, double value)
{
    return (value - low) / (high - low);
}

void PdfLink::convertToPageRect(const PDFRectangle *pageRect)
{
    d->area.setLeft(toRatio(pageRect->x1, pageRect->x2, d->area.left()));
    d->area.setRight(toRatio(pageRect->x1, pageRect->x2, d->area.right()));
    d->area.setTop(toRatio(pageRect->y1, pageRect->y2, d->area.top()));
    d->area.setBottom(toRatio(pageRect->y1, pageRect->y2, d->area.bottom()));
}
