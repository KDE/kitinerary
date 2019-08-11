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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "uic9183parser.h"
#include "logging.h"
#include "rct2ticket.h"
#include "uic9183block.h"
#include "uic9183ticketlayout.h"

#include <QDateTime>
#include <QDebug>

#include <zlib.h>

#include <cassert>
#include <cstring>

using namespace KItinerary;

namespace KItinerary {

// 0080BL vendor block sub-block ("S block")
// 1x 'S'
// 3x field type
// 4x field value length
// nx field value
class Vendor0080BLSubBlock
{
public:
    Vendor0080BLSubBlock() = default;
    Vendor0080BLSubBlock(const char *data, int size);

    bool isNull() const { return m_size <= 0 || !m_data; }
    int size() const { return m_size; }
    const char *id() const { return m_data + 1; }
    const char *data() const { return m_data + 8; }

    QString toString() const { return QString::fromUtf8(data(), size()); }

private:
    const char *m_data = nullptr;
    int m_size = 0;
};

// 0080BL vendor block (DB) (version 2/3, dynamic size)
// 2x stuff
// 1x number of certificate blocks
// 22+8+8+8x (v2) or 8+8+10x (v3) certificate block
// 2x number of sub blocks
class Vendor0080BLBlock
{
public:
    Vendor0080BLBlock(const Uic9183Block &block);

    bool isValid() const;
    Vendor0080BLSubBlock findSubBlock(const char id[3]) const;

private:
    static int subblockOffset(const Uic9183Block &block);

    Uic9183Block m_block;
};

class Uic9183ParserPrivate : public QSharedData
{
public:
    QByteArray m_payload;
    QDateTime m_contextDt;
};
}

Vendor0080BLSubBlock::Vendor0080BLSubBlock(const char *data, int size)
    : m_data(data)
    , m_size(size)
{
}

Vendor0080BLBlock::Vendor0080BLBlock(const Uic9183Block &block)
{
    if (block.isNull()) {
        return;
    }
    if (block.version() != 2 && block.version() != 3) {
        qCWarning(Log) << "Unsupported version of 0080BL vendor block." << block.version();
        return;
    }
    if (block.isNull() || block.size() < 15 || subblockOffset(block) > block.size()) {
        return;
    }
    m_block = block;
}

bool Vendor0080BLBlock::isValid() const
{
    return !m_block.isNull();
}

Vendor0080BLSubBlock Vendor0080BLBlock::findSubBlock(const char id[3]) const
{
    for (int i = subblockOffset(m_block); i < m_block.size();) {
        if (*(m_block.data() + i) != 'S') {
            qCWarning(Log) << "0080BL invalid S-block format.";
            return {};
        }
        const int subblockSize = QByteArray(m_block.data() + i + 4, 4).toInt();
        if (subblockSize + i > m_block.size()) {
            qCWarning(Log) << "0080BL S-block size exceeds block size.";
            return {};
        }
        Vendor0080BLSubBlock sb(m_block.data() + i, subblockSize);
        if (!sb.isNull() && strncmp(sb.id(), id, 3) == 0) {
            return sb;
        }
        i += subblockSize + 8;
    }
    return {};
}

int Vendor0080BLBlock::subblockOffset(const Uic9183Block& block)
{
    const auto certCount = *(block.content() + 2) - '0';
    const auto certSize = block.version() == 2 ? 46 : 26;
    return 15 + certSize * certCount + 2;
}


Uic9183Block Uic9183Parser::firstBlock() const
{
    return Uic9183Block(d->m_payload, 0);
}

Uic9183Block Uic9183Parser::findBlock(const char name[6]) const
{
    auto block = firstBlock();
    while (!block.isNull()) {
        if (strncmp(name, block.name(), 6) == 0) {
            return block;
        }
        block = block.nextBlock();
    }
    return {};
}


Uic9183Parser::Uic9183Parser()
    : d(new Uic9183ParserPrivate)
{
}

Uic9183Parser::Uic9183Parser(const Uic9183Parser&) = default;
Uic9183Parser::~Uic9183Parser() = default;
Uic9183Parser& Uic9183Parser::operator=(const Uic9183Parser&) = default;

void Uic9183Parser::setContextDate(const QDateTime &contextDt)
{
    d->m_contextDt = contextDt;
}

void Uic9183Parser::parse(const QByteArray &data)
{
    d->m_payload.clear();

    // header and signature block (64 byte total)
    if (!Uic9183Parser::maybeUic9183(data)) {
        qCWarning(Log) << "UIC 918-3 ticket too short or has wrong header/version.";
        return;
    }

    // 3x header
    // 2x version
    // 4x UIC code of the signing carrier
    // 5x signature key id
    // 50x ASN.1 signature

    // zlib compressed payload
    if (data.size() < 64 + 8) {
        qCWarning(Log) << "UIC 918-3 payload too short.";
        return;
    }
    // 4x compressed payload size as string
    // 2x zlib header 0x789C
    if (data[68] != 0x78 || ((uchar)data[69] != 0x9C && (uchar)data[69] != 0xDA)) {
        qCWarning(Log) << "UIC 918-3 payload has wrong zlib header.";
        return;
    }
    // nx zlib payload
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
    //qCDebug(Log) << res <<  d->m_payload << stream.avail_out;
}

