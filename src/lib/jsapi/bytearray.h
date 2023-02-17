/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_JSAPI_BYTEARRAY_H
#define KITINERARY_JSAPI_BYTEARRAY_H

#include <QObject>
#include <QJSValue>
#include <QVariant>

class QByteArray;

namespace KItinerary {
namespace JsApi {

/** API for dealing with QByteArray and/or JS ArrayBuffer objects. */
class ByteArray : public QObject
{
    Q_OBJECT
public:
    /** Perform zlib decompression on the given byte array.
     *  @returns a JS ArrayBuffer with the decompressed result.
     */
    Q_INVOKABLE QJSValue inflate(const QByteArray &input) const;

    /** Converts the given QByteArray or JS ArrayBuffer into an base64 encoded string. */
    Q_INVOKABLE QString toBase64(const QByteArray &input) const;
    /** Converts a given Base64 encoded string to a JS ArrayBuffer. */
    Q_INVOKABLE QJSValue fromBase64(const QString &b64) const;

    /** Convert a QByteArray/ArrayBuffer to a string, assuming UTF-8 encoding. */
    Q_INVOKABLE QString decodeUtf8(const QByteArray &input) const;
    /** Convert a QByteArray/ArrayBuffer to a string, assuming Latin1 encoding. */
    Q_INVOKABLE QString decodeLatin1(const QByteArray &input) const;

    /** Converts the given QByteArray or JS ArrayBuffer into a BitArray. */
    Q_INVOKABLE QVariant toBitArray(const QByteArray &input) const;

    /** Creates a Protocol Buffers stream reader for the given JS ArrayBuffer. */
    Q_INVOKABLE QVariant toProtobufStreamReader(const QByteArray &input) const;

    /** Decode/decrypt a UK RSP-6 ticket barcode. */
    Q_INVOKABLE QJSValue decodeRsp6Ticket(const QString &text) const;

    /** Convert a QByteArray to a JS ArrayBuffer.
     *  This is mainly a migration aid until we return ArrayBuffers everywhere.
     */
    Q_INVOKABLE QJSValue toArrayBuffer(const QByteArray &input) const;
};

}}

#endif // KITINERARY_JSAPI_BYTEARRAY_H
