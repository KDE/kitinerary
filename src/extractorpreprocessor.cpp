/*
   Copyright (c) 2017 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#include "config-kitinerary.h"

#include "extractorpreprocessor.h"
#include "logging.h"

#ifdef HAVE_POPPLER
#include <poppler-qt5.h>
#endif

#include <memory>

using namespace KItinerary;

void ExtractorPreprocessor::preprocessPlainText(const QString &input)
{
    m_buffer = input;
}

void ExtractorPreprocessor::preprocessHtml(const QString &input)
{
    m_buffer.reserve(input.size());
    int begin = 0;
    int end = input.indexOf(QLatin1Char('<'), begin);
    while (begin < input.size() && end < input.size() && end >= 0 && begin >= 0) {
        if (end > begin) {
            replaceEntityAndAppend(input.midRef(begin, end - begin));
        }
        begin = input.indexOf(QLatin1Char('>'), end);
        if (begin < 0) {
            break;
        }

        // replace elements with something suitable for field separation
        const auto elementName = input.mid(end + 1, begin - end - 1);
        if (elementName.startsWith(QLatin1String("br"), Qt::CaseInsensitive)) {
            m_buffer.append(QLatin1Char('\n'));
        } else {
            m_buffer.append(QLatin1Char(' '));
        }

        ++begin;
        end = input.indexOf(QLatin1Char('<'), begin);
    }
    if (begin >= 0 && end < 0) {
        replaceEntityAndAppend(input.midRef(begin));
    }
    //qCDebug(Log) << "Preprocessed HTML content: " << m_buffer;
}

void ExtractorPreprocessor::preprocessPdf(const QByteArray &input)
{
#ifdef HAVE_POPPLER
    std::unique_ptr<Poppler::Document> doc(Poppler::Document::loadFromData(input));
    if (!doc || doc->isLocked()) {
        return;
    }

    for (int i = 0, total = doc->numPages(); i < total; ++i) {
        std::unique_ptr<Poppler::Page> page(doc->page(i));
        m_buffer += page->text({}, Poppler::Page::PhysicalLayout);
    }
#else
    Q_UNUSED(input);
#endif
}

QString ExtractorPreprocessor::text() const
{
    return m_buffer;
}

void ExtractorPreprocessor::replaceEntityAndAppend(const QStringRef &source)
{
    int begin = 0;
    int end = source.indexOf(QLatin1Char('&'), begin);
    while (begin < source.size() && end < source.size() && end >= 0 && begin >= 0) {
        if (end > begin) {
            m_buffer.append(source.mid(begin, end - begin));
        }
        begin = source.indexOf(QLatin1Char(';'), end);
        if (begin < 0) {
            break;
        }
        const auto entityName = source.mid(end + 1, begin - end - 1);
        if (entityName == QLatin1String("nbsp")) {
            m_buffer.append(QLatin1Char(' '));
        } else {
            // keep unknown entities
            m_buffer.append(source.mid(end, begin - end + 1));
        }
        ++begin;
        end = source.indexOf(QLatin1Char('&'), begin);
    }
    if (begin >= 0 && end < 0) {
        m_buffer.append(source.mid(begin));
    }
}
