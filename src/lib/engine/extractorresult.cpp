/*
    SPDX-FileCopyrightText: 2019-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "extractorresult.h"

#include <KItinerary/JsonLdDocument>

using namespace KItinerary;

ExtractorResult::ExtractorResult() = default;

ExtractorResult::ExtractorResult(const QJsonArray &result)
    : m_jsonLdResult(result)
{
}

ExtractorResult::ExtractorResult(const QList<QVariant> &result)
    : m_result(result) {}

ExtractorResult::~ExtractorResult() = default;

bool ExtractorResult::isEmpty() const
{
    return m_result.isEmpty() && m_jsonLdResult.isEmpty();
}

int ExtractorResult::size() const
{
    return std::max(m_result.size(), m_jsonLdResult.size());
}

QJsonArray ExtractorResult::jsonLdResult() const
{
    if (m_jsonLdResult.isEmpty()) {
        m_jsonLdResult = JsonLdDocument::toJson(m_result);
    }
    return m_jsonLdResult;
}

QList<QVariant> ExtractorResult::result() const {
    if (m_result.isEmpty()) {
        m_result = JsonLdDocument::fromJson(m_jsonLdResult);
    }
    return m_result;
}

void ExtractorResult::append(ExtractorResult &&other)
{
    if (other.isEmpty()) {
        return;
    }

    if (isEmpty()) {
        m_result = std::move(other.m_result);
        m_jsonLdResult = std::move(other.m_jsonLdResult);
        return;
    }

    if (!m_result.isEmpty()) {
        auto r = other.result();
        m_result.reserve(m_result.size() + r.size());
        std::copy(r.begin(), r.end(), std::back_inserter(m_result));
    }
    if (!m_jsonLdResult.isEmpty()) {
        auto r = other.jsonLdResult();
        std::copy(r.begin(), r.end(), std::back_inserter(m_jsonLdResult));
    }
}
