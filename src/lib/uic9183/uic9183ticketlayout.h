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

/** Low-level field entries in a U_TLAY block.
 *  For most uses, the high-level API to retrieve assembled text (Uic9183TicketLayout::text())
 *  is probably preferable to this.
 */
class KITINERARY_EXPORT Uic9183TicketLayoutField
{
public:
    Uic9183TicketLayoutField();
    /** Create a new U_TLAY field starting at @p data.
     *  @param size The size of the remaining U_TLAY field array (not just this field!).
     */
    Uic9183TicketLayoutField(const char *data, int size);

    /** Returns @true if this is an empty field, e.g. when iterating beyond the last one. */
    bool isNull() const;
    /** Size of the entire field data (not the size of the text content!). */
    int size() const;

    int row() const;
    int column() const;
    int height() const;
    int width() const;
    int format() const;
    QString text() const;

    /** Returns the next field object, or a null one if there isn't one. */
    Uic9183TicketLayoutField next() const;

private:
    const char *m_data = nullptr;
    int m_size = 0;
};


/** Parser for a U_TLAY block in a UIC 918-3 ticket container, such as a ERA TLB ticket.
 *  @see ERA TAP TSI TD B.12 Digital Security Elements For Rail Passenger Ticketing - §10 TLB - Ticket Layout Barcode
 */
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

    /** Low-level field iteration access.
     *  Prefer text() over this to avoid doing your own text layout assembly.
     */
    Uic9183TicketLayoutField firstField() const;
private:
    QExplicitlySharedDataPointer<Uic9183TicketLayoutPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::Uic9183TicketLayout)

#endif // KITINERARY_UIC9183TICKETLAYOUT_H
