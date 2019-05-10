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

#ifndef KITINERARY_JSAPI_BITARRAY_H
#define KITINERARY_JSAPI_BITARRAY_H

#include <QByteArray>
#include <QMetaType>

namespace KItinerary {
namespace JsApi {

/** Bit array methods for JS extractor scripts. */
class BitArray
{
    Q_GADGET
public:
    BitArray();
    explicit BitArray(const QByteArray &data);
    ~BitArray();

    /** Reads a @p size bit long numerical value in most significant bit first format starting at bit offset @p startBit. */
    Q_INVOKABLE int readNumberMSB(int startBit, int size) const;

private:
    QByteArray m_data;
};

}}

Q_DECLARE_METATYPE(KItinerary::JsApi::BitArray)

#endif // KITINERARY_JSAPI_BITARRAY_H
