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

ExtractorFilter::ExtractorFilter() = default;
ExtractorFilter::~ExtractorFilter() = default;

const char *ExtractorFilter::headerName() const
{
    return m_headerName.constData();
}

bool ExtractorFilter::matches(const QString &headerData) const
{
    return m_exp.match(headerData).hasMatch();
}

bool ExtractorFilter::load(const QJsonObject& obj)
{
    m_headerName = obj.value(QLatin1String("header")).toString().toUtf8();
    m_exp.setPattern(obj.value(QLatin1String("match")).toString());
    return !m_headerName.isEmpty() && m_exp.isValid();
}
