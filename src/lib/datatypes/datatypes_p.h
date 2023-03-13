/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "datatypes_impl.h"

///@cond internal
#define KITINERARY_PRIVATE_BASE_GADGET(Class) \
public: \
virtual ~ Class ## Private() = default; \
virtual Class ## Private * clone() const { \
    return new Class ##Private(*this); \
} \
typedef Class ## Private base_type; \
typedef Class ## Private this_type; \
private: \

#define KITINERARY_PRIVATE_GADGET(Class) \
public: \
inline base_type* clone() const override { \
    return new Class ## Private(*this); \
} \
typedef this_type super_type; \
typedef Class ## Private this_type; \
private:

///@endcond
