/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-kitinerary.h"
#include "pdfdocument.h"
#include "pdfdocument_p.h"
#include "pdfextractoroutputdevice_p.h"
#include "pdfimage_p.h"
#include "popplerglobalparams_p.h"
#include "popplerutils_p.h"
#include "logging.h"

#include <QDebug>
#include <QImage>
#include <QScopedValueRollback>
#include <QTimeZone>

#include <DateInfo.h>
#include <PDFDoc.h>
#include <PDFDocEncoding.h>
#include <Stream.h>
#include <UTF.h>

#include <cmath>

using namespace KItinerary;

void PdfPagePrivate::load()
{
    if (m_loaded) {
        return;
    }

    PopplerGlobalParams gp;
    PdfExtractorOutputDevice device;
    m_doc->m_popplerDoc->displayPageSlice(&device, m_pageNum + 1, 72, 72, 0, false, true, false, -1, -1, -1, -1);
    m_doc->m_popplerDoc->processLinks(&device, m_pageNum + 1);
    device.finalize();
    const auto pageRect = m_doc->m_popplerDoc->getPage(m_pageNum + 1)->getCropBox();
#if KPOPPLER_VERSION < QT_VERSION_CHECK(25, 1, 0)
    std::unique_ptr<GooString> s(device.getText(pageRect->x1, pageRect->y1, pageRect->x2, pageRect->y2));
    m_text = QString::fromUtf8(s->c_str());
#else
    const auto s = device.getText(pageRect->x1, pageRect->y1, pageRect->x2, pageRect->y2);
    m_text = QString::fromUtf8(s.c_str());
#endif

    m_images = std::move(device.m_images);
    for (auto it = m_images.begin(); it != m_images.end(); ++it) {
        (*it).d->m_page = this;
    }

    m_links = std::move(device.m_links);
    for (auto &link : m_links) {
        link.convertToPageRect(pageRect);
    }

    m_loaded = true;
}

PdfPage::PdfPage()
    : d(new PdfPagePrivate)
{
}

PdfPage::PdfPage(const PdfPage&) = default;
PdfPage::~PdfPage() = default;
PdfPage& PdfPage::operator=(const PdfPage&) = default;

QString PdfPage::text() const
{
    d->load();
    return d->m_text;
}

static double ratio(double begin, double end, double ratio)
{
    return begin + (end - begin) * ratio;
}

QString PdfPage::textInRect(double left, double top, double right, double bottom, int rotate) const
{
    PopplerGlobalParams gp;

    const auto page = d->m_doc->m_popplerDoc->getPage(d->m_pageNum + 1);
    const auto pageRect = page->getCropBox();

    double l;
    double t;
    double r;
    double b;
    switch (page->getRotate()) {
        case 0:
            l = ratio(pageRect->x1, pageRect->x2, left);
            t = ratio(pageRect->y1, pageRect->y2, top);
            r = ratio(pageRect->x1, pageRect->x2, right);
            b = ratio(pageRect->y1, pageRect->y2, bottom);
            break;
        case 90:
            l = ratio(pageRect->y1, pageRect->y2, left);
            t = ratio(pageRect->x1, pageRect->x2, top);
            r = ratio(pageRect->y1, pageRect->y2, right);
            b = ratio(pageRect->x1, pageRect->x2, bottom);
            break;
        default:
            qCWarning(Log) << "Unsupported page rotation!" << page->getRotate();
            return {};
    }

    TextOutputDev device(nullptr, false, 0, false, false);
    d->m_doc->m_popplerDoc->displayPageSlice(&device, d->m_pageNum + 1, 72, 72, rotate, false, true, false, -1, -1, -1, -1);
#if KPOPPLER_VERSION <QT_VERSION_CHECK(25, 1, 0)
    std::unique_ptr<GooString> s(device.getText(l, t, r, b));
    return QString::fromUtf8(s->c_str());
#else
    const auto s = device.getText(l, t, r, b);
    return QString::fromUtf8(s.c_str());
#endif
}

int PdfPage::imageCount() const
{
    d->load();
    return d->m_images.size();
}

PdfImage PdfPage::image(int index) const
{
    d->load();
    return d->m_images[index];
}

