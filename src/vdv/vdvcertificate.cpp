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

#include "vdvcertificate_p.h"
#include "vdvdata_p.h"

#include <QDebug>
#include <QFile>

using namespace KItinerary;

VdvCertificate::VdvCertificate() = default;

VdvCertificate::VdvCertificate(const QByteArray &data, int offset)
    : m_offset(offset)
{
    if ((unsigned)data.size() <= m_offset + sizeof(VdvCertificateHeader)) {
        qWarning() << "Certificate data too small:" << data.size() << offset;
        return;
    }

    m_data = data;
    const auto hdr = header();
    if (!hdr->isValid() || data.size() < hdr->size() + offset) {
        qWarning() << "Invalid certificate header:" << hdr->isValid() << hdr->size() << data.size() << offset;
        m_data.clear();
        return;
    }

    qDebug() << "key:" << certKey()->isValid();
    qDebug() << "car:" << QByteArray(certKey()->car.region, 2) << QByteArray(certKey()->car.name, 3);
    qDebug() << "chr:" << QByteArray(certKey()->chr.name, 5);
    qDebug() << "cha:" << QByteArray(certKey()->cha.name, 6);
    qDebug() << "modulus:" << modulusSize() << *modulus() << *(modulus() + modulusSize() - 1);
    qDebug() << "exponent:" << exponentSize() << *exponent() << *(exponent() + exponentSize() - 1);
}

VdvCertificate::~VdvCertificate() = default;

bool VdvCertificate::isValid() const
{
    return !m_data.isEmpty();
}

uint16_t VdvCertificate::modulusSize() const
{
    switch (certKey()->certificateProfileIdentifier) {
        case 3:
            return 1536 / 8;
        case 4:
            return 1024 / 8;
    }
    qWarning() << "Unknown certificate profile identifier: " << certKey()->certificateProfileIdentifier;
    return 0;
}

const uint8_t* VdvCertificate::modulus() const
{
    return &(certKey()->modulusBegin);
}

uint16_t VdvCertificate::exponentSize() const
{
    return 4;
}

const uint8_t* VdvCertificate::exponent() const
{
    return &(certKey()->modulusBegin) + modulusSize();
}

const VdvCertificateHeader* VdvCertificate::header() const
{
    return reinterpret_cast<const VdvCertificateHeader*>(m_data.constData() + m_offset);
}

const VdvCertificateKey* VdvCertificate::certKey() const
{
    // TODO check if m_data is large enough
    return reinterpret_cast<const VdvCertificateKey*>(m_data.constData() + m_offset + header()->contentOffset());
}


VdvCertificate VdvPkiRepository::caCertificate(uint8_t serNum)
{
    QFile f(QLatin1String(":/org.kde.pim/kitinerary/vdv/certs/") + QString::number(serNum) + QLatin1String(".vdv-cert"));
    if (!f.open(QFile::ReadOnly)) {
        qWarning() << "Failed to open CA cert file" << serNum << f.errorString();
        return VdvCertificate();
    }

    qDebug() << f.size();
    return VdvCertificate(f.readAll());
}
