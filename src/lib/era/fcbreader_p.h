/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_FCBREADER_P_H
#define KITINERARY_FCBREADER_P_H

// common macros for reading FCB data

#define FCB_READ_CONSTRAINED_INT(Name, Min, Max) \
    if (Name ## IsSet()) \
        Name = decoder.readConstrainedWholeNumber(Min, Max)

#define FCB_READ_UNCONSTRAINED_INT(Name) \
    if (Name ## IsSet()) \
        Name = decoder.readUnconstrainedWholeNumber()

#define FCB_READ_IA5STRING(Name) \
    if (Name ## IsSet()) \
        Name = decoder.readIA5String()

#define FCB_READ_IA5STRING_CONSTRAINED(Name, Min, Max) \
    if (Name ## IsSet()) \
        Name = decoder.readIA5String(Min, Max)

#define FCB_READ_UTF8STRING(Name) \
    if (Name ## IsSet()) \
        Name = decoder.readUtf8String()

#define FCB_READ_OCTETSTRING(Name) \
    if (Name ## IsSet()) \
        Name = decoder.readOctetString()

#define FCB_READ_ENUM(Name) \
    do { if (Name ## IsSet()) { \
        if constexpr (uperHasExtensionMarker(decltype(Name){})) { \
            Name = decoder.readEnumeratedWithExtensionMarker<decltype(Name)>(); \
        } else { \
            Name = decoder.readEnumerated<decltype(Name)>(); \
        } \
    } } while(false)

#define FCB_READ_CUSTOM(Name) \
    if (Name ## IsSet()) \
        Name.decode(decoder);

#define FCB_READ_SEQUENCE_OF_CONTRAINED_INT(Name, Min, Max) \
    if (Name ## IsSet()) \
        Name = decoder.readSequenceOfConstrainedWholeNumber(Min, Max)

#define FCB_READ_SEQUENCE_OF_UNCONTRAINED_INT(Name) \
    if (Name ## IsSet()) \
        Name = decoder.readSequenceOfUnconstrainedWholeNumber()

#define FCB_READ_SEQUENCE_OF_IA5STRING(Name) \
    if (Name ## IsSet()) \
        Name = decoder.readSequenceOfIA5String()

#define FCB_READ_SEQUENCE_OF_UTF8STRING(Name) \
    if (Name ## IsSet()) \
        Name = decoder.readSequenceOfUtf8String()

#define FCB_READ_SEQUENCE_OF_CUSTOM(Name) \
    if (Name ## IsSet()) \
        Name = decoder.readSequenceOf<decltype(Name)::value_type>();

#define FCB_READ_OBJECT_IDENTIFIER(Name) \
    if (Name ## IsSet()) \
        Name = decoder.readObjectIdentifier();

#define FCB_READ_INT_IA5_PAIR(Name, Min, Max) \
    FCB_READ_CONSTRAINED_INT(Name ## Num, Min, Max); \
    FCB_READ_IA5STRING(Name ## IA5)

#define FCB_READ_INT_IA5_PAIR_UNCONSTRAINED(Name) \
    FCB_READ_UNCONSTRAINED_INT(Name ## Num); \
    FCB_READ_IA5STRING(Name ## IA5)

#define FCB_READ_TIME(Name) \
    FCB_READ_CONSTRAINED_INT(Name, 0, FCB_TIME_MAX)

#endif
