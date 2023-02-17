/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "rsp6decoder.h"

#include <openssl/bignum_p.h>
#include <openssl/opensslpp_p.h>

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

static std::vector<openssl::rsa_ptr> loadKeys(std::string_view keyId)
{
    qDebug() << "looking for key:" << QLatin1String(keyId.data(), keyId.size());

    const QString keyFileName = QLatin1String(":/org.kde.pim/kitinerary/rsp6/keys/") + QLatin1String(keyId.data(), keyId.size()) + QLatin1String(".json");
    QFile keyFile(keyFileName);
    if (!keyFile.open(QFile::ReadOnly)) {
        qWarning() << "failed to open RSP-6 key file:" << keyFileName << keyFile.errorString();
        return {};
    }

    const auto keysArray = QJsonDocument::fromJson(keyFile.readAll()).array();
    std::vector<openssl::rsa_ptr> keys;
    keys.reserve(keysArray.size());

    for (const auto &keyVal : keysArray) {
        const auto keyObj = keyVal.toObject();
        auto n = Bignum::fromByteArray(QByteArray::fromBase64(keyObj.value(QLatin1String("n")).toString().toLatin1()));
        auto e = Bignum::fromByteArray(QByteArray::fromBase64(keyObj.value(QLatin1String("e")).toString().toLatin1()));

        openssl::rsa_ptr rsa(RSA_new());
        RSA_set0_key(rsa.get(), n.release(), e.release(), nullptr);
        keys.push_back(std::move(rsa));
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
    for (const auto &rsa : keys) {
        decrypted.resize(RSA_size(rsa.get()));
        const auto decryptedSize = RSA_public_decrypt(decoded.size(), reinterpret_cast<const uint8_t*>(decoded.data()), reinterpret_cast<uint8_t*>(decrypted.data()), rsa.get(), RSA_PKCS1_PADDING);
        if (decryptedSize < 0) {
            qDebug() << "RSA error:" << ERR_error_string(ERR_get_error(), nullptr);
            continue;
        }
        decrypted.resize(decryptedSize);
        break;
    }

    return decrypted;
}
