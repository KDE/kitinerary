/*
 * SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KITINERARY_OPENSSLPP_P_H
#define KITINERARY_OPENSSLPP_P_H

#include <functional>
#include <memory>

#include <openssl/bn.h>
#include <openssl/rsa.h>

namespace openssl {

    namespace detail {
        template <typename T, void(*F)(T*)>
        struct deleter {
            void operator()(T *ptr) { std::invoke(F, ptr); }
        };
    }

    using bn_ptr = std::unique_ptr<BIGNUM, detail::deleter<BIGNUM, &BN_free>>;
    using rsa_ptr = std::unique_ptr<RSA, detail::deleter<RSA, &RSA_free>>;
}

#endif
