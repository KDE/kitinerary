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

#include "extractor.h"
#include "extractorfilter.h"
#include "logging.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>

#include <memory>

using namespace KItinerary;

namespace KItinerary {
class ExtractorPrivate
{
public:
    QString m_scriptName;
    std::vector<ExtractorFilter> m_filters;
};
}

Extractor::Extractor()
    : d(new ExtractorPrivate)
{
}
Extractor::Extractor(Extractor &&) = default;
Extractor::~Extractor() = default;

bool Extractor::load(const QJsonObject &obj, const QString &baseDir)
{
    for (const auto &filterValue : obj.value(QLatin1String("filter")).toArray()) {
        ExtractorFilter f;
        if (!f.load(filterValue.toObject())) {
            return false;
        }
        d->m_filters.push_back(std::move(f));
    }

    const auto scriptName = obj.value(QLatin1String("script")).toString();
    d->m_scriptName = baseDir + QLatin1Char('/') + scriptName;

    if (d->m_scriptName.isEmpty() || !QFile::exists(d->m_scriptName)) {
        qCWarning(Log) << "Script file not found:" << d->m_scriptName;
        return false;
    }

    return !d->m_filters.empty();
}

QString Extractor::scriptFileName() const
{
    return d->m_scriptName;
}

const std::vector<ExtractorFilter> &Extractor::filters() const
{
    return d->m_filters;
}
