/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_UPERELEMENT_H
#define KITINERARY_UPERELEMENT_H

namespace KItinerary {

// The property counter thing is based on the approach described here https://woboq.com/blog/verdigris-implementation-tricks.html
namespace detail {
template <int N = 255> struct num : public num<N - 1> {
    static constexpr int value = N;
    static constexpr num<N - 1> prev() { return {}; }
};
template <> struct num<0> { static constexpr int value = 0; };
}

#define UPER_GADGET \
    Q_GADGET \
    static constexpr detail::num<0> _uper_optional_counter(detail::num<0>) { return {}; }

#define UPER_ELEMENT(Type, Name) \
public: \
    Type Name = {}; \
    Q_PROPERTY(Type Name MEMBER Name CONSTANT)

#define UPER_ELEMENT_OPTIONAL(Type, Name) \
public: \
    Type Name = {}; \
    Q_PROPERTY(Type Name MEMBER Name CONSTANT) \
    Q_PROPERTY(bool Name ## IsSet READ Name ## IsSet) \
private: \
    static constexpr int _uper_ ## Name ## OptionalIndex = decltype(_uper_optional_counter(detail::num<>()))::value; \
    static constexpr auto _uper_optional_counter(detail::num<decltype(_uper_optional_counter(detail::num<>()))::value + 1> n) \
        -> decltype(n) { return {}; } \
public: \
    inline bool Name ## IsSet() const { return m_optionals[m_optionals.size() - _uper_ ## Name ## OptionalIndex - 1]; }

#define UPER_ELEMENT_DEFAULT(Type, Name, DefaultValue) \
public: \
    Type Name = DefaultValue; \
    Q_PROPERTY(Type Name MEMBER Name CONSTANT) \
private: \
    static constexpr int _uper_ ## Name ## OptionalIndex = decltype(_uper_optional_counter(detail::num<>()))::value; \
    static constexpr auto _uper_optional_counter(detail::num<decltype(_uper_optional_counter(detail::num<>()))::value + 1> n) \
        -> decltype(n) { return {}; } \
    inline bool Name ## IsSet() const { return m_optionals[m_optionals.size() - _uper_ ## Name ## OptionalIndex - 1]; }
}

#endif // KITINERARY_UPERELEMENT_H
