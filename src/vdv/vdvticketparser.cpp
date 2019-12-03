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

#include "vdvticketparser.h"
#include "vdvdata_p.h"
#include "vdvcertificate_p.h"
#include "iso9796_2decoder_p.h"

#include <QByteArray>
#include <QDebug>

using namespace KItinerary;

VdvTicketParser::VdvTicketParser() = default;
VdvTicketParser::~VdvTicketParser() = default;

void VdvTicketParser::parse(const QByteArray &data)
{
    qDebug() << data.size();
    if (!maybeVdvTicket(data)) {
        qWarning() << "Input data is not a VDV ticket!";
        return;
    }

    // (1) find the certificate authority reference (CAR) to identify the key to decode the CV certificate
    const auto sigRemainder = reinterpret_cast<const VdvSignatureRemainder*>(data.constData() + VdvSignatureRemainder::Offset);
    if (!sigRemainder->isValid() || VdvSignatureRemainder::Offset + sigRemainder->size() + sizeof(VdvCertificateHeader) > (unsigned)data.size()) {
        qWarning() << "Invalid VDV signature remainder.";
        return;
    }
    qDebug() << sigRemainder->contentSize();

    const auto cvCertOffset = VdvSignatureRemainder::Offset + sigRemainder->size();
    auto cvCert = VdvCertificate(data ,cvCertOffset);
    if ((!cvCert.isValid() && !cvCert.needsCaKey()) || cvCertOffset + cvCert.size() + sizeof(VdvCaReferenceBlock) > (unsigned)data.size()) {
        qWarning() << "Invalid CV signature:" << cvCert.isValid() << cvCertOffset << cvCert.size();
        return;
    }

    const auto carOffset = cvCertOffset + cvCert.size();
    const auto carBlock = reinterpret_cast<const VdvCaReferenceBlock*>(data.constData() + carOffset);
    if (!carBlock->isValid() || carBlock->contentSize() < sizeof(VdvCaReference)) {
        qWarning() << "Invalid CA Reference.";
        return;
    }
    const auto car = carBlock->contentAt<VdvCaReference>(0);
    qDebug() << QByteArray(car->name, 3) << car->serviceIndicator << car->discretionaryData << car->algorithmReference << car->year;

    const auto caCert = VdvPkiRepository::caCertificate(car);
    if (!caCert.isValid()) {
        qWarning() << "Could not find CA certificate" << QByteArray(reinterpret_cast<const char*>(car), sizeof(VdvCaReference)).toHex();
        return;
    }

    // (2) decode the CV certificate
    cvCert.setCaCertificate(caCert);
    if (!cvCert.isValid()) {
        qDebug() << "Failed to decode CV certificate.";
        return;
    }

    // (3) decode the ticket data using the decoded CV certificate
    const auto sig = reinterpret_cast<const VdvSignature*>(data.constData());
    qDebug() << sig->isValid() << sig->contentSize();

    Iso9796_2Decoder decoder;
    decoder.setRsaParameters(cvCert.modulus(), cvCert.modulusSize(), cvCert.exponent(), cvCert.exponentSize());
    decoder.addWithRecoveredMessage(sig->contentData(), sig->contentSize());
    decoder.add(sigRemainder->contentData(), sigRemainder->contentSize());

    // (4) profit!
    qDebug() << decoder.recoveredMessage();
    qDebug() << decoder.recoveredMessage().toHex();
    // TODO
}

bool VdvTicketParser::maybeVdvTicket(const QByteArray& data)
{
    if (data.size() < 352) {
        return false;
    }

    // signature header
    if ((uint8_t)data[0] != TagSignature || (uint8_t)data[1] != 0x81 || (uint8_t)data[2] != 0x80 || (uint8_t)data[VdvSignatureRemainder::Offset] != TagSignatureRemainder) {
        return false;
    }

    const uint8_t len = data[132]; // length of the 0x9A unsigned data block
    if (len + 133 > data.size()) {
        return false;
    }

    // verify the "VDV" marker is there
    return strncmp(data.constData() + 133 + len - 5, "VDV", 3) == 0;
}
