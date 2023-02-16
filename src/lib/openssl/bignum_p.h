/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KITINERARY_BIGNUM_P_H
#define KITINERARY_BIGNUM_P_H

#include "opensslpp_p.h"

#include <QByteArray>

/** Utilities for working with OpenSSL BIGNUM objects. */
class Bignum
{
public:
    /** @see BN_bin2bn */
    static inline openssl::bn_ptr fromByteArray(const uint8_t *bin, std::size_t size)
    {
        return openssl::bn_ptr(BN_bin2bn(bin, size, nullptr));
    }
    static inline openssl::bn_ptr fromByteArray(const char *bin, std::size_t size)
    {
        return openssl::bn_ptr(BN_bin2bn(reinterpret_cast<const uint8_t*>(bin), size, nullptr));
    }
    static inline openssl::bn_ptr fromByteArray(const QByteArray &bin)
    {
        return fromByteArray(bin.constData(), bin.size());
    }

    /** @see BN_bn2bin */
    static inline QByteArray toByteArray(const openssl::bn_ptr &bn)
    {
        QByteArray bin;
        bin.resize(BN_num_bytes(bn.get()));
        BN_bn2bin(bn.get(), reinterpret_cast<uint8_t*>(bin.data()));
        return bin;
    }
};

#endif

