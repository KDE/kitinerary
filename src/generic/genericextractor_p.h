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

#ifndef KITINERARY_GENERICEXTRACTOR_P_H
#define KITINERARY_GENERICEXTRACTOR_P_H

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

#endif // KITINERARY_GENERICEXTRACTOR_P_H
