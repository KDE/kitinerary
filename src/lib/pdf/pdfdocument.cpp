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

#ifdef HAVE_POPPLER
#include <DateInfo.h>
#include <PDFDoc.h>
#include <Stream.h>
#endif

#include <cmath>

using namespace KItinerary;

void PdfPagePrivate::load()
{
    if (m_loaded) {
        return;
    }

#ifdef HAVE_POPPLER
    PopplerGlobalParams gp;
    PdfExtractorOutputDevice device;
    m_doc->m_popplerDoc->displayPageSlice(&device, m_pageNum + 1, 72, 72, 0, false, true, false, -1, -1, -1, -1);
    device.finalize();
    const auto pageRect = m_doc->m_popplerDoc->getPage(m_pageNum + 1)->getCropBox();
    std::unique_ptr<GooString> s(device.getText(pageRect->x1, pageRect->y1, pageRect->x2, pageRect->y2));

#if KPOPPLER_VERSION >= QT_VERSION_CHECK(0, 72, 0)
    m_text = QString::fromUtf8(s->c_str());
#else
    m_text = QString::fromUtf8(s->getCString());
#endif
    m_images = std::move(device.m_images);
    for (auto it = m_images.begin(); it != m_images.end(); ++it) {
        (*it).d->m_page = this;
    }
#endif
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

#ifdef HAVE_POPPLER
static double ratio(double begin, double end, double ratio)
{
    return begin + (end - begin) * ratio;
}
#endif

QString PdfPage::textInRect(double left, double top, double right, double bottom) const
{
#ifdef HAVE_POPPLER
    PopplerGlobalParams gp;

    const auto page = d->m_doc->m_popplerDoc->getPage(d->m_pageNum + 1);
    const auto pageRect = page->getCropBox();

    double l, t, r, b;
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
    d->m_doc->m_popplerDoc->displayPageSlice(&device, d->m_pageNum + 1, 72, 72, 0, false, true, false, -1, -1, -1, -1);
    std::unique_ptr<GooString> s(device.getText(l, t, r, b));
#if KPOPPLER_VERSION >= QT_VERSION_CHECK(0, 72, 0)
    return QString::fromUtf8(s->c_str());
#else
    return QString::fromUtf8(s->getCString());
#endif
#else
    Q_UNUSED(left)
    Q_UNUSED(top)
    Q_UNUSED(right)
    Q_UNUSED(bottom)
    return {};
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
#ifdef HAVE_POPPLER
    PopplerGlobalParams gp;
    const auto pageRect = d->m_doc->m_popplerDoc->getPage(d->m_pageNum + 1)->getCropBox();

    for (const auto &img : d->m_images) {
        if ((img.d->m_transform.dx() >= ratio(pageRect->x1, pageRect->x2, left) && img.d->m_transform.dx() <= ratio(pageRect->x1, pageRect->x2, right)) &&
            (img.d->m_transform.dy() >= ratio(pageRect->y1, pageRect->y2, top)  && img.d->m_transform.dy() <= ratio(pageRect->y1, pageRect->y2, bottom)))
        {
            l.push_back(QVariant::fromValue(img));
        }
    }
#else
    Q_UNUSED(left)
    Q_UNUSED(top)
    Q_UNUSED(right)
    Q_UNUSED(bottom)
#endif
    return l;
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
#ifdef HAVE_POPPLER
    return d->m_popplerDoc->getNumPages();
#else
    return 0;
#endif
}

PdfPage PdfDocument::page(int index) const
{
    return d->m_pages[index];
}

int PdfDocument::fileSize() const
{
    return d->m_pdfData.size();
}

#ifdef HAVE_POPPLER
#if KPOPPLER_VERSION >= QT_VERSION_CHECK(21, 8, 0)
static QDateTime parsePdfDateTime(const GooString *str)
#else
static QDateTime parsePdfDateTime(const char *str)
#endif
{
    int year, month, day, hour, min, sec, tzHours, tzMins;
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
        return QDateTime(date, time, Qt::OffsetFromUTC, offset);
    } else if (tz == '-') {
        return QDateTime(date, time, Qt::OffsetFromUTC, -offset);
    }
    return QDateTime(date, time, Qt::UTC);
}
#endif

QDateTime PdfDocument::creationTime() const
{
#ifdef HAVE_POPPLER
    std::unique_ptr<GooString> dt(d->m_popplerDoc->getDocInfoCreatDate());
    if (!dt) {
        return {};
    }
#if KPOPPLER_VERSION >= QT_VERSION_CHECK(21, 8, 0)
    return parsePdfDateTime(dt.get());
#elif KPOPPLER_VERSION >= QT_VERSION_CHECK(0, 72, 0)
    return parsePdfDateTime(dt->c_str());
#else
    return parsePdfDateTime(dt->getCString());
#endif
#else
    return {};
#endif
}

QDateTime PdfDocument::modificationTime() const
{
#ifdef HAVE_POPPLER
    std::unique_ptr<GooString> dt(d->m_popplerDoc->getDocInfoModDate());
    if (!dt) {
        return {};
    }
#if KPOPPLER_VERSION >= QT_VERSION_CHECK(21, 8, 0)
    return parsePdfDateTime(dt.get());
#elif KPOPPLER_VERSION >= QT_VERSION_CHECK(0, 72, 0)
    return parsePdfDateTime(dt->c_str());
#else
    return parsePdfDateTime(dt->getCString());
#endif
#else
    return {};
#endif
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
#ifdef HAVE_POPPLER
    PopplerGlobalParams gp;

    std::unique_ptr<PdfDocument> doc(new PdfDocument(parent));
    doc->d->m_pdfData = data;
    // PDFDoc takes ownership of stream
#if KPOPPLER_VERSION >= QT_VERSION_CHECK(0, 58, 0)
    auto stream = new MemStream(const_cast<char*>(doc->d->m_pdfData.constData()), 0, doc->d->m_pdfData.size(), Object());
#else
    Object obj;
    obj.initNull();
    auto stream = new MemStream(const_cast<char*>(doc->d->m_pdfData.constData()), 0, doc->d->m_pdfData.size(), &obj);
#endif
    std::unique_ptr<PDFDoc> popplerDoc(new PDFDoc(stream, nullptr, nullptr));
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
#else
    Q_UNUSED(data)
    Q_UNUSED(parent)
    return nullptr;
#endif
}

bool PdfDocument::maybePdf(const QByteArray &data)
{
    return data.startsWith("%PDF");
}
