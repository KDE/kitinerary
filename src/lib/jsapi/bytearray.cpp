/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "bytearray.h"
#include "bitarray.h"
#include "logging.h"

#include "protobuf/protobufstreamreader.h"
#include "rsp/rsp6decoder.h"

#include <QQmlEngine>

#include <zlib.h>

using namespace KItinerary;

QJSValue JsApi::ByteArray::inflate(const QByteArray &input) const
{
    QByteArray output;
    output.resize(4096);
    z_stream stream;
    stream.zalloc = nullptr;
    stream.zfree = nullptr;
    stream.opaque = nullptr;
    stream.avail_in = input.size();
    stream.next_in = reinterpret_cast<unsigned char*>(const_cast<char*>(input.data()));
    stream.avail_out = output.size();
    stream.next_out = reinterpret_cast<unsigned char*>(output.data());

    inflateInit2(&stream, MAX_WBITS + 32);
    const auto res = ::inflate(&stream, Z_NO_FLUSH);
    switch (res) {
        case Z_OK:
        case Z_STREAM_END:
            break; // all good
        default:
            qCWarning(Log) << "zlib decompression failed " << stream.msg << stream.avail_in;
            return {};
    }
    inflateEnd(&stream);
    output.truncate(output.size() - stream.avail_out);
    return toArrayBuffer(output);
}

QString JsApi::ByteArray::toBase64(const QByteArray &input) const
{
    return QString::fromUtf8(input.toBase64());
}

QJSValue JsApi::ByteArray::fromBase64(const QString &b64) const
{
    return toArrayBuffer(QByteArray::fromBase64(b64.toUtf8()));
}

QString JsApi::ByteArray::decodeUtf8(const QByteArray &input) const
{
    // explicitly truncate at the first null byte, Qt6 doesn't do that automatically anymore
    const auto idx = input.indexOf('\0');
    return QString::fromUtf8(input.constData(), idx >= 0 ? idx : input.size());
}

QString JsApi::ByteArray::decodeLatin1(const QByteArray &input) const
{
    // explicitly truncate at the first null byte, Qt6 doesn't do that automatically anymore
    const auto idx = input.indexOf('\0');
    return QString::fromLatin1(input.constData(), idx >= 0 ? idx : input.size());
}

QVariant JsApi::ByteArray::toBitArray(const QByteArray &input) const
{
    return QVariant::fromValue(JsApi::BitArray(input));
}

QVariant JsApi::ByteArray::toProtobufStreamReader(const QByteArray &input) const
{
    return QVariant::fromValue(ProtobufStreamReader(input));
}

QJSValue JsApi::ByteArray::decodeRsp6Ticket(const QString &text) const
{
    return toArrayBuffer(Rsp6Decoder::decode(text.toLatin1()));
}

QJSValue JsApi::ByteArray::toArrayBuffer(const QByteArray &input) const
{
    const auto engine = qjsEngine(this);
    return QJSValue(QJSManagedValue(QVariant(input), engine));
}
