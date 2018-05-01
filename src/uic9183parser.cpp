/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "config-kitinerary.h"
#include "uic9183parser.h"
#include "logging.h"

#include <KItinerary/Person>

#include <QDebug>

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

#include <cassert>
#include <cstring>

using namespace KItinerary;

namespace KItinerary {

class Uic9183Block
{
public:
    Uic9183Block() = default;
    Uic9183Block(const char *data, int size);

    const char *data() const { return m_data; }
    int version() const;
    int size() const { return m_size; }
    bool isNull() const { return m_size <= 12; }

private:
    const char *m_data = nullptr;
    int m_size = 0;
};

class Uic9183ParserPrivate : public QSharedData
{
public:
    // name is either "U_" + 4 letter type or a 4 digit vendor id + 2 char type
    Uic9183Block findBlock(const char name[6]) const;

    QByteArray m_payload;
};
}

Uic9183Block::Uic9183Block(const char* data, int size)
    : m_data(data)
    , m_size(size)
{
}

int Uic9183Block::version() const
{
    return QByteArray(m_data + 6, 2).toInt();
}


Uic9183Block Uic9183ParserPrivate::findBlock(const char name[6]) const
{
    // 6x header name
    // 2x block version
    // 4x block size as string, including the header

    for (int i = 0; i < m_payload.size() - 12;) {
        const int blockSize = m_payload.mid(i + 8, 4).toInt();
        if (blockSize + i >= m_payload.size()) {
            qCWarning(Log) << "UIC 918-3 block size exceeds payload size.";
            return {};
        }
        if (strncmp(name, m_payload.data() + i, 6) == 0) {
            return Uic9183Block(m_payload.data() + i, blockSize);
        }
        i += blockSize;
    }
    return {};
}


Uic9183Parser::Uic9183Parser()
    : d(new Uic9183ParserPrivate)
{
}

Uic9183Parser::Uic9183Parser(const Uic9183Parser&) = default;
Uic9183Parser::~Uic9183Parser() = default;
Uic9183Parser& Uic9183Parser::operator=(Uic9183Parser&) = default;

void Uic9183Parser::parse(const QByteArray &data)
{
    d->m_payload.clear();

    // header and signature block (64 byte total)
    if (data.size() < 64) {
        qCWarning(Log) << "UIC 918-3 ticket too short.";
        return;
    }

    // 3x header
    if (!data.startsWith("#UT") && !data.startsWith("OTI")) {
        qCWarning(Log) << "Data does not contain a UIC 918-3 ticket.";
        return;
    }
    // 2x version
    if (data.at(3) != '0' || data.at(4) != '1') {
        qCWarning(Log) << "Unsupported UIC918-3 ticket version.";
        return;
    }
    // 4x issuer UIC carrier code
    // 5x signature key id
    // 50x ASN.1 signature

    // zlib compressed payload
    if (data.size() < 64 + 8) {
        qCWarning(Log) << "UIC 918-3 payload too short.";
        return;
    }
    // 4x compressed payload size as string
    // 2x zlib header 0x789C
    if (data[68] != 0x78 || (uchar)data[69] != 0x9C) {
        qCWarning(Log) << "UIC 918-3 payload has wrong zlib header.";
        return;
    }
    // nx zlib payload
#ifdef HAVE_ZLIB
    d->m_payload.resize(4096);
    z_stream stream;
    stream.zalloc = nullptr;
    stream.zfree = nullptr;
    stream.opaque = nullptr;
    stream.avail_in = data.size() - 68;
    stream.next_in = reinterpret_cast<unsigned char*>(const_cast<char*>(data.data() + 68));
    stream.avail_out = d->m_payload.size();
    stream.next_out = reinterpret_cast<unsigned char*>(d->m_payload.data());

    inflateInit(&stream);
    const auto res = inflate(&stream, Z_NO_FLUSH);
    switch (res) {
        case Z_OK:
        case Z_STREAM_END:
            break; // all good
        default:
            qCWarning(Log) << "UIC 918.3 payload zlib decompression failed" << stream.msg;
            return;
    }
    inflateEnd(&stream);
    d->m_payload.truncate(d->m_payload.size() - stream.avail_out);
    qDebug() << res <<  d->m_payload << stream.avail_out;
#endif
}

bool Uic9183Parser::isValid() const
{
    return !d->m_payload.isEmpty();
}

// U_HEAD (version 1, size 53)
// 4x issuing carrier id
// 6x PNR
// 20x stuff
// 12x issuing date/time as ddMMyyyyHHMM
// 1x flags
// 2x ticket language
// 2x secondary ticket language

QString Uic9183Parser::pnr() const
{
    const auto b = d->findBlock("U_HEAD");
    qDebug() << b.isNull() << b.version() << b.size();
    if (b.isNull() || b.version() != 1 || b.size() != 53) {
        return {};
    }
    return QString::fromUtf8(b.data() + 16, 6);
}

// 0080BL vendor block (DB) (version 2, dynamic size)

Person Uic9183Parser::person() const
{
    // TODO
    return {};
}

#include "moc_uic9183parser.cpp"
