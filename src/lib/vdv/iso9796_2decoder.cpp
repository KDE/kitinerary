/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "iso9796_2decoder_p.h"

#include "openssl/bignum_p.h"

#include <QDebug>

#include <openssl/bn.h>
#include <openssl/err.h>

using namespace KItinerary;

Iso9796_2Decoder::Iso9796_2Decoder() = default;
Iso9796_2Decoder::~Iso9796_2Decoder() = default;

void Iso9796_2Decoder::setRsaParameters(const uint8_t *modulus, uint16_t modulusSize, const uint8_t *exponent, uint16_t exponentSize)
{
    auto n = Bignum::fromByteArray(modulus, modulusSize);
    auto e = Bignum::fromByteArray(exponent, exponentSize);

    openssl::ossl_param_bld_ptr param_bld(OSSL_PARAM_BLD_new());
    OSSL_PARAM_BLD_push_BN(param_bld.get(), "n", n.get());
    OSSL_PARAM_BLD_push_BN(param_bld.get(), "e", e.get());
    openssl::ossl_param_ptr params(OSSL_PARAM_BLD_to_param(param_bld.get()));

    openssl::evp_pkey_ctx_ptr ctx(EVP_PKEY_CTX_new_from_name(nullptr, "RSA", nullptr));
    EVP_PKEY_fromdata_init(ctx.get());
    EVP_PKEY *pkey = nullptr;
    if (const auto res = EVP_PKEY_fromdata(ctx.get(), &pkey, EVP_PKEY_PUBLIC_KEY, params.get()); res <= 0) {
        qWarning() << ERR_error_string(ERR_get_error(), nullptr);
    }

    m_key.reset(pkey);
}

void Iso9796_2Decoder::addWithRecoveredMessage(const uint8_t *data, int size)
{
    openssl::evp_pkey_ctx_ptr ctx(EVP_PKEY_CTX_new(m_key.get(), nullptr));
    EVP_PKEY_verify_recover_init(ctx.get());
    EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_NO_PADDING);
    std::size_t outSize = 0;
    if (EVP_PKEY_verify_recover(ctx.get(), nullptr, &outSize, data, size) <= 0) {
        qWarning() << "RSA error:" << ERR_error_string(ERR_get_error(), nullptr);
        return;
    }

    QByteArray out;
    out.resize((qsizetype)outSize);
    EVP_PKEY_verify_recover(ctx.get(), reinterpret_cast<uint8_t*>(out.data()), &outSize, data, size);
    if ((uint8_t)out[0] != 0x6a || (uint8_t)out[out.size() - 1] != 0xbc || out.size() < 22) { // 20 byte SHA-1 + padding/trailer
        qWarning() << "RSA message recovery failed:" << out.toHex() << outSize;
        return;
    }

    m_recoveredMsg.append(out.constData() + 1, out.size() - 22);
}

void Iso9796_2Decoder::add(const uint8_t *data, int size)
{
    if (m_recoveredMsg.isEmpty()) { // previous failure
        return;
    }
    m_recoveredMsg.append((const char*)data, size);
}

QByteArray Iso9796_2Decoder::recoveredMessage() const
{
    return m_recoveredMsg;
}
