/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QByteArray>

#include <openssl/opensslpp_p.h>

#include <cstdint>
#include <memory>

namespace KItinerary {

/** Message recovery for ISO 9796-2 Schema 1 signatures.
 *  This does not care at all about security or actually validating the signature,
 *  this is merely about recoverying the part of the signed message that is mangled
 *  by the signature.
 */
class Iso9796_2Decoder
{
public:
    Iso9796_2Decoder();
    ~Iso9796_2Decoder();

    /** Set RSA modulus and exponents (@see VdvCertificate). */
    void setRsaParameters(const uint8_t *modulus, uint16_t modulusSize, const uint8_t *exponent, uint16_t exponentSize);

    /** Process the first block of data, containing the recoverable message and the SHA-1 hash. */
    void addWithRecoveredMessage(const uint8_t *data, int size);
    /** Process any further data. */
    void add(const uint8_t *data, int size);

    /** Returns the recovered message.
     *  This should either be modulusSize - 22 bytes, or empty in case of an error.
     */
    QByteArray recoveredMessage() const;

private:
    openssl::rsa_ptr m_rsa;
    QByteArray m_recoveredMsg;
};

}

