/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_PROTOBUFSTREAMREADER_H
#define KITINERARY_PROTOBUFSTREAMREADER_H

#include <string_view>

class QString;

namespace KItinerary {

/** Protocol Buffers stream reader.
 *  For use on protobuf data for which the full format definition is unknown.
 *  @see https://protobuf.dev/
 */
class ProtobufStreamReader
{
public:
    explicit ProtobufStreamReader(std::string_view data);
    ~ProtobufStreamReader();

    /** Read Base 128 varint value from the current stream position and advances the cursor. */
    uint64_t readVarint();
    /** Read Base 128 varint value from the current stream position without advancing the cursor. */
    uint64_t peekVarint();

    /** Returns the number of the current field.
     *  Assumes the cursor is on the beginning of a field. The cursor does not advance.
     */
    uint64_t fieldNumber();

    enum WireType {
        VARINT,
        I64,
        LEN,
        SGROUP,
        EGROUP,
        I32,
    };

    /** Returns the wire type of the current field.
     *  Assumes the cursor is on the beginning of a field. The cursor does not advance.
     */
    WireType wireType();

    /** Reads a field of type LEN.
     *  This assumes the cursor is placed at the beginning of a field with wire type LEN.
     *  The cursor is advanced to after the field.
     */
    std::string_view readLengthDelimitedRecord();

    /** Reads a string.
     *  This assumes the cursor is placed at the beginning of a field with wire type LEN
     *  containing a string. The cursor is advanced to after the field.
     */
    QString readString();

    /** Reads a nested message.
     *  This assumes the cursor is placed at the beginning of a field with wire type LEN
     *  containing a sub-message. The cursor is advanced to after the field.
     */
    ProtobufStreamReader readSubMessage();

private:
    std::string_view m_data;
    std::string_view::size_type m_cursor = 0;
};

}

#endif // KITINERARY_PROTOBUFSTREAMREADER_H
