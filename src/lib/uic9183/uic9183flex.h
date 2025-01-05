/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_UIC9183FLEX_H
#define KITINERARY_UIC9183FLEX_H

#include "kitinerary_export.h"
#include "uic9183block.h"

#include "era/fcbticket.h"

namespace KItinerary {

class TrainStation;

/** Represents a U_FLEX block holding different versions of an FCB payload.
 *  @see ERA TAP TSI TD B.12 Digital Security Elements For Rail Passenger Ticketing - §11 FCB - Flexible Content Barcode
 */
class KITINERARY_EXPORT Uic9183Flex
{
    Q_GADGET
    Q_PROPERTY(QVariant fcb READ fcbVariant)

public:
    Uic9183Flex();
    /** Parse U_FLEX block in @p data.
     *  It's the callers responsibility to ensure @p data outlives this instance, the data
     *  is not copied.
     *  @param block A UIC 918.3 U_FLEX data block
     */
    Uic9183Flex(const Uic9183Block &block);
    Uic9183Flex(const Uic9183Flex&);
    ~Uic9183Flex();
    Uic9183Flex& operator=(const Uic9183Flex&);

    /** Returns whether this is a valid U_FLEX layout block. */
    [[nodiscard]] bool isValid() const;

    /** Issuing date/time. */
    [[nodiscard]] QDateTime issuingDateTime() const;

    /** @c true when this is a valid FCB with at least one transport document. */
    [[nodiscard]] bool hasTransportDocument() const;
    /** Transport documents of the contained FCB. */
    [[nodiscard]] QList<QVariant> transportDocuments() const;

    /** Returns the FCB payload.
     *  Varies depending on the version of this block.
     */
    [[nodiscard]] Fcb::v13::UicRailTicketData fcb() const;

    /** Read departure station info from the given FCB travel document, if applicable. */
    static void readDepartureStation(const QVariant &doc, TrainStation &station);
    /** Read arrival station info from the given FCB travel document, if applicable. */
    static void readArrivalStation(const QVariant &doc, TrainStation &station);
    /** Fix known issues with station identifiers. */
    static void fixStationCode(TrainStation &station);

    static constexpr const char RecordId[] = "U_FLEX";

private:
    [[nodiscard]] QVariant fcbVariant() const;

    Uic9183Block m_block;
    Fcb::v13::UicRailTicketData m_data;
};

}

#endif
