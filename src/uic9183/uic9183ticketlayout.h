/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_UIC9183TICKETLAYOUT_H
#define KITINERARY_UIC9183TICKETLAYOUT_H

#include "kitinerary_export.h"

#include <QExplicitlySharedDataPointer>
#include <QMetaType>

class QSize;

namespace KItinerary {

class Uic9183Block;
class Uic9183TicketLayoutPrivate;

/** Parser for a U_TLAY block in a UIC 918-3 ticket. */
class KITINERARY_EXPORT Uic9183TicketLayout
{
    Q_GADGET
    /** Ticket type (e.g. RCT2). */
    Q_PROPERTY(QString type READ type)

public:
    Uic9183TicketLayout();
    /** Parse U_TLAY ticket layout block in @p data.
     *  It's the callers responsibility to ensure @p data outlives this instance, the data
     *  is not copied.
     *  @param block A UIC 918.3 U_TLAY data block
     */
    Uic9183TicketLayout(const Uic9183Block &block);
    Uic9183TicketLayout(const Uic9183TicketLayout&);
    ~Uic9183TicketLayout();
    Uic9183TicketLayout& operator=(const Uic9183TicketLayout&);

    /** Returns whether this is a valid U_TLAY layout block. */
    bool isValid() const;

    /** Type of the ticket layout. */
    QString type() const;

    /** Returns the text in the given coordinates.
     *  @note @p row and @p column are 0-based unlike the U_TLAY spec, which is 1-based!
     */
    Q_INVOKABLE QString text(int row, int column, int width, int height) const;

    /** The size of the layout, as width and height in layout coordinates. */
    QSize size() const;

private:
    QExplicitlySharedDataPointer<Uic9183TicketLayoutPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::Uic9183TicketLayout)

#endif // KITINERARY_UIC9183TICKETLAYOUT_H
