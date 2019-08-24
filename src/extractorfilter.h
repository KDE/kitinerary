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

#ifndef EXTRACTORFILTER_H
#define EXTRACTORFILTER_H

#include "kitinerary_export.h"

#include "extractorinput.h"

#include <QRegularExpression>
#include <QByteArray>

class QJsonObject;

namespace KItinerary {

class ExtractorFilterPrivate;

/** Determines whether an extractor is applicable to a given email. */
class KITINERARY_EXPORT ExtractorFilter
{
public:
    ExtractorFilter();
    ~ExtractorFilter();
    ExtractorFilter(const ExtractorFilter&);
    ExtractorFilter(ExtractorFilter&&) noexcept;
    ExtractorFilter& operator=(const ExtractorFilter&);
    ExtractorFilter& operator=(ExtractorFilter&&);

    /** The filter type. */
    ExtractorInput::Type type() const;
    /** The field to filter on. */
    const char *fieldName() const;
    /** Check if @p data matches this filter. */
    bool matches(const QString &data) const;
    /** Load filter from @p obj. */
    bool load(const QJsonObject &obj);
    /** Pattern to match field value against. */
    QString pattern() const;

    ///@cond internal
    void setType(ExtractorInput::Type type);
    void setFieldName(const QString &fieldName);
    void setPattern(const QString &pattern);
    ///@endcond

private:
    QExplicitlySharedDataPointer<ExtractorFilterPrivate> d;
};

}

#endif // EXTRACTORFILTER_H
