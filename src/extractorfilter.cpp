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

#include "extractorfilter.h"

#include <QJsonObject>

using namespace KItinerary;

ExtractorFilter::ExtractorFilter() = default;
ExtractorFilter::~ExtractorFilter() = default;

ExtractorFilter::Type ExtractorFilter::type() const
{
    return m_type;
}

const char *ExtractorFilter::fieldName() const
{
    return m_fieldName.constData();
}

bool ExtractorFilter::matches(const QString &data) const
{
    return m_exp.match(data).hasMatch();
}

bool ExtractorFilter::load(const QJsonObject &obj)
{
    auto it = obj.find(QLatin1String("header"));
    if (it != obj.end()) {
        m_fieldName = it.value().toString().toUtf8();
        m_type = Mime;
    }

    it = obj.find(QLatin1String("field"));
    if (it != obj.end()) {
        m_fieldName = it.value().toString().toUtf8();
        m_type = PkPass;
    }

    it = obj.find(QLatin1String("property"));
    if (it != obj.end()) {
        m_fieldName = it.value().toString().toUtf8();
        m_type = JsonLd;
    }

    if (m_type == Undefined) {
        const auto typeName = obj.value(QLatin1String("type"));
        if (typeName == QLatin1String("Barcode")) {
            m_type = Barcode;
        }
    }

    m_exp.setPattern(obj.value(QLatin1String("match")).toString());
    return m_type != Undefined && (!m_fieldName.isEmpty() || m_type == Barcode) && m_exp.isValid();
}
