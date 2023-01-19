/*
    SPDX-FileCopyrightText: 2018-2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_INTERNAL_INSTANCE_COUNTER_H
#define KITINERARY_INTERNAL_INSTANCE_COUNTER_H

#include <QDateTime>
#include <QTimeZone>

namespace KItinerary {

/** Implementation details of template code.
 *  @internal
 */
namespace detail {

/** Compile-time instance counter.
 *  This is based on the approach described in https://woboq.com/blog/verdigris-implementation-tricks.html
 *  Useful for daisy-chaining overloads in the same context.
 */
template <int N = 255> struct num : public num<N - 1> {
    static constexpr int value = N;
    static constexpr num<N - 1> prev() { return {}; }
};
template <> struct num<0> { static constexpr int value = 0; };

/** type tag, to avoid unwanted overload resolution on arguments other than num<> */
template <typename T> struct tag {};

}
}

#endif
