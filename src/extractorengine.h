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

#ifndef EXTRACTORENGINE_H
#define EXTRACTORENGINE_H

#include "kitinerary_export.h"
#include "extractor.h"

#include <QJsonArray>
#include <QString>

#include <vector>

class QDateTime;

namespace KItinerary {

class ContextObject;

/** Code for executing an extractor rule set on a specific email part. */
class KITINERARY_EXPORT ExtractorEngine
{
public:
    ExtractorEngine();
    ~ExtractorEngine();

    void setExtractor(const Extractor *extractor);
    void setText(const QString &text);
    /** The date the email containing the processed text was sent. */
    void setSenderDate(const QDateTime &dt);

    QJsonArray extract();

private:
    void executeScript();

    const Extractor *m_extractor = nullptr;
    ContextObject *m_context = nullptr;
    QString m_text;
    QJsonArray m_result;
};

}

#endif // EXTRACTORENGINE_H
