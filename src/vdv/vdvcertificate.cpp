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
#include "iso9796_2decoder_p.h"

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
    const auto certKeyBlock = hdr->contentAt<VdvCertificateKeyBlock>(0);
    if (certKeyBlock->isValid()) {
        m_type = Raw;
        qDebug() << "found decrypted key";
        qDebug() << "car:" << QByteArray(certKey()->car.region, 2) << QByteArray(certKey()->car.name, 3);
        qDebug() << "chr:" << QByteArray(certKey()->chr.name, 5) << certKey()->chr.algorithmReference << certKey()->chr.year;
        qDebug() << "cha:" << QByteArray(certKey()->cha.name, 6);
        qDebug() << "modulus:" << modulusSize() << *modulus() << *(modulus() + modulusSize() - 1) << (modulus() - (const uint8_t*)certKey());
        qDebug() << "exponent:" << exponentSize() << *exponent() << *(exponent() + exponentSize() - 1) << (exponent() - (const uint8_t*)certKey());
        return;
    }

    const auto sig = hdr->contentAt<VdvCertificateSignature>(0);
    if (!sig->isValid()) {
        qWarning() << "Invalid certificate content: neither a key nor a signature!";
        m_data.clear();
        return;
    }

    m_type = Signed;
    qDebug() << "found encrypted key";
}

VdvCertificate::~VdvCertificate() = default;

bool VdvCertificate::isValid() const
{
    if (m_type == Invalid) {
        return false;
    }
    return m_type == Signed ? !m_recoveredData.isEmpty() : !m_data.isEmpty();
}

bool VdvCertificate::needsCaKey() const
{
    return m_type == Signed && m_recoveredData.isEmpty();
}

int VdvCertificate::size() const
{
    return m_type == Invalid ? 0 : header()->size();
}

uint16_t VdvCertificate::modulusSize() const
{
    switch (certKey()->certificateProfileIdentifier) {
        case 3:
            return 1536 / 8;
        case 4:
            return 1024 / 8;
        case 7:
            return 1984 / 8;
    }
    qWarning() << "Unknown certificate profile identifier: " << certKey()->certificateProfileIdentifier;
    return 0;
}

const uint8_t* VdvCertificate::modulus() const
{
    const auto k = certKey();
    return (&k->oidBegin) + k->oidSize();
}

uint16_t VdvCertificate::exponentSize() const
{
    return 4;
}

const uint8_t* VdvCertificate::exponent() const
{
    return modulus() + modulusSize();
}

void VdvCertificate::setCaCertificate(const VdvCertificate &caCert)
{
    if (!caCert.isValid()) {
        qWarning() << "Invalid CA certificate.";
        return;
    }

    Iso9796_2Decoder decoder;
    decoder.setRsaParameters(caCert.modulus(), caCert.modulusSize(), caCert.exponent(), caCert.exponentSize());

    const auto sig = header()->contentAt<VdvCertificateSignature>(0);
    decoder.addWithRecoveredMessage(sig->contentData(), sig->contentSize());

    if (header()->contentSize() > sig->size()) {
        const auto rem = header()->contentAt<VdvCertificateSignatureRemainder>(sig->size());
        if (rem->isValid() && rem->size() + sig->size() >= header()->contentSize()) {
            decoder.add(rem->contentData(), rem->contentSize());
        } else {
            qWarning() << "Invalid signature remainder!" << rem->isValid() << rem->size() << sig->size() << header()->contentSize();
        }
        qDebug() << rem->isValid() << rem->contentOffset() << rem->contentSize();
    }

    m_recoveredData = decoder.recoveredMessage();
    qDebug() << m_recoveredData.toHex() << m_recoveredData.size();
    if (!m_recoveredData.isEmpty() && m_recoveredData.size() >= (certKey()->headerSize() + modulusSize() + exponentSize())) {
        qDebug() << "successfully decrypted key";
        qDebug() << "car:" << QByteArray(certKey()->car.region, 2) << QByteArray(certKey()->car.name, 3);
        qDebug() << "chr:" << QByteArray(certKey()->chr.name, 5) << certKey()->chr.algorithmReference << certKey()->chr.year;
        qDebug() << "cha:" << QByteArray(certKey()->cha.name, 6);
        qDebug() << "modulus:" << modulusSize() << *modulus() << *(modulus() + modulusSize() - 1) << (modulus() - (const uint8_t*)certKey());
        qDebug() << "exponent:" << exponentSize() << *exponent() << *(exponent() + exponentSize() - 1) << (exponent() - (const uint8_t*)certKey());
    } else {
        qWarning() << "decrypting certificate key failed!";
        qDebug() << "size is:" << m_recoveredData.size() << "expected:" << (certKey()->headerSize() + modulusSize() + exponentSize());
        qDebug() << QByteArray((const char*)caCert.modulus(), caCert.modulusSize()).toHex();
        qDebug() << QByteArray((const char*)caCert.exponent(), caCert.exponentSize()).toHex();
        qDebug() << QByteArray((const char*)sig->contentData(), sig->contentSize()).toHex();;
        m_type = Invalid;
        m_recoveredData.clear();
    }
}

const VdvCertificateHeader* VdvCertificate::header() const
{
    return reinterpret_cast<const VdvCertificateHeader*>(m_data.constData() + m_offset);
}

const VdvCertificateKey* VdvCertificate::certKey() const
{
    if (m_type == Signed) {
        return reinterpret_cast<const VdvCertificateKey*>(m_recoveredData.constData());
    } else if (m_type == Raw) {
        return header()->contentAt<VdvCertificateKeyBlock>(0)->contentAt<VdvCertificateKey>(0);
    }
    return nullptr;
}


VdvCertificate VdvPkiRepository::caCertificate(const VdvCaReference *car)
{
    QFile f(QLatin1String(":/org.kde.pim/kitinerary/vdv/certs/")
        + QString::fromLatin1(QByteArray(reinterpret_cast<const char*>(car), sizeof(VdvCaReference)).toHex())
        + QLatin1String(".vdv-cert"));
    if (!f.open(QFile::ReadOnly)) {
        qWarning() << "Failed to open CA cert file" << f.fileName() << f.errorString();
        return VdvCertificate();
    }

    VdvCertificate cert(f.readAll());
    if (cert.needsCaKey()) {
        VdvCaReference rootCAR;
        rootCAR.region[0] = 'E'; rootCAR.region[1] = 'U';
        rootCAR.name[0] = 'V'; rootCAR.name[1] = 'D'; rootCAR.name[2] = 'V';
        rootCAR.serviceIndicator = 0;
        rootCAR.discretionaryData = 1;
        rootCAR.algorithmReference = 1;
        rootCAR.year = 6;
        cert.setCaCertificate(caCertificate(&rootCAR));
    }
    return cert;
}
