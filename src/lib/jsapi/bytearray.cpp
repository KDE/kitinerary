/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "bytearray.h"
#include "logging.h"

#include <QQmlEngine>

#include <private/qv4arraybuffer_p.h>
#include <private/qv4engine_p.h>

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

QString JsApi::ByteArray::decodeUtf8(const QByteArray &input) const
{
    return QString::fromUtf8(input);
}

QJSValue JsApi::ByteArray::toArrayBuffer(const QByteArray &input) const
{
    const auto engine = qjsEngine(this);
    return QJSValue(engine->handle(), engine->handle()->newArrayBuffer(input)->asReturnedValue());
}