QVariantList PdfPage::imagesVariant() const
{
    d->load();
    QVariantList l;
    l.reserve(imageCount());
    std::for_each(d->m_images.begin(), d->m_images.end(), [&l](const PdfImage& img) { l.push_back(QVariant::fromValue(img)); });
    return l;
}

QVariantList PdfPage::imagesInRect(double left, double top, double right, double bottom) const
{
    d->load();
    QVariantList l;
    PopplerGlobalParams gp;
    const auto pageRect = d->m_doc->m_popplerDoc->getPage(d->m_pageNum + 1)->getCropBox();

    for (const auto &img : d->m_images) {
        if ((img.d->m_transform.dx() >= ratio(pageRect->x1, pageRect->x2, left) && img.d->m_transform.dx() <= ratio(pageRect->x1, pageRect->x2, right)) &&
            (img.d->m_transform.dy() >= ratio(pageRect->y1, pageRect->y2, top)  && img.d->m_transform.dy() <= ratio(pageRect->y1, pageRect->y2, bottom)))
        {
            l.push_back(QVariant::fromValue(img));
        }
    }
    return l;
}

int PdfPage::linkCount() const
{
    d->load();
    return d->m_links.size();
}

PdfLink PdfPage::link(int index) const
{
    d->load();
    return d->m_links[index];
}

QVariantList PdfPage::linksVariant() const
{
    d->load();
    QVariantList l;
    l.reserve(d->m_links.size());
    std::transform(d->m_links.begin(), d->m_links.end(), std::back_inserter(l), [](const PdfLink &link) { return QVariant::fromValue(link); });
    return l;
}

QVariantList PdfPage::linksInRect(double left, double top, double right, double bottom) const
{
    QRectF bbox(QPointF(left, top), QPointF(right, bottom));
    d->load();

    QVariantList l;
    for (const auto &link : d->m_links) {
        if (!link.area().intersects(bbox)) {
            continue;
        }
        l.push_back(QVariant::fromValue(link));
    }

    std::sort(l.begin(), l.end(), [](const auto &lhs, const auto &rhs) {
        const auto lhsLink = lhs.template value<PdfLink>();
        const auto rhsLink = rhs.template value<PdfLink>();
        if (lhsLink.area().top() == rhsLink.area().top()) {
            return lhsLink.area().left() < rhsLink.area().left();
        }
        return lhsLink.area().top() < rhsLink.area().top();
    });

    return l;
}

static constexpr inline double pdfToMM(double points)
{
    return points * 25.4 / 72.0;
}

int PdfPage::width() const
{
    const auto page = d->m_doc->m_popplerDoc->getPage(d->m_pageNum + 1);
    const auto rot = page->getRotate();
    if (rot == 90 || rot == 270) {
        return pdfToMM(page->getCropHeight());
    }
    return pdfToMM(page->getCropWidth());
}

int PdfPage::height() const
{
    const auto page = d->m_doc->m_popplerDoc->getPage(d->m_pageNum + 1);
    const auto rot = page->getRotate();
    if (rot == 90 || rot == 270) {
        return pdfToMM(page->getCropWidth());
    }
    return pdfToMM(page->getCropHeight());
}


PdfDocument::PdfDocument(QObject *parent)
    : QObject(parent)
    , d(new PdfDocumentPrivate)
{
}

PdfDocument::~PdfDocument() = default;

QString PdfDocument::text() const
{
    QString text;
    std::for_each(d->m_pages.begin(), d->m_pages.end(), [&text](const PdfPage &p) { text += p.text(); });
    return text;
}

int PdfDocument::pageCount() const
{
    return d->m_popplerDoc->getNumPages();
}

PdfPage PdfDocument::page(int index) const
{
    return d->m_pages[index];
}

int PdfDocument::fileSize() const
{
    return d->m_pdfData.size();
}

static QDateTime parsePdfDateTime(const GooString *str)
{
    int year;
    int month;
    int day;
    int hour;
    int min;
    int sec;
    int tzHours;
    int tzMins;
    char tz;

    if (!parseDateString(str, &year, &month, &day, &hour, &min, &sec, &tz, &tzHours, &tzMins)) {
        return {};
    }

    QDate date(year, month, day);
    QTime time(hour, min, sec);
    if (!date.isValid() || !time.isValid()) {
        return {};
    }

    int offset = tzHours * 3600 + tzMins * 60;
    if (tz == '+') {
        return QDateTime(date, time, QTimeZone::fromSecondsAheadOfUtc(offset));
    } else if (tz == '-') {
        return QDateTime(date, time, QTimeZone::fromSecondsAheadOfUtc(-offset));
    }
    return QDateTime(date, time, QTimeZone::UTC);
}

