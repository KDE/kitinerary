/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_VENDOR0080BLOCK_H
#define KITINERARY_VENDOR0080BLOCK_H

#include "kitinerary_export.h"
#include "uic9183block.h"

class QString;

namespace KItinerary {

/** UIC 918.3 0080BL vendor data block sub-block. */
class KITINERARY_EXPORT Vendor0080BLSubBlock
{
public:
    Vendor0080BLSubBlock();
    Vendor0080BLSubBlock(const Uic9183Block &block, int offset);

    bool isNull() const;

    /** Type of the S-block. */
    const char *id() const;
    /** Size of the entire S-block. */
    int size() const;
    /** Next S-block in the 0080BL block. */
    Vendor0080BLSubBlock nextBlock() const;

    /** Size of the content of the S-block. */
    int contentSize() const;
    /** Content data of the S-block. */
    const char *content() const;

    /** Content as Unicode string. */
    QString toString() const;

private:
    Uic9183Block m_block;
    int m_offset = 0;
};


/** UIC 918.3 0080BL vendor data block. */
class KITINERARY_EXPORT Vendor0080BLBlock
{
public:
    Vendor0080BLBlock(const Uic9183Block &block);

    bool isValid() const;
    /** First S-block, for iterating. */
    Vendor0080BLSubBlock firstBlock() const;
    /** Finds a S-block by type. */
    Vendor0080BLSubBlock findSubBlock(const char id[3]) const;

    static constexpr const char RecordId[] = "0080BL";
private:
    static int subblockOffset(const Uic9183Block &block);

    Uic9183Block m_block;
};

class Vendor0080VUCommonData;
class Vendor0080VUTicketData;

/** UIC 918.3 0080VU vendor data block (DB local public transport extensions).
 *  @see UIC918.3* Interoperabilität Barcode DB Online-Ticker VDV-KA
 */
class KITINERARY_EXPORT Vendor0080VUBlock
{
public:
    Vendor0080VUBlock(const Uic9183Block &block);
    bool isValid() const;

    const Vendor0080VUCommonData* commonData() const;
    const Vendor0080VUTicketData* ticketData(int index) const;

    static constexpr const char RecordId[] = "0080VU";
private:
    Uic9183Block m_block;
};

}

#endif // KITINERARY_VENDOR0080BLOCK_H
