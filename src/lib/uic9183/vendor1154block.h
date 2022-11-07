/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_VENDOR1154BLOCK_H
#define KITINERARY_VENDOR1154BLOCK_H

#include "kitinerary_export.h"
#include "uic9183block.h"

namespace KItinerary {

/** UIC 918.3 1154UT vendor data block sub-block. */
class KITINERARY_EXPORT Vendor1154UTSubBlock
{
    Q_GADGET
    Q_PROPERTY(QString content READ toString)
public:
    Vendor1154UTSubBlock();
    Vendor1154UTSubBlock(const Uic9183Block &block, int offset);
    bool isNull() const;

    /** Type of the block. */
    const char *id() const;
    /** Size of the entire S-block. */
    int size() const;
    /** Next sub-block in the 1154UT block. */
    Vendor1154UTSubBlock nextBlock() const;

    /** Size of the content the sub-block. */
    int contentSize() const;
    /** Content data of the sub-block. */
    const char *content() const;

    /** Content as Unicode string. */
    QString toString() const;

private:
    Uic9183Block m_block;
    int m_offset = 0;
};

/** UIC 918.3 1154UT vendor data block. */
class KITINERARY_EXPORT Vendor1154UTBlock
{
    Q_GADGET
public:
    Vendor1154UTBlock(const Uic9183Block &block = Uic9183Block());

    bool isValid() const;
    /** First S-block, for iterating. */
    Vendor1154UTSubBlock firstBlock() const;
    /** Finds a S-block by type. */
    Vendor1154UTSubBlock findSubBlock(const char id[3]) const;
    Q_INVOKABLE QVariant findSubBlock(const QString &str) const;

    static constexpr const char RecordId[] = "1154UT";
private:
    Uic9183Block m_block;
};

}

Q_DECLARE_METATYPE(KItinerary::Vendor1154UTBlock)
Q_DECLARE_METATYPE(KItinerary::Vendor1154UTSubBlock)

#endif // KITINERARY_VENDOR1154BLOCK_H
