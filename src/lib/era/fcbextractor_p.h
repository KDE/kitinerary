/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_FCBEXTRACTOR_H
#define KITINERARY_FCBEXTRACTOR_H

#include "era/fcbticket.h"

#include <QVariant>

class QString;

namespace KItinerary {

class Organization;
class Person;

/** Generic extractor functions for FCB tickets. */
class FcbExtractor
{
public:
    /** Human-readable name of the ticket, typically what would be printed on top. */
    [[nodiscard]] static QString ticketName(const Fcb::UicRailTicketData &fcb);

    /** Reference number/PNR. */
    [[nodiscard]] static QString pnr(const Fcb::UicRailTicketData &fcb);

    /** Seating class, if available. */
    [[nodiscard]] static QString seatingType(const Fcb::UicRailTicketData &fcb);

    /** Issuer organization of this ticket. */
    [[nodiscard]] static QString issuerId(const Fcb::UicRailTicketData &fcb);
    [[nodiscard]] static Organization issuer(const Fcb::UicRailTicketData &fcb);
    /** Traveller who this ticket is for. */
    [[nodiscard]] static Person person(const Fcb::UicRailTicketData &fcb);

    /** Ticket validity range. */
    [[nodiscard]] static QDateTime validFrom(const Fcb::UicRailTicketData &fcb);
    [[nodiscard]] static QDateTime validUntil(const Fcb::UicRailTicketData &fcb);
};

}

#endif
