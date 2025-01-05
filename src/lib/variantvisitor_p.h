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

}

#endif