QDateTime PdfDocument::creationTime() const
{
    std::unique_ptr<GooString> dt(d->m_popplerDoc->getDocInfoCreatDate());
    if (!dt) {
        return {};
    }
    return parsePdfDateTime(dt.get());
}

QDateTime PdfDocument::modificationTime() const
{
    std::unique_ptr<GooString> dt(d->m_popplerDoc->getDocInfoModDate());
    if (!dt) {
        return {};
    }
    return parsePdfDateTime(dt.get());
}


QString gooStringToUnicode(const std::unique_ptr<GooString> &s)
{
    if (!s) {
        return {};
    }

#if KPOPPLER_VERSION >= QT_VERSION_CHECK(24, 5, 0)
    if (hasUnicodeByteOrderMark(s->toStr()) || hasUnicodeByteOrderMarkLE(s->toStr())) {
#else
    if (s->hasUnicodeMarker() || s->hasUnicodeMarkerLE()) {
#endif
        return QString::fromUtf16(reinterpret_cast<const char16_t*>(s->toStr().c_str()), s->toStr().size() / 2);
    } else {
#if KPOPPLER_VERSION >= QT_VERSION_CHECK(25, 2, 0)
        const auto utf16Data = pdfDocEncodingToUTF16(s->toStr());
        return QString::fromUtf16(reinterpret_cast<const char16_t *>(utf16Data.c_str()), utf16Data.size() / 2);
#else
        int len = 0;
        std::unique_ptr<const char[]> utf16Data(pdfDocEncodingToUTF16(s->toStr(), &len));
        return QString::fromUtf16(reinterpret_cast<const char16_t*>(utf16Data.get()), len / 2);
#endif
    }

    return QString::fromUtf8(s->c_str());
}

QString PdfDocument::title() const
{
    return gooStringToUnicode(d->m_popplerDoc->getDocInfoTitle());
}

QString PdfDocument::producer() const
{
    return gooStringToUnicode(d->m_popplerDoc->getDocInfoProducer());
}

QString PdfDocument::creator() const
{
    return gooStringToUnicode(d->m_popplerDoc->getDocInfoCreator());
}

QString PdfDocument::author() const
{
    return gooStringToUnicode(d->m_popplerDoc->getDocInfoAuthor());
}

QVariantList PdfDocument::pagesVariant() const
{
    QVariantList l;
    l.reserve(pageCount());
    std::for_each(d->m_pages.begin(), d->m_pages.end(), [&l](const PdfPage& p) { l.push_back(QVariant::fromValue(p)); });
    return l;
}

PdfDocument* PdfDocument::fromData(const QByteArray &data, QObject *parent)
{
    PopplerGlobalParams gp;

    std::unique_ptr<PdfDocument> doc(new PdfDocument(parent));
    doc->d->m_pdfData = data;
    // PDFDoc takes ownership of stream
    auto stream = new MemStream(const_cast<char*>(doc->d->m_pdfData.constData()), 0, doc->d->m_pdfData.size(), Object());
    std::unique_ptr<PDFDoc> popplerDoc(new PDFDoc(stream));
    if (!popplerDoc->isOk()) {
        qCWarning(Log) << "Got invalid PDF document!" << popplerDoc->getErrorCode();
        return nullptr;
    }

    doc->d->m_pages.reserve(popplerDoc->getNumPages());
    for (int i = 0; i < popplerDoc->getNumPages(); ++i) {
        PdfPage page;
        page.d->m_pageNum = i;
        page.d->m_doc = doc->d.get();
        doc->d->m_pages.push_back(page);
    }

    doc->d->m_popplerDoc = std::move(popplerDoc);
    return doc.release();
}

bool PdfDocument::maybePdf(const QByteArray &data)
{
    return data.startsWith("%PDF");
}

#include "moc_pdfdocument.cpp"
