/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_VDVTICKET_H
#define KITINERARY_VDVTICKET_H

#include "kitinerary_export.h"

#include <KItinerary/Person>

#include <QDateTime>
#include <QExplicitlySharedDataPointer>
#include <QMetaType>

namespace KItinerary {

class VdvTicketHeader;
class VdvTicketPrivate;
class VdvTicketCommonTransactionData;
class VdvTicketIssueData;
struct VdvTicketTrailer;
namespace BER { class Element; }

/** Ticket information from a VDV barcode.
 *  For use by tooling or custom extractor scripts.
 */
class KITINERARY_EXPORT VdvTicket
{
    Q_GADGET
    /** Begin of the validitiy of this ticket. */
    Q_PROPERTY(QDateTime beginDateTime READ beginDateTime)
    /** End of the validity of this ticket. */
    Q_PROPERTY(QDateTime endDateTime READ endDateTime)

    /** VDV organization identifier of the ticket issuer. */
    Q_PROPERTY(int issuerId READ issuerId)
    /** Service class for this ticket. */
    Q_PROPERTY(ServiceClass serviceClass READ serviceClass)
    /** The person this ticket is valid for. */
    Q_PROPERTY(KItinerary::Person person READ person)
    /** Ticket number. */
    Q_PROPERTY(QString ticketNumber READ ticketNumber)

public:
    VdvTicket();
    VdvTicket(const QByteArray &data);
    VdvTicket(const VdvTicket&);
    ~VdvTicket();
    VdvTicket& operator=(const VdvTicket&);

    QDateTime beginDateTime() const;
    QDateTime endDateTime() const;
    int issuerId() const;

    enum ServiceClass {
        UnknownClass = 0,
        FirstClass = 1,
        SecondClass = 2,
        FirstClassUpgrade = 3
    };
    Q_ENUM(ServiceClass)
    ServiceClass serviceClass() const;

    Person person() const;
    QString ticketNumber() const;

    // low-level content access
    const VdvTicketHeader* header() const;
    BER::Element productData() const;
    const VdvTicketCommonTransactionData* commonTransactionData() const;
    BER::Element productSpecificTransactionData() const;
    const VdvTicketIssueData* issueData() const;
    const VdvTicketTrailer* trailer() const;

private:
    QExplicitlySharedDataPointer<VdvTicketPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::VdvTicket)

#endif // KITINERARY_VDVTICKET_H