bool Uic9183Parser::isValid() const
{
    return !d->m_payload.isEmpty();
}

// U_HEAD Block (version 1, size 53)
// 4x issuing carrier id
// 6x PNR
// 20x unique ticket key
// 12x issuing date/time as ddMMyyyyHHMM, as UTC
// 1x flags
// 2x ticket language
// 2x secondary ticket language

QString Uic9183Parser::pnr() const
{
    const auto b = findBlock("U_HEAD");
    if (b.isNull() || b.version() != 1 || b.size() != 53) {
        return {};
    }
    return QString::fromUtf8(b.content() + 4, 6);
}

QString Uic9183Parser::carrierId() const
{
    const auto b = findBlock("U_HEAD");
    if (b.isNull() || b.version() != 1 || b.size() != 53) {
        return {};
    }
    return QString::fromUtf8(b.content(), 4);
}

Person Uic9183Parser::person() const
{
    // Deutsche Bahn vendor block
    const auto b = Vendor0080BLBlock(findBlock("0080BL"));
    if (b.isValid()) {
        // S028 contains family and given name separated by a '#', UTF-8 encoded
        auto sblock = b.findSubBlock("028");
        if (!sblock.isNull()) {
            const auto endIt = sblock.data() + sblock.size();
            auto it = std::find(sblock.data(), endIt, '#');
            if (it != endIt) {
                Person p;
                p.setGivenName(QString::fromUtf8(sblock.data(), std::distance(sblock.data(), it)));
                ++it;
                p.setFamilyName(QString::fromUtf8(it, std::distance(it, endIt)));
                return p;
            }
        }
        // S023 contains the full name, UTF-8 encoded
        sblock = b.findSubBlock("023");
        if (!sblock.isNull()) {
            Person p;
            p.setName(sblock.toString());
            return p;
        }
    }

    // RCT2 tickets
    const auto rct2 = rct2Ticket();
    if (rct2.isValid()) {
        const auto name = rct2.passengerName();
        if (!name.isEmpty()) {
            Person p;
            p.setName(name);
            return p;
        }
    }

    return {};
}

QString Uic9183Parser::outboundDepartureStationId() const
{
    const auto b = Vendor0080BLBlock(findBlock("0080BL"));
    if (b.isValid()) {
        // S035 contains the IBNR, possible with leading '80' country code and leading 0 stripped
        const auto sblock = b.findSubBlock("035");
        if (!sblock.isNull() && sblock.size() <= 7) {
            QString ibnr = QStringLiteral("ibnr:8000000");
            const auto s = sblock.toString();
            return ibnr.replace(ibnr.size() - s.size(), s.size(), s);
        }
    }
    return {};
}

QString Uic9183Parser::outboundArrivalStationId() const
{
    const auto b = Vendor0080BLBlock(findBlock("0080BL"));
    if (b.isValid()) {
        // S036 contains the IBNR, possible with leading '80' country code and leading 0 stripped
        const auto sblock = b.findSubBlock("036");
        if (!sblock.isNull() && sblock.size() <= 7) {
            QString ibnr = QStringLiteral("ibnr:8000000");
            const auto s = sblock.toString();
            return ibnr.replace(ibnr.size() - s.size(), s.size(), s);
        }
    }
    return {};
}

QString Uic9183Parser::seatingType() const
{
    const auto b = Vendor0080BLBlock(findBlock("0080BL"));
    if (b.isValid()) {
        // S014 contains the class, possibly with a leading 'S' for some reason
        const auto sblock = b.findSubBlock("014");
        if (!sblock.isNull()) {
            const auto s = sblock.toString();
            return s.startsWith(QLatin1Char('S')) ? s.right(1) : s;
        }
    }

    const auto rct2 = rct2Ticket();
    if (rct2.isValid()) {
        return rct2.outboundClass();
    }
    return {};
}

Uic9183TicketLayout Uic9183Parser::ticketLayout() const
{
    const auto block = findBlock("U_TLAY");
    Uic9183TicketLayout layout(block.data(), block.size());
    return layout;
}

QVariant Uic9183Parser::ticketLayoutVariant() const
{
    const auto layout = ticketLayout();
    return layout.isValid() ? QVariant::fromValue(layout) : QVariant();
}

Rct2Ticket Uic9183Parser::rct2Ticket() const
{
    Rct2Ticket rct2(ticketLayout());
    rct2.setContextDate(d->m_contextDt);
    return rct2;
}

QVariant Uic9183Parser::rct2TicketVariant() const
{
    const auto rct2 = rct2Ticket();
    if (rct2.isValid()) {
        return QVariant::fromValue(rct2);
    }
    return {};
}

bool Uic9183Parser::maybeUic9183(const QByteArray& data)
{
    if (data.size() < 64) {
        return false;
    }

    if (!data.startsWith("#UT") && !data.startsWith("OTI")) {
        return false;
    }

    if (data.at(3) != '0' || data.at(4) != '1') {
        return false;
    }

    return true;
}

#include "moc_uic9183parser.cpp"
