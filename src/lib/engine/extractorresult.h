/*
    SPDX-FileCopyrightText: 2019-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include <QJsonArray>
#include <QList>
#include <QVariant>

namespace KItinerary {

/** Generic extraction result.
 *  This can represent results both in JSON-LD serialized form and in QVariant decoded form.
 */
class KITINERARY_EXPORT ExtractorResult
{
public:
    ExtractorResult();
    ExtractorResult(const QJsonArray &result);
    ExtractorResult(const QList<QVariant> &result);
    ~ExtractorResult();

    /** Checks if there is any relevant result set in here. */
    bool isEmpty() const;
    /** Amount of contained result elements. */
    int size() const;

    /** JSON-LD data extracted from this document or page. */
    QJsonArray jsonLdResult() const;
    /** Result in decoded form. */
    QList<QVariant> result() const;

    /** Append another result to this one. */
    void append(ExtractorResult &&other);

private:
    mutable QJsonArray m_jsonLdResult;
    mutable QList<QVariant> m_result;
};

}

