/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include <QByteArray>
#include <QMetaType>
#include <QString>

namespace KItinerary {

/** A data block from a UIC 918.3 ticket. */
class KITINERARY_EXPORT Uic9183Block
{
    Q_GADGET
    /** Content as string, for use in JS. */
    Q_PROPERTY(QString contentText READ contentText)
public:
    Uic9183Block();
    explicit Uic9183Block(const QByteArray &data, int offset);
    Uic9183Block(const Uic9183Block&);
    Uic9183Block(Uic9183Block&&);
    Uic9183Block& operator=(const Uic9183Block&);
    Uic9183Block& operator=(Uic9183Block&&);

    /** Returns the block name (6 characters).
     *  The name is either "U_" + 4 letter standard type or a 4 digit vendor id + 2 char vendor type
     */
    const char *name() const;

    /** Checks if this block has the given record id. */
    bool isA(const char recordId[6]) const;
    /** Checks if a block is of type @tparam T. */
    template <typename T> inline bool isA() const
    {
        return isA(T::RecordId);
    }

    /** Returns the payload data (not including the block header). */
    const char *content() const;
    /** Returns the size of the entire block data. */
    int size() const;
    /** Returns the size of the content data. */
    int contentSize() const;
    /** Returns the version number of this block. */
    int version() const;

    /** Checks if the block is valid or empty/default constructed. */
    bool isNull() const;

    /** Returns the next block in the ticket.
     *  If there is no more block, a null block is returned.
     */
    Uic9183Block nextBlock() const;

private:
    QString contentText() const;

    QByteArray m_data;
    int m_offset = 0;
};

}

Q_DECLARE_METATYPE(KItinerary::Uic9183Block)

