/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
    QString fieldName() const;
    /** Check if @p data matches this filter. */
    bool matches(const QString &data) const;
    /** Pattern to match field value against. */
    QString pattern() const;

    ///@cond internal
    /** Load filter from @p obj. */
    bool load(const QJsonObject &obj);
    /** Serialize filter to a JSON object. */
    QJsonObject toJson() const;

    void setType(ExtractorInput::Type type);
    void setFieldName(const QString &fieldName);
    void setPattern(const QString &pattern);
    ///@endcond

private:
    QExplicitlySharedDataPointer<ExtractorFilterPrivate> d;
};

}

#endif // EXTRACTORFILTER_H
