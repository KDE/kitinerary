/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_PLISTDATA_P_H
#define KITINERARY_PLISTDATA_P_H

#include <QtEndian>

#include <cstdint>

namespace KItinerary {

enum class PListObjectType {
    Null,
    Bool,
    Url,
    Uuid,
    Fill,
    Int,
    Real,
    Date,
    Data,
    String,
    Uid,
    Array,
    Ordset,
    Set,
    Dict,
    Unused
};

#pragma pack(push)
#pragma pack(1)

template <typename T>
struct PListBigEndianNumber {
public:
    operator T() const {
        if constexpr (std::is_same_v<T, uint64_t>)
            return qFromBigEndian((quint64)m_value);
        else
            return qFromBigEndian(m_value);
    }
private:
    T m_value;
};

constexpr inline const char PListMagic[] = "bplist";
constexpr inline std::size_t PListMagicSize = 6;

struct PListHeader {
    uint8_t magic[PListMagicSize];
    uint8_t version[2];
};

constexpr inline uint8_t PListContainerTypeMask = 0b1111'0000;
constexpr inline uint8_t PListContainerSizeMask = 0b0000'1111;

constexpr inline uint8_t PListFalse = 0b000'1000;
constexpr inline uint8_t PListTrue = 0b000'1001;

struct PListTrailer {
    PListBigEndianNumber<uint8_t> offsetIntSize;
    PListBigEndianNumber<uint8_t> objectRefSize;
    PListBigEndianNumber<uint64_t> numObjects;
    PListBigEndianNumber<uint64_t> rootObjectIndex;
    PListBigEndianNumber<uint64_t> offsetTableOffset;
};

#pragma pack(pop)

}

#endif

