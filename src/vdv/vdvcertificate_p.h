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

#ifndef KITINERARY_VDVCERTIFICATE_H
#define KITINERARY_VDVCERTIFICATE_H

#include <QByteArray>

namespace KItinerary {

struct VdvCertificateHeader;
struct VdvCertificateKey;

/** Certificate object, to obtain the RSA parameters. */
class VdvCertificate
{
public:
    VdvCertificate();
    explicit VdvCertificate(const QByteArray &data, int offset = 0);
    ~VdvCertificate();

    bool isValid() const;

    /** Amount of bytes in the RSA modulus. */
    uint16_t modulusSize() const;
    /** RSA modulus. */
    const uint8_t* modulus() const;

    /** Amount of bytes in the RSA exponent. */
    uint16_t exponentSize() const;
    /** RSA exponent. */
    const uint8_t* exponent() const;

private:
    const VdvCertificateHeader *header() const;
    const VdvCertificateKey *certKey() const;

    QByteArray m_data;
    int m_offset = 0;
};

/** VDV (sub)CA certificate access. */
namespace VdvPkiRepository
{
    /** Returns the (sub)CA certificate for the given serial number. */
    VdvCertificate caCertificate(uint8_t serNum);
}

}

#endif // KITINERARY_VDVCERTIFICATE_H
