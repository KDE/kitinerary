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

#ifndef STRUCTUREDDATAEXTRACTOR_H
#define STRUCTUREDDATAEXTRACTOR_H

#include "kitinerary_export.h"

#include <QJsonArray>

class QJsonObject;
class QString;
class QXmlStreamReader;

/** Extract schema.org structured data from HTML text.
 *  @see https://developers.google.com/gmail/markup/getting-started
 */
class KITINERARY_EXPORT StructuredDataExtractor
{
public:
    void parse(const QString &text);
    QJsonArray data() const
    {
        return m_data;
    }

private:
    /** Try to parse using an actual XML parser. */
    void parseXml(const QString &text);
    /** Try to find application/ld+json content with basic string search. */
    void findLdJson(const QString &text);
    /** Try to fix some common HTML4 damage to make @p text consumable for parseXml(). */
    QString fixupHtml4(const QString &text) const;
    /** Recursive microdata parsing. */
    QJsonObject parseMicroData(QXmlStreamReader &reader) const;
    /** Element-dependent Microdata property value. */
    QString valueForItemProperty(QXmlStreamReader &reader) const;

    QJsonArray m_data;
};

#endif // STRUCTUREDDATAEXTRACTOR_H
