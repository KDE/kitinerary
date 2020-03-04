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

#ifndef KITINERARY_VDVCERTIFICATE_P_H
#define KITINERARY_VDVCERTIFICATE_P_H

#include <QByteArray>

class QDate;
class QIODevice;

namespace KItinerary {

namespace BER { class Element; }
struct VdvCaReference;
struct VdvCertificateKey;

/** Certificate object, to obtain the RSA parameters.
 *  This can be both a raw certificate which can be directly consumed,
 *  or one with an ISO 9796-2 signature with message recovery. In the latter
 *  case you need to provide the key of the corresponding CA certificate
 *  for decoding too.
*/
class VdvCertificate
{
public:
    VdvCertificate();
    explicit VdvCertificate(const QByteArray &data, int offset = 0);
    ~VdvCertificate();

    bool isValid() const;
    bool needsCaKey() const;

    /** Size of the entire encoded certificate data. */
    int size() const;

    /** Amount of bytes in the RSA modulus. */
    uint16_t modulusSize() const;
    /** RSA modulus. */
    const uint8_t* modulus() const;

    /** Amount of bytes in the RSA exponent. */
    uint16_t exponentSize() const;
    /** RSA exponent. */
    const uint8_t* exponent() const;

    /** Sets the CA certificate for decoding ISO 9796-2 signed certificates. */
    void setCaCertificate(const VdvCertificate &caCert);

    /** Write the key to @p out, in ISO 9796-2 format, without signatures. */
    void writeKey(QIODevice *out) const;

    /** Returns whether this is a self-signed (== root) certificate. */
    bool isSelfSigned() const;
    /** Returns the date this certificate expires. */
    QDate endOfValidity() const;

private:
    BER::Element header() const;
    const VdvCertificateKey *certKey() const;

    QByteArray m_data;
    QByteArray m_recoveredData;
    int m_offset = 0;
    enum CertificateType {
        Invalid,
        Raw,
        Signed
    } m_type = Invalid;
};

/** VDV (sub)CA certificate access. */
namespace VdvPkiRepository
{
    /** Returns the (sub)CA certificate for the given CA Reference (CAR). */
    VdvCertificate caCertificate(const VdvCaReference *car);
}

}

#endif // KITINERARY_VDVCERTIFICATE_P_H
