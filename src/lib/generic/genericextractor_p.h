/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QJsonArray>
#include <QVariant>
#include <QVector>

namespace KItinerary {

/** Shared bits between all generic extractors. */
namespace GenericExtractor
{

/** Generic extraction result.
 *  This can represent results both in JSON-LD serialized form and in QVariant decoded form.
 */
class Result
{
public:
    Result();
    explicit Result(const QJsonArray &result, const QVariant &barcode = {});
    explicit Result(const QVector<QVariant> &result, const QVariant &barcode = {});
    ~Result();

    /** Checks if there is any relevant result set in here. */
    bool isEmpty() const;

    /** Unrecognized barcode for further processing.
     *  Can be either a QByteArray or a QString.
     */
    QVariant barcode() const;

    /** Page number, if result is from a single PDF page. */
    int pageNumber() const;
    void setPageNumber(int pageNum);

    /** JSON-LD data extracted from this document or page. */
    QJsonArray jsonLdResult() const;
    /** Result in decoded form. */
    QVector<QVariant> result() const;

private:
    mutable QJsonArray m_jsonLdResult;
    mutable QVector<QVariant> m_result;
    QVariant m_barcode;
    int m_pageNum = -1;
};

}

}

