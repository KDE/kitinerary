/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "rsp6decoder.h"

#include "openssl/bignum_p.h"
#include "openssl/opensslpp_p.h"

#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

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

static std::vector<openssl::evp_pkey_ptr> loadKeys(std::string_view keyId)
{
  qDebug() << "looking for key:"
           << QLatin1StringView(keyId.data(), keyId.size());

  const QString keyFileName =
      QLatin1StringView(":/org.kde.pim/kitinerary/rsp6/keys/") +
      QLatin1StringView(keyId.data(), keyId.size()) + QLatin1StringView(".json");
  QFile keyFile(keyFileName);
  if (!keyFile.open(QFile::ReadOnly)) {
    qWarning() << "failed to open RSP-6 key file:" << keyFileName
               << keyFile.errorString();
    return {};
    }

    const auto keysArray = QJsonDocument::fromJson(keyFile.readAll()).array();
    std::vector<openssl::evp_pkey_ptr> keys;
    keys.reserve(keysArray.size());

    for (const auto &keyVal : keysArray) {
        const auto keyObj = keyVal.toObject();
        auto n = Bignum::fromByteArray(QByteArray::fromBase64(
            keyObj.value(QLatin1StringView("n")).toString().toLatin1()));
        auto e = Bignum::fromByteArray(QByteArray::fromBase64(
            keyObj.value(QLatin1StringView("e")).toString().toLatin1()));

        openssl::ossl_param_bld_ptr param_bld(OSSL_PARAM_BLD_new());
        OSSL_PARAM_BLD_push_BN(param_bld.get(), "n", n.get());
        OSSL_PARAM_BLD_push_BN(param_bld.get(), "e", e.get());
        openssl::ossl_param_ptr params(OSSL_PARAM_BLD_to_param(param_bld.get()));

        openssl::evp_pkey_ctx_ptr ctx(EVP_PKEY_CTX_new_from_name(nullptr, "RSA", nullptr));
        EVP_PKEY_fromdata_init(ctx.get());
        EVP_PKEY *pkey = nullptr;
        if (const auto res = EVP_PKEY_fromdata(ctx.get(), &pkey, EVP_PKEY_PUBLIC_KEY, params.get()); res <= 0) {
            qWarning() << ERR_error_string(ERR_get_error(), nullptr);
            continue;
        }

        keys.emplace_back(pkey);
    }

    return keys;
}

QByteArray Rsp6Decoder::decode(const QByteArray &data)
{
    // verify version signature and sufficient size
    if (data.size() < 20 || !data.startsWith("06")) {
        return {};
    }

    // load RSA key
    const auto keys = loadKeys(std::string_view(data.data() + 13, 2));
    if (keys.empty()) {
        qWarning() << "no RSP-6 key found for issuer:" << QByteArray(data.data() + 13 , 2);
        return {};
    }

    // remove base26 transport encoding
    const auto decoded = decodeBase45Backward(data.begin() + 15, data.end());

    // decrypt payload, try all keys for the current issuer until we find one that works
    QByteArray decrypted;
    QByteArray depadded;
    for (const auto &key : keys) {
        openssl::evp_pkey_ctx_ptr ctx(EVP_PKEY_CTX_new(key.get(), nullptr));
        EVP_PKEY_verify_recover_init(ctx.get());
        EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_NO_PADDING);
        std::size_t decryptedSize = 0;
        if (EVP_PKEY_verify_recover(ctx.get(), nullptr, &decryptedSize, reinterpret_cast<const uint8_t*>(decoded.data()), decoded.size()) <= 0) {
            continue;
        }
        decrypted.resize((qsizetype)decryptedSize);
        EVP_PKEY_verify_recover(ctx.get(), reinterpret_cast<uint8_t*>(decrypted.data()), &decryptedSize, reinterpret_cast<const uint8_t*>(decoded.data()), decoded.size());

        // we don't know the padding scheme, so we have to try all of them
        depadded.resize((qsizetype)decryptedSize);
        // PKCS#1 type 1
        const auto rsa = openssl::rsa_ptr(EVP_PKEY_get1_RSA(key.get()));
        auto depaddedSize = RSA_padding_check_PKCS1_type_1((uint8_t*)depadded.data(), depadded.size(), (const uint8_t*)decrypted.data(), decrypted.size(), RSA_size(rsa.get()));
        if (depaddedSize > 0) {
            depadded.resize(depaddedSize);
            return depadded;
        }
        // PKCS#1 type 2
        depaddedSize = RSA_padding_check_PKCS1_type_2((uint8_t*)depadded.data(), depadded.size(), (const uint8_t*)decrypted.data(), decrypted.size(), RSA_size(rsa.get()));
        if (depaddedSize > 0) {
            depadded.resize(depaddedSize);
            return depadded;
        }
    }

    return {};
}
