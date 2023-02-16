/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "rsp6decoder.h"

#include <openssl/bignum_p.h>
#include <openssl/opensslpp_p.h>

#include <QByteArray>
#include <QDebug>

#include <openssl/err.h>

using namespace KItinerary;

static QByteArray decodeBase45Backward(const char *begin, const char *end)
{
    openssl::bn_ptr bn(BN_new());
    BN_zero(bn.get());
    for (auto it = end - 1; it >= begin; --it) {
        BN_mul_word(bn.get(), 26);
        if ((*it) < 'A' || (*it) > 'Z') {
            return {};
        }
        BN_add_word(bn.get(), (*it) - 'A');
    }

    auto result = Bignum::toByteArray(bn);
    std::reverse(result.begin(), result.end());
    return result;
}

static openssl::rsa_ptr loadKey(std::string_view keyId)
{
    qDebug() << "looking for key:" << QByteArray(keyId.data(), keyId.size());
    // TODO load correct key
    auto modulus = Bignum::fromByteArray(QByteArray::fromHex("9774021C2BAD13DC31C1EC8192E084D60D82DC327016862E95ED093AD41CC9082A6015631C8E6B8148A15EC9856E2A5E16519E52EDC0C3DF2836935055EF53E1738293256464F0AD4AE3C01AE3CDD910CB1CBDC0AB35C1AD8CF0A9376B3921DEE3D1FC26FFA3409C4DC8F813A5E326D78C63ABA9A59120D74043DBA1141047AF"));
    auto exponent = Bignum::fromByteArray(QByteArray::fromHex("10001"));

    openssl::rsa_ptr rsa(RSA_new());
    RSA_set0_key(rsa.get(), modulus.release(), exponent.release(), nullptr);
    return rsa;
}

QByteArray Rsp6Decoder::decode(const QByteArray &data)
{
    // verify version signature and sufficient size
    if (data.size() < 20 || !data.startsWith("06")) {
        return {};
    }

    // load RSA key
    const auto rsa = loadKey(std::string_view(data.data() + 13, 2));
    if (!rsa) {
        return {};
    }

    // remove base26 transport encoding
    const auto decoded = decodeBase45Backward(data.begin() + 15, data.end());

    // decrypt payload
    QByteArray decrypted;
    decrypted.resize(RSA_size(rsa.get()));
    const auto decryptedSize = RSA_public_decrypt(decoded.size(), reinterpret_cast<const uint8_t*>(decoded.data()), reinterpret_cast<uint8_t*>(decrypted.data()), rsa.get(), RSA_PKCS1_PADDING);
    if (decryptedSize < 0) {
        qWarning() << "RSA error:" << ERR_error_string(ERR_get_error(), nullptr);
        return {};
    }
    decrypted.resize(decryptedSize);
    return decrypted;
}
