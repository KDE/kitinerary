/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include <KItinerary/Organization>
#include <KItinerary/Person>
#include <KItinerary/Place>

#include <QExplicitlySharedDataPointer>
#include <QMetaType>

class QDateTime;

namespace KItinerary {

class Rct2Ticket;
class Uic9183Block;
class Uic9183Header;
class Uic9183ParserPrivate;
class Uic9183TicketLayout;

/** Parser for UIC 918.3 and 918.3* train tickets.
 *
 *  @see https://www.era.europa.eu/sites/default/files/filesystem/tap/baseline_1.2.0_archive/era_technical_document_tap_b_7_v1.2.pdf
 *    for information about the general UIC 918-3 structure
 *  @see https://www.era.europa.eu/sites/default/files/filesystem/tap/baseline_1.2.0_archive/era_technical_document_tap_b_6_v1.2.pdf
 *    for information about the U_TLAY block
 *  @see https://www.bahn.de/p/view/angebot/regio/barcode.shtml
 *    for information about the 0080VU vendor block
 */
class KITINERARY_EXPORT Uic9183Parser
{
    Q_GADGET
    Q_PROPERTY(QString pnr READ pnr)
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString carrierId READ carrierId)
    Q_PROPERTY(KItinerary::Organization issuer READ issuer)
    Q_PROPERTY(QDateTime validFrom READ validFrom)
    Q_PROPERTY(QDateTime validUntil READ validUntil)
    Q_PROPERTY(KItinerary::Person person READ person)
    Q_PROPERTY(KItinerary::TrainStation outboundDepartureStation READ outboundDepartureStation)
    Q_PROPERTY(KItinerary::TrainStation outboundArrivalStation READ outboundArrivalStation)
    Q_PROPERTY(KItinerary::TrainStation returnDepartureStation READ returnDepartureStation)
    Q_PROPERTY(KItinerary::TrainStation returnArrivalStation READ returnArrivalStation)
    Q_PROPERTY(QString seatingType READ seatingType)
    /** U_TLAY ticket layout block, if present, @c null otherwise. */
    Q_PROPERTY(QVariant ticketLayout READ ticketLayoutVariant)
    /** RCT2 ticket layout block, if present, @c null otherwise. */
    Q_PROPERTY(QVariant rct2Ticket READ rct2TicketVariant)

    Q_PROPERTY(QByteArray rawData READ rawData STORED false)

public:
    Uic9183Parser();
    Uic9183Parser(const Uic9183Parser&);
    ~Uic9183Parser();
    Uic9183Parser& operator=(const Uic9183Parser&);

    /** Date/time this ticket was first encountered.
     *  This is used to recover a missing year in the ticket data.
     */
    [[deprecated("calling this is longer needed")]] void setContextDate(const QDateTime&);

    void parse(const QByteArray &data);
    bool isValid() const;

    /** The booking reference. */
    QString pnr() const;
    /** Ticket name. */
    QString name() const;
    /** The UIC carrier code. */
    QString carrierId() const;
    /** Full issuer organization element. */
    Organization issuer() const;
    /** Begin of validity. */
    QDateTime validFrom() const;
    /** End of validity. */
    QDateTime validUntil() const;
    /** The person this ticket is issued to. */
    Person person() const;

    /** Station object for the departure station of the outbound trip. */
    TrainStation outboundDepartureStation() const;
    /** Station object for the arrival station of the outbound trip. */
    TrainStation outboundArrivalStation() const;
    /** Station object for the departure station of the return trip. */
    TrainStation returnDepartureStation() const;
    /** Station object for the arrival station of the return trip. */
    TrainStation returnArrivalStation() const;

    /** @see Ticket::seatingType */
    QString seatingType() const;

    /** U_TLAY ticket layout block. */
    Uic9183TicketLayout ticketLayout() const;

    /** RCT2 ticket layout, if present. */
    Rct2Ticket rct2Ticket() const;

    /** First data block in this ticket.
     *  Useful for iterating over all blocks.
     */
    Uic9183Block firstBlock() const;
    /** Returns the first block with the given name.
     *  A null block is returned if no such block exists.
     */
    Uic9183Block findBlock(const char name[6]) const;
    /** Returns the first block of type @tparam T.
     *  A null block is returned if no such block exists.
     */
    template <typename T>
    inline T findBlock() const
    {
        return T(findBlock(T::RecordId));
    }

    /** Same as the above, but for JS usage. */
    Q_INVOKABLE QVariant block(const QString &name) const;

    /** Header found before the compressed payload. */
    Uic9183Header header() const;

    /** Raw data of this ticket.
     *  Useful for generating a barcode for it again.
     */
    QByteArray rawData() const;

    /** Quickly checks if @p might be UIC 918.3 content.
     *  This prioritizes speed over correctness and is used in barcode content auto-detection.
     */
    static bool maybeUic9183(const QByteArray &data);

private:
    QVariant ticketLayoutVariant() const;
    QVariant rct2TicketVariant() const;
    QExplicitlySharedDataPointer<Uic9183ParserPrivate> d;
};

}
