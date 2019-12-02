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

#include "iso9796_2decoder_p.h"

#include <QDebug>

#ifdef HAVE_OPENSSL_RSA
#include <ssl/bn.h>
#endif

using namespace KItinerary;

Iso9796_2Decoder::Iso9796_2Decoder()
#ifdef HAVE_OPENSSL_RSA
    : m_rsa(RSA_new(), RSA_free)
#endif
{
}

Iso9796_2Decoder::~Iso9796_2Decoder() = default;

void Iso9796_2Decoder::setRsaParameters(const uint8_t *modulus, uint16_t modulusSize, const uint8_t *exponent, uint16_t exponentSize)
{
#ifdef HAVE_OPENSSL_RSA
    const auto n = BN_bin2bn(modulus, modulusSize, nullptr);
    const auto e = BN_bin2bn(exponent, exponentSize, nullptr);
    RSA_set0_key(m_rsa.get(), n, e, nullptr); // takes ownership of n and e
#else
    Q_UNUSED(modulus);
    Q_UNUSED(modulusSize);
    Q_UNUSED(exponent);
    Q_UNUSED(exponentSize);
#endif
}

void Iso9796_2Decoder::addWithRecoveredMessage(const uint8_t *data, int size)
{
#ifdef HAVE_OPENSSL_RSA
    QByteArray out;
    out.resize(RSA_size(m_rsa.get()));
    const auto outSize = RSA_public_decrypt(size, data, (uint8_t*)out.data(), m_rsa.get(), RSA_NO_PADDING);
    out.resize(outSize);
    qDebug() << outSize << out.toHex();
#else
    Q_UNUSED(data);
    Q_UNUSED(size);
#endif
}

void Iso9796_2Decoder::add(const uint8_t *data, int size)
{
#ifdef HAVE_OPENSSL_RSA
    // TODO
#else
    Q_UNUSED(data);
    Q_UNUSED(size);
#endif
}

QByteArray Iso9796_2Decoder::recoveredMessage() const
{
    return {}; // TODO
}
