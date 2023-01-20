/*
    SPDX-FileCopyrightText: 2018-2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_INTERNAL_PARAMETER_TYPE_H
#define KITINERARY_INTERNAL_PARAMETER_TYPE_H

#include <type_traits>

namespace KItinerary {
namespace detail {

/** Meta-function to select the right way to pass a parameter of type @tparam T.
 *  @internal
 */
template <typename T>
struct parameter_type
{
    using type = typename std::conditional<std::is_fundamental<T>::value, T, const T&>::type;
};

}
}

#endif
