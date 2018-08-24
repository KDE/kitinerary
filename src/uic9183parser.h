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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KITINERARY_UIC9183PARSER_H
#define KITINERARY_UIC9183PARSER_H

#include "kitinerary_export.h"

#include <KItinerary/Person>

#include <QExplicitlySharedDataPointer>
#include <QMetaType>

namespace KItinerary {

class Uic9183ParserPrivate;

/** Parser for UIC 918-3 and 918-3* train tickets.
 *
 *  @see http://www.era.europa.eu/Document-Register/Documents/ERA_Technical_Document_TAP_B_7_v1.2.pdf
 *    for information about the general UIC 918-3 structure
 *  @see http://www.era.europa.eu/Document-Register/Documents/ERA_Technical_Document_TAP_B_6_v1_2.pdf
 *    for information about the U_TLAY block
 *  @see https://www.bahn.de/p/view/angebot/regio/barcode.shtml
 *    for information about the 0080VU vendor block
 */
class KITINERARY_EXPORT Uic9183Parser
{
    Q_GADGET
    Q_PROPERTY(QString pnr READ pnr)
    Q_PROPERTY(KItinerary::Person person READ person)
    Q_PROPERTY(QString outboundDepartureStationId READ outboundDepartureStationId)
    Q_PROPERTY(QString outboundArrivalStationId READ outboundArrivalStationId)

public:
    Uic9183Parser();
    Uic9183Parser(const Uic9183Parser&);
    ~Uic9183Parser();
    Uic9183Parser& operator=(const Uic9183Parser&);

    void parse(const QByteArray &data);
    bool isValid() const;

    /** The booking reference. */
    QString pnr() const;
    /** The person this ticket is issued to. */
    Person person() const;

    /** Station identifier for the departure station of the outbound trip. */
    QString outboundDepartureStationId() const;
    /** Station identifier for the arrival station of the outbound trip. */
    QString outboundArrivalStationId() const;

private:
    QExplicitlySharedDataPointer<Uic9183ParserPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::Uic9183Parser)

#endif // KITINERARY_UIC9183PARSER_H
