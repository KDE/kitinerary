/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vdvticketparser.h"
#include "vdvdata_p.h"
#include "vdvcertificate_p.h"
#include "iso9796_2decoder_p.h"
#include "logging.h"

#include "../tlv/berelement.h"

#include <QByteArray>
#include <QDebug>

using namespace KItinerary;

VdvTicketParser::VdvTicketParser() = default;
VdvTicketParser::~VdvTicketParser() = default;

bool VdvTicketParser::parse(const QByteArray &data)
{
    // (1) find the certificate authority reference (CAR) to identify the key to decode the CV certificate
    const auto sig = BER::TypedElement<TagSignature>(data);
    if (!sig.isValid()) {
        qCDebug(Log) << "Invalid VDV ticket signature.";
        return false;
    }
    const auto sigRemainder = BER::TypedElement<TagSignatureRemainder>(data, sig.size());
    if (!sigRemainder.isValid()) {
        qCDebug(Log) << "Invalid VDV signature remainder.";
        return false;
    }

    const auto cvCertOffset = sig.size() + sigRemainder.size();
    auto cvCert = VdvCertificate(data, cvCertOffset);
    if ((!cvCert.isValid() && !cvCert.needsCaKey())) {
        qCDebug(Log) << "Invalid CV signature:" << cvCert.isValid() << cvCertOffset << cvCert.size();
        return false;
    }

    const auto carOffset = cvCertOffset + cvCert.size();
    const auto carBlock = BER::TypedElement<TagCaReference>(data, carOffset);
    if (!carBlock.isValid()) {
        qCDebug(Log) << "Invalid CA Reference.";
        return false;
    }
    const auto car = carBlock.contentAt<VdvCaReference>(0);
    if (!car) {
        qCDebug(Log) << "Cannot obtain CA Reference.";
        return false;
    }
    qCDebug(Log) << "CV CAR:" << QByteArray(car->region, 5) << car->serviceIndicator << car->discretionaryData << car->algorithmReference << car->year;

    const auto caCert = VdvPkiRepository::caCertificate(car);
    if (!caCert.isValid()) {
        qCWarning(Log) << "Could not find CA certificate" << QByteArray(reinterpret_cast<const char*>(car), sizeof(VdvCaReference)).toHex();
        return false;
    }

    // (2) decode the CV certificate
    cvCert.setCaCertificate(caCert);
    if (!cvCert.isValid()) {
        qCWarning(Log) << "Failed to decode CV certificate.";
        return false;
    }

    // (3) decode the ticket data using the decoded CV certificate
    Iso9796_2Decoder decoder;
    decoder.setRsaParameters(cvCert.modulus(), cvCert.modulusSize(), cvCert.exponent(), cvCert.exponentSize());
    decoder.addWithRecoveredMessage(sig.contentData(), sig.contentSize());
    decoder.add(sigRemainder.contentData(), sigRemainder.contentSize());

    // (4) profit!
    m_ticket = VdvTicket(decoder.recoveredMessage(), data);
    return true;
}

bool VdvTicketParser::maybeVdvTicket(const QByteArray& data)
{
    if (data.size() < 352) {
        return false;
    }

    // signature header
    const auto sig = BER::TypedElement<TagSignature>(data);
    if (!sig.isValid()) {
        return false;
    }
    const auto rem = BER::TypedElement<TagSignatureRemainder>(data, sig.size());
    if (!rem.isValid()) {
        return false;
    }

    // verify the "VDV" marker is there
    return strncmp((const char*)(rem.contentData() + rem.contentSize() - 5), "VDV", 3) == 0;
}

VdvTicket VdvTicketParser::ticket() const
{
    return m_ticket;
}
