/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_PROTOBUFSTREAMREADER_H
#define KITINERARY_PROTOBUFSTREAMREADER_H

#include <QByteArray>
#include <QMetaType>

#include <string_view>

class QString;

namespace KItinerary {

/** Protocol Buffers stream reader.
 *  For use on protobuf data for which the full format definition is unknown.
 *  @see https://protobuf.dev/
 */
class ProtobufStreamReader
{
    Q_GADGET
public:
    explicit ProtobufStreamReader();
    explicit ProtobufStreamReader(std::string_view data);
    explicit ProtobufStreamReader(const QByteArray &data);
    ~ProtobufStreamReader();

    /** Returns the number of the current field.
     *  Assumes the cursor is on the beginning of a field. The cursor does not advance.
     */
    Q_INVOKABLE quint64 fieldNumber();

    enum WireType {
        VARINT,
        I64,
        LEN,
        SGROUP,
        EGROUP,
        I32,
    };
    Q_ENUM(WireType)

    /** Returns the wire type of the current field.
     *  Assumes the cursor is on the beginning of a field. The cursor does not advance.
     */
    Q_INVOKABLE WireType wireType();

    /** Read a field of type VARINT.
     *  This assumes the cursor is placed at the beginning of a field with wire type VARINT.
     *  The cursor is advanced to after the field.
     */
    Q_INVOKABLE quint64 readVarintField();

    /** Reads a field of type LEN.
     *  This assumes the cursor is placed at the beginning of a field with wire type LEN.
     *  The cursor is advanced to after the field.
     */
    std::string_view readLengthDelimitedRecord();

    /** Reads a string.
     *  This assumes the cursor is placed at the beginning of a field with wire type LEN
     *  containing a string. The cursor is advanced to after the field.
     */
    Q_INVOKABLE QString readString();

    /** Reads a nested message.
     *  This assumes the cursor is placed at the beginning of a field with wire type LEN
     *  containing a sub-message. The cursor is advanced to after the field.
     */
    Q_INVOKABLE KItinerary::ProtobufStreamReader readSubMessage();

    /** Returns @c true when having reached the end of the stream. */
    Q_INVOKABLE bool atEnd() const;

    /** Skips over the next field in the stream. */
    Q_INVOKABLE void skip();

    ///@cond internal
    /** Read Base 128 varint value from the current stream position and advances the cursor. */
    uint64_t readVarint();
    /** Read Base 128 varint value from the current stream position without advancing the cursor. */
    uint64_t peekVarint();
    ///@endcond

private:
    QByteArray m_ownedData;
    std::string_view m_data;
    std::string_view::size_type m_cursor = 0;
};

}

Q_DECLARE_METATYPE(KItinerary::ProtobufStreamReader)

#endif // KITINERARY_PROTOBUFSTREAMREADER_H
