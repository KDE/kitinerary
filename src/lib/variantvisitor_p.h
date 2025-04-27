/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_VARIANTVISITOR_H
#define KITINERARY_VARIANTVISITOR_H

#include <QVariant>

#include <type_traits>

namespace KItinerary {

/** Like std::visit, but for a fixed set of types inside a QVariant. */
template <typename Func>
class VariantVisitor
{
private:
    Func m_func;

public:
    explicit VariantVisitor(Func func)
        : m_func(func) {}

    template <typename T, typename ...Ts>
    auto inline visit(const QVariant &v) -> decltype(m_func(v.value<T>())) {
        using RetT = decltype(m_func(v.value<T>()));
        return visitImpl<RetT, T, Ts...>(v);
    }

private:
    template <typename RetT>
    inline RetT visitImpl(const QVariant&) {
        if constexpr (std::is_same_v<RetT, void>) {
            return;
        } else {
            return {};
        }
    }
    template <typename RetT, typename T, typename ...Ts>
    inline RetT visitImpl(const QVariant &v) {
        if (v.typeId() == qMetaTypeId<T>()) {
            return m_func(v.value<T>());
        }
        return visitImpl<RetT, Ts...>(v);
    }
};

template <typename T, typename U, typename ...Us>
struct is_any_of : std::conditional_t<std::is_same_v<std::decay_t<T>, U>, std::true_type, is_any_of<T, Us...>>{};

template <typename T, typename U>
struct is_any_of<T, U> : std::is_same<std::decay_t<T>, U>{};

template <typename T, typename U, typename ...Us>
inline constexpr bool is_any_of_v = is_any_of<T, U, Us...>::value;

}

#endif
