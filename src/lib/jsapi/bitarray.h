/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

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
    Q_INVOKABLE quint64 readNumberMSB(int startBit, int size) const;

private:
    QByteArray m_data;
};

}}

