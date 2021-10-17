/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "iso9796_2decoder_p.h"

#include <QDebug>

#include <openssl/bn.h>
#include <openssl/err.h>

using namespace KItinerary;

Iso9796_2Decoder::Iso9796_2Decoder()
    : m_rsa(RSA_new(), RSA_free)
{
}

Iso9796_2Decoder::~Iso9796_2Decoder() = default;

void Iso9796_2Decoder::setRsaParameters(const uint8_t *modulus, uint16_t modulusSize, const uint8_t *exponent, uint16_t exponentSize)
{
    const auto n = BN_bin2bn(modulus, modulusSize, nullptr);
    const auto e = BN_bin2bn(exponent, exponentSize, nullptr);
    RSA_set0_key(m_rsa.get(), n, e, nullptr); // takes ownership of n and e
}

void Iso9796_2Decoder::addWithRecoveredMessage(const uint8_t *data, int size)
{
    QByteArray out;
    out.resize(RSA_size(m_rsa.get()));
    const auto outSize = RSA_public_decrypt(size, data, (uint8_t*)out.data(), m_rsa.get(), RSA_NO_PADDING);
    if (outSize < 0) {
        qWarning() << "RSA error:" << ERR_error_string(ERR_get_error(), nullptr);
        return;
    }

    out.resize(outSize);
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
