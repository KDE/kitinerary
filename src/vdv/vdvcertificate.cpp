/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vdvcertificate_p.h"
#include "vdvdata_p.h"
#include "iso9796_2decoder_p.h"

#include <QDate>
#include <QDebug>
#include <QFile>

using namespace KItinerary;

VdvCertificate::VdvCertificate() = default;

VdvCertificate::VdvCertificate(const QByteArray &data, int offset)
    : m_offset(offset)
{
    const auto hdr = BER::TypedElement<TagCertificate>(data, offset);
    if (!hdr.isValid()) {
        qDebug() << "Invalid certificate header:" << hdr.isValid() << data.size() << offset;
        return;
    }

    m_data = data;
    const auto certKeyBlock = hdr.find(TagCertificateContent);
    if (certKeyBlock.isValid()) {
        m_type = Raw;
        qDebug() << "found decrypted key";
        qDebug() << "CHR:" << QByteArray(certKey()->chr.name, 5) << certKey()->chr.algorithmReference << certKey()->chr.year;
        qDebug() << "CAR:" << QByteArray(certKey()->car.region, 2) << QByteArray(certKey()->car.name, 3);
        return;
    }

    const auto sig = hdr.find(TagCertificateSignature);
    if (!sig.isValid()) {
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
    return m_type == Invalid ? 0 : header().size();
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

    const auto sig = header().find(TagCertificateSignature);
    decoder.addWithRecoveredMessage(sig.contentData(), sig.contentSize());

    if (header().contentSize() > sig.size()) {
        const auto rem = header().find(TagCertificateSignatureRemainder);
        if (rem.isValid()) {
            decoder.add(rem.contentData(), rem.contentSize());
        } else {
            qWarning() << "Invalid signature remainder!" << rem.isValid() << rem.size() << sig.size() << header().contentSize();
        }
    }

    m_recoveredData = decoder.recoveredMessage();
    if (!m_recoveredData.isEmpty() && m_recoveredData.size() >= (certKey()->headerSize() + modulusSize() + exponentSize())) {
        qDebug() << "successfully decrypted key";
        qDebug() << "CAR:" << QByteArray(certKey()->car.region, 2) << QByteArray(certKey()->car.name, 3);
        qDebug() << "CHR:" << QByteArray(certKey()->chr.name, 5) << certKey()->chr.algorithmReference << certKey()->chr.year;
    } else {
        qWarning() << "decrypting certificate key failed!";
        qDebug() << "size is:" << m_recoveredData.size() << "expected:" << (certKey()->headerSize() + modulusSize() + exponentSize());
        qDebug() << QByteArray((const char*)sig.contentData(), sig.contentSize()).toHex();
        m_type = Invalid;
        m_recoveredData.clear();
    }
}

void VdvCertificate::writeKey(QIODevice *out) const
{
    out->write("\x7F\x21");
    if (m_type == Signed) {
        BER::Element::writeSize(out, m_recoveredData.size() + 3);
        out->write("\x5F\x4E");
        BER::Element::writeSize(out, m_recoveredData.size());
        out->write(m_recoveredData);
    } else if (m_type == Raw) {
        const auto keyBlock = header().find(TagCertificateContent);
        BER::Element::writeSize(out, keyBlock.size());
        out->write(keyBlock.rawData(), keyBlock.size());
    }
}

bool VdvCertificate::isSelfSigned() const
{
    return memcmp(&certKey()->car, certKey()->chr.name, sizeof(VdvCaReference)) == 0;
}

QDate KItinerary::VdvCertificate::endOfValidity() const
{
    const auto key = certKey();
    return QDate(key->date.year(), key->date.month(), key->date.day());
}

BER::Element VdvCertificate::header() const
{
    return BER::Element(m_data, m_offset);
}

const VdvCertificateKey* VdvCertificate::certKey() const
{
    if (m_type == Signed) {
        return reinterpret_cast<const VdvCertificateKey*>(m_recoveredData.constData());
    } else if (m_type == Raw) {
        return header().find(TagCertificateContent).contentAt<VdvCertificateKey>();
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
