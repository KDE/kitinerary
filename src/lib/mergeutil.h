/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include <QVariant>

#include <functional>

namespace KItinerary {
class Flight;
class Person;

/** Utilities for merging reservations or elements of them. */
class MergeUtil
{
public:
    /**
     *  Checks if two Reservation or Trip values refer to the same booking element.
     *
     *  This does not mean being exactly equal, but having matching identifying properties.
     *  What this means exactly depends on the data type:
     *  - Flights: booking reference, flight number and departure day match
     *  - Train trip: booking reference, train number and departure day match
     *  - Bus trip: booking ref and/or number and departure time match
     *  - Hotel booking: hotel name, booking reference and checkin day match
     *
     *  For all reservation types, the Reservation::underName property is
     *  checked and either needs to be equal or absent in one of the values.
     */
    KITINERARY_EXPORT static bool isSame(const QVariant &lhs, const QVariant &rhs);

    /**
     * Checks if two Person objects refer to the same person.
     *
     * Essentially a case-insensitive comparison of the name components.
     */
    KITINERARY_EXPORT static bool isSamePerson(const Person &lhs, const Person &rhs);

    /**
     *  Checks whether to elements refer to the same thing, just for different people.
     *  For example two reservations for the same trip or event, but with separate tickets
     *  for different attendees.
     *  This is useful for batching elements together.
     *  @since 5.23.41
     */
    KITINERARY_EXPORT static bool isSameIncidence(const QVariant &lhs, const QVariant &rhs);

    /**
     * Checks whether two transport reservation elements refer to the same departure.
     * This considers time, location and mode of transport.
     */
    static bool hasSameDeparture(const QVariant &lhs, const QVariant &rhs);

    /**
     * Checks whether two transport reservation elements refer to the same arrival.
     * This considers time, location and mode of transport.
     */
    static bool hasSameArrival(const QVariant &lhs, const QVariant &rhs);

    /**
     * Merge the two given objects.
     * This is the same as JsonLdDocument::apply in most cases, but if one side
     * can be determined to be "better", that one is preferred.
     */
    KITINERARY_EXPORT static QVariant merge(const QVariant &lhs, const QVariant &rhs);

    /** Merge the common parts of @p other into reservations @p res, both representating the same incidence.
     *  Behavior is undefined if isSameIncidence(lhs, rhs) would be @c false.
     *  @see MergeUtil::isSameIncidence
     *  @since 25.12
     */
    static QVariant mergeIncidence(const QVariant &res, const QVariant &other);

    /** Register a comparator function for a custom type that will be used by isSame. */
    template <typename T>
    inline static void registerComparator(bool(*func)(const T&, const T&))
    {
        std::function<bool(const QVariant&, const QVariant &)> f([func](const QVariant &lhs, const QVariant &rhs) -> bool {
            return (*func)(lhs.value<T>(), rhs.value<T>());
        });
        registerComparator(qMetaTypeId<T>(), std::move(f));
    }

private:
    KITINERARY_EXPORT static void registerComparator(int metaTypeId, std::function<bool(const QVariant&, const QVariant &)> &&func);
};

}

