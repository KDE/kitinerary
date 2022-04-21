/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include <QDebug>
#include <QString>
#include <QStringView>

#include <cstdint>
#include <functional>

namespace KItinerary {
namespace KnowledgeDb {

///@cond internal
namespace Internal {
    // "private" API for the template code below
    KITINERARY_EXPORT uint32_t alphaIdFromString(QStringView s, int size);
    KITINERARY_EXPORT QString alphaIdToString(uint32_t id, int size);
}
///@endcond

/**
 *  Compact storage for fixed-size identifiers consisting out of uppercase latin letters,
 *  such as IATA airport codes or ISO 3166 country codes.
 */
template <typename T, int N>
class AlphaId {
    static_assert((N * 5) < (sizeof(T) * 8), "Not enough space to hold identifier.");
public:
    inline constexpr AlphaId() = default;
    /** Create identifier from a literal. */
    inline explicit constexpr AlphaId(const char s[N])
    {
        for (int i = 0; i < N; ++i) {
            if (s[i] < 'A' || s[i] > 'Z') {
                m_id = {};
                return;
            }
            m_id |= (s[i] - '@') << (5 * (N-i-1));
        }
    }
    /** Create identifier from a QString. */
    inline explicit AlphaId(QStringView s)
    {
        m_id = Internal::alphaIdFromString(s, N);
    }

    /** Returns @c true if this is a valid identifier. */
    inline constexpr bool isValid() const
    {
        return m_id != 0;
    }

    inline constexpr bool operator<(AlphaId<T, N> rhs) const
    {
        return m_id < rhs.m_id;
    }
    inline constexpr bool operator==(AlphaId<T, N> other) const
    {
        return m_id == other.m_id;
    }
    inline constexpr bool operator!=(AlphaId<T, N> other) const
    {
        return m_id != other.m_id;
    }

    /** Returns a string representation of this identifier. */
    inline QString toString() const
    {
        return Internal::alphaIdToString(m_id, N);
    }

    /** @internal for std::hash */
    inline constexpr T value() const
    {
        return m_id;
    }
private:
    T m_id = {};
};

}

}

template <typename T, int N>
inline QDebug operator<<(QDebug dbg, KItinerary::KnowledgeDb::AlphaId<T, N> id)
{
    dbg << id.toString();
    return dbg;
}

namespace std {
template <typename T, int N> struct hash<KItinerary::KnowledgeDb::AlphaId<T, N>>
{
    typedef KItinerary::KnowledgeDb::AlphaId<T, N> argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type id) const noexcept
    {
        return std::hash<T>()(id.value());
    }
};
}
