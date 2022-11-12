/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_UPERELEMENT_H
#define KITINERARY_UPERELEMENT_H

#include "uperdecoder.h"

namespace KItinerary {

// The property counter thing is based on the approach described here https://woboq.com/blog/verdigris-implementation-tricks.html
namespace uper_detail {
template <int N = 255> struct num : public num<N - 1> {
    static constexpr int value = N;
    static constexpr num<N - 1> prev() { return {}; }
};
template <> struct num<0> { static constexpr int value = 0; };
}

// start of an ASN.1 SEQUENCE definition
#define UPER_GADGET \
    Q_GADGET \
    static constexpr uper_detail::num<0> _uper_optional_counter(uper_detail::num<0>) { return {}; } \
    static constexpr bool _uper_ExtensionMarker = false;

// same as UPER_GADGET, for SEQUENCES with an extension marker
#define UPER_EXTENDABLE_GADGET \
    Q_GADGET \
    static constexpr uper_detail::num<0> _uper_optional_counter(uper_detail::num<0>) { return {}; } \
    static constexpr bool _uper_ExtensionMarker = true;

// ASN.1 ENUMERATED definitions, with or without extension marker
#define UPER_ENUM(Name) \
    Q_ENUM_NS(Name) \
    constexpr bool uperHasExtensionMarker(Name) { return false; }
#define UPER_EXTENABLE_ENUM(Name) \
    Q_ENUM_NS(Name) \
    constexpr bool uperHasExtensionMarker(Name) { return true; }

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
    static constexpr int _uper_ ## Name ## OptionalIndex = decltype(_uper_optional_counter(uper_detail::num<>()))::value; \
    static constexpr auto _uper_optional_counter(uper_detail::num<decltype(_uper_optional_counter(uper_detail::num<>()))::value + 1> n) \
        -> decltype(n) { return {}; } \
public: \
    inline bool Name ## IsSet() const { return m_optionals[m_optionals.size() - _uper_ ## Name ## OptionalIndex - 1]; }

#define UPER_ELEMENT_DEFAULT(Type, Name, DefaultValue) \
public: \
    Type Name = DefaultValue; \
    Q_PROPERTY(Type Name MEMBER Name CONSTANT) \
private: \
    static constexpr int _uper_ ## Name ## OptionalIndex = decltype(_uper_optional_counter(uper_detail::num<>()))::value; \
    static constexpr auto _uper_optional_counter(uper_detail::num<decltype(_uper_optional_counter(uper_detail::num<>()))::value + 1> n) \
        -> decltype(n) { return {}; } \
    inline bool Name ## IsSet() const { return m_optionals[m_optionals.size() - _uper_ ## Name ## OptionalIndex - 1]; }
}

#define UPER_GADGET_FINALIZE \
public: \
    void decode(UPERDecoder &decoder); \
private: \
    static constexpr auto _uper_OptionalCount = decltype(_uper_optional_counter(uper_detail::num<>()))::value; \
    std::bitset<_uper_OptionalCount> m_optionals; \
    inline void decodeSequence(UPERDecoder &decoder) { \
        if constexpr (_uper_ExtensionMarker) { \
            if (decoder.readBoolean()) { \
                decoder.setError("SEQUENCE with extension marker set not implemented."); \
                return; \
            } \
        } \
        m_optionals = decoder.readBitset<_uper_OptionalCount>(); \
    }

#endif // KITINERARY_UPERELEMENT_H
