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

#ifndef KITINERARY_VDVTICKETPARSER_H
#define KITINERARY_VDVTICKETPARSER_H

#include "kitinerary_export.h"
#include "vdvticket.h"

class QByteArray;

namespace KItinerary {

/** Parser for VDV tickets.
 *  Or more correctly for: "Statische Berechtigungen der VDV-Kernapplikation"
 *  That is, a standard for 2D barcode tickets for local public transport, commonly found in Germany
 *  and some neighbouring countries.
 *
 *  This is based on "VDV-Kernapplikation - Spezifikation statischer Berechtigungen für 2D Barcode-Tickets"
 *  which your favorite search engine should find as a PDF.
 *
 *  The crypto stuff used here is ISO 9796-2, and you'll find some terminology also used in ISO 7816-6/8,
 *  which isn't entirely surprising given this also exists in a NFC card variant.
 *
 *  Do not use directly, only installed for use in tooling.
 */
class KITINERARY_EXPORT VdvTicketParser
{
public:
    VdvTicketParser();
    ~VdvTicketParser();

    /** Tries to parse the ticket in @p data. */
    bool parse(const QByteArray &data);

    /** Returns the parsed ticket data. */
    VdvTicket ticket() const;

    /** Fast check if @p data might contain a VDV ticket.
     *  Does not perform full decoding, mainly useful for content auto-detection.
     */
    static bool maybeVdvTicket(const QByteArray &data);

private:
    VdvTicket m_ticket;
};

}

#endif // KITINERARY_VDVTICKETPARSER_H
