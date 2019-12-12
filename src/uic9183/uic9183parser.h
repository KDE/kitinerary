/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

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

#ifndef KITINERARY_UIC9183PARSER_H
#define KITINERARY_UIC9183PARSER_H

#include "kitinerary_export.h"

#include <KItinerary/Person>

#include <QExplicitlySharedDataPointer>
#include <QMetaType>

class QDateTime;

namespace KItinerary {

class Rct2Ticket;
class Uic9183Block;
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
    Q_PROPERTY(QString carrierId READ carrierId)
    Q_PROPERTY(KItinerary::Person person READ person)
    Q_PROPERTY(QString outboundDepartureStationId READ outboundDepartureStationId)
    Q_PROPERTY(QString outboundArrivalStationId READ outboundArrivalStationId)
    Q_PROPERTY(QString seatingType READ seatingType)
    /** U_TLAY ticket layout block, if present, @c null otherwise. */
    Q_PROPERTY(QVariant ticketLayout READ ticketLayoutVariant)
    /** RCT2 ticket layout block, if present, @c null otherwise. */
    Q_PROPERTY(QVariant rct2Ticket READ rct2TicketVariant)

public:
    Uic9183Parser();
    Uic9183Parser(const Uic9183Parser&);
    ~Uic9183Parser();
    Uic9183Parser& operator=(const Uic9183Parser&);

    /** Date/time this ticket was first encountered.
     *  This is used to recover a missing year in the ticket data.
     */
    void setContextDate(const QDateTime &contextDt);

    void parse(const QByteArray &data);
    bool isValid() const;

    /** The booking reference. */
    QString pnr() const;
    /** The UIC carrier code. */
    QString carrierId() const;
    /** The person this ticket is issued to. */
    Person person() const;

    /** Station identifier for the departure station of the outbound trip. */
    QString outboundDepartureStationId() const;
    /** Station identifier for the arrival station of the outbound trip. */
    QString outboundArrivalStationId() const;

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

    /** Same as the above, but for JS usage. */
    Q_INVOKABLE QVariant block(const QString &name) const;

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

Q_DECLARE_METATYPE(KItinerary::Uic9183Parser)

#endif // KITINERARY_UIC9183PARSER_H
