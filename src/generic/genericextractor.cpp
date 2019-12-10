/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "genericextractor_p.h"

using namespace KItinerary;

GenericExtractor::Result::Result() = default;

GenericExtractor::Result::Result(const QJsonArray &result, const QVariant &barcode)
    : result(result)
    , m_barcode(barcode)
{
}

GenericExtractor::Result::~Result() = default;

bool KItinerary::GenericExtractor::Result::isEmpty() const
{
    return result.isEmpty() && m_barcode.isNull();
}

QVariant GenericExtractor::Result::barcode() const
{
    return m_barcode;
}

int GenericExtractor::Result::pageNumber() const
{
    return m_pageNum;
}

void GenericExtractor::Result::setPageNumber(int pageNum)
{
    m_pageNum = pageNum;
}
