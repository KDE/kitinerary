/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include <qobjectdefs.h>
#include <QByteArray>

#include <cstdint>

namespace KItinerary {

/** Internal base class for ERA SSB tickets. */
class KITINERARY_EXPORT SSBTicketBase
{
    Q_GADGET
protected:
    SSBTicketBase();
    ~SSBTicketBase();

    // start and length in bits
    Q_INVOKABLE quint64 readNumber(int start, int length) const;
    Q_INVOKABLE QString readString(int start, int length) const;

    QByteArray m_data;
};

#define SSB_NUM_PROPERTY(Name, Start, Len) \
public: \
    inline int Name() const { return readNumber(Start, Len); } \
    Q_PROPERTY(int Name READ Name)
#define SSB_LONG_PROPERTY(Name, Start, Len) \
public: \
    inline quint64 Name() const { return readNumber(Start, Len); } \
    Q_PROPERTY(quint64 Name READ Name)
#define SSB_STR_PROPERTY(Name, Start, Len) \
public: \
    inline QString Name() const { return readString(Start, Len); } \
    Q_PROPERTY(QString Name READ Name)

}

