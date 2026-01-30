/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include <qobjectdefs.h>
#include <QByteArray>

#include <cstdint>

class QDate;
class QDateTime;

namespace KItinerary {

/** Internal base class for ERA SSB tickets. */
class KITINERARY_EXPORT SSBTicketBase
{
    Q_GADGET

    Q_PROPERTY(QByteArray rawData READ rawData STORED false)
    Q_PROPERTY(QByteArray encodedData READ encodedData STORED false)

    /** Base 64 encoded payload, off-standard but used by Eurostar
     *  and necessary to not break their scanners.
     */
    Q_PROPERTY(bool isBase64 MEMBER m_isBase64 STORED false)

public:
    /** Raw SSB data, base64 decoding applied if necessary. */
    [[nodiscard]] QByteArray rawData() const;
    /** Barcode content data as it was scanned, possibly base64 encoded if that's how it was. */
    [[nodiscard]] QByteArray encodedData() const;

protected:
    SSBTicketBase();
    ~SSBTicketBase();

    // start and length in bits
    Q_INVOKABLE [[nodiscard]] quint64 readNumber(int start, int length) const;
    Q_INVOKABLE [[nodiscard]] QString readString(int start, int length) const;

    /** Convert a SSBv1 or v2 day number to a date based on @p context. */
    [[nodiscard]] static QDate dayNumberToDate(int days, const QDateTime &context);

    QByteArray m_data;
    bool m_isBase64 = false;
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

