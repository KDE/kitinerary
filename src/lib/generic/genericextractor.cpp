/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "genericextractor_p.h"

#include <KItinerary/JsonLdDocument>

using namespace KItinerary;

GenericExtractor::Result::Result() = default;

GenericExtractor::Result::Result(const QJsonArray &result, const QVariant &barcode)
    : m_jsonLdResult(result)
    , m_barcode(barcode)
{
}

GenericExtractor::Result::Result(const QVector<QVariant>& result, const QVariant& barcode)
    : m_result(result)
    , m_barcode(barcode)
{
}

GenericExtractor::Result::~Result() = default;

bool KItinerary::GenericExtractor::Result::isEmpty() const
{
    return m_result.isEmpty() && m_jsonLdResult.isEmpty() && m_barcode.isNull();
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

QJsonArray KItinerary::GenericExtractor::Result::jsonLdResult() const
{
    if (m_jsonLdResult.isEmpty()) {
        m_jsonLdResult = JsonLdDocument::toJson(m_result);
    }
    return m_jsonLdResult;
}

QVector<QVariant> KItinerary::GenericExtractor::Result::result() const
{
    if (m_result.isEmpty()) {
        m_result = JsonLdDocument::fromJson(m_jsonLdResult);
    }
    return m_result;
}
