/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_UPERELEMENT_H
#define KITINERARY_UPERELEMENT_H

namespace KItinerary {

#define UPER_GADGET \
    Q_GADGET

#define UPER_ELEMENT(Type, Name) \
public: \
    Type Name = {}; \
    Q_PROPERTY(Type Name MEMBER Name CONSTANT)

#define UPER_ELEMENT_OPTIONAL(Type, Name, OptionalIndex) \
public: \
    Type Name = {}; \
    inline bool Name ## IsSet() const { return m_optionals[m_optionals.size() - OptionalIndex - 1]; } \
    Q_PROPERTY(Type Name MEMBER Name CONSTANT) \
    Q_PROPERTY(bool Name ## IsSet READ Name ## IsSet)

#define UPER_ELEMENT_DEFAULT(Type, Name, DefaultValue, OptionalIndex) \
public: \
    Type Name = DefaultValue; \
    Q_PROPERTY(Type Name MEMBER Name CONSTANT) \
private: \
    inline bool Name ## IsSet() const { return m_optionals[m_optionals.size() - OptionalIndex - 1]; }
}

#endif // KITINERARY_UPERELEMENT_H
