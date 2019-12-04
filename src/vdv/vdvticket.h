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

#ifndef KITINERARY_VDVTICKET_H
#define KITINERARY_VDVTICKET_H

#include "kitinerary_export.h"

#include <QDateTime>
#include <QExplicitlySharedDataPointer>
#include <QMetaType>

namespace KItinerary {

class VdvTicketPrivate;

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

public:
    VdvTicket();
    VdvTicket(const QByteArray &data);
    VdvTicket(const VdvTicket&);
    ~VdvTicket();
    VdvTicket& operator=(const VdvTicket&);

    QDateTime beginDateTime() const;
    QDateTime endDateTime() const;

private:
    QExplicitlySharedDataPointer<VdvTicketPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::VdvTicket)

#endif // KITINERARY_VDVTICKET_H
