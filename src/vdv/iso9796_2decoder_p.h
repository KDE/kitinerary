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

#ifndef KITINERARY_ISO9796_2DECODER_P_H
#define KITINERARY_ISO9796_2DECODER_P_H

#include "config-kitinerary.h"

#include <QByteArray>

#ifdef HAVE_OPENSSL_RSA
#include <openssl/rsa.h>
#endif

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
#ifdef HAVE_OPENSSL_RSA
    std::unique_ptr<RSA, void(*)(RSA*)> m_rsa;
#endif
    QByteArray m_recoveredMsg;
};

}

#endif // KITINERARY_ISO9796_2DECODER_P_H
