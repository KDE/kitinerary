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

#include "uic9183parser.h"
#include "logging.h"

#include <QDateTime>
#include <QDebug>

#include <zlib.h>

#include <cassert>
#include <cstring>

using namespace KItinerary;

static int asciiToInt(const char *s, int size)
{
    if (!s) {
        return 0;
    }

    int v = 0;
    for (int i = 0; i < size; ++i) {
        v *= 10;
        v += (*(s + i)) - '0';
    }
    return v;
}

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
    // name is either "U_" + 4 letter type or a 4 digit vendor id + 2 char type
    Uic9183Block findBlock(const char name[6]) const;

    QByteArray m_payload;
};

// 2x field line, number as ascii text
// 2x field column
// 2x field height
// 2x field width
// 1x field format
// 4x text length
// Nx text content
class Rct2TicketField
{
public:
    Rct2TicketField() = default;
    Rct2TicketField(const char *data);
    // size of the field data, not size of the text content
    int size() const;

    int row() const;
    int column() const;
    int height() const;
    int width() const;
    QString text() const;

private:
    const char *m_data = nullptr;
};

class Rct2TicketPrivate : public QSharedData
{
public:
    QString fieldText(int row, int column, int width, int height = 1) const;
    QDate firstDayOfValidity() const;
    QDateTime parseTime(const QString &dateStr, const QString &timeStr) const;

    Uic9183Block block;
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
    const auto certCount = *(block.data() + 14) - '0';
    const auto certSize = block.version() == 2 ? 46 : 26;
    return 15 + certSize * certCount + 2;
}


Uic9183Block Uic9183ParserPrivate::findBlock(const char name[6]) const
{
    // 6x header name
    // 2x block version
    // 4x block size as string, including the header

    for (int i = 0; i < m_payload.size() - 12;) {
        const int blockSize = m_payload.mid(i + 8, 4).toInt();
        if (blockSize + i > m_payload.size()) {
            qCWarning(Log) << "UIC 918-3 block size exceeds payload size.";
            return {};
        }
        if (strncmp(name, m_payload.data() + i, 6) == 0) {
            return {m_payload.data() + i, blockSize};
        }
        i += blockSize;
    }
    return {};
}


Rct2TicketField::Rct2TicketField(const char *data)
    : m_data(data)
{
}

int Rct2TicketField::size() const
{
    return asciiToInt(m_data + 9, 4) + 13;
}

int Rct2TicketField::row() const
{
    return asciiToInt(m_data, 2);
}

int Rct2TicketField::column() const
{
    return asciiToInt(m_data + 2, 2);
}

int Rct2TicketField::height() const
{
    return asciiToInt(m_data + 4, 2);
}

int Rct2TicketField::width() const
{
    return asciiToInt(m_data + 6, 2);
}

QString Rct2TicketField::text() const
{
    return QString::fromUtf8(m_data + 13, asciiToInt(m_data + 9, 4));
}

QString Rct2TicketPrivate::fieldText(int row, int column, int width, int height) const
{
    QString s;
    for (int i = 20; i < block.size();) {
        Rct2TicketField f(block.data() + i);
        i += f.size();

        if (f.row() + f.height() - 1 < row || f.row() > row + height - 1) {
            continue;
        }
        if (f.column() + f.width() - 1 < column || f.column() > column + width - 1) {
            continue;
        }
        //qDebug() << "Field:" << f.height() << f.column() << f.height() << f.width() << f.size() << f.text();

        // split field into lines
        // TODO this needs to follow the RCT2 word-wrapping algorithm?
        const auto content = f.text();
        const auto lines = content.splitRef(QLatin1Char('\n'));

        // cut out the right part of the line
        for (int i = 0; i < lines.size(); ++i) {
            if (f.row() + i < row) {
                continue;
            }
            if (f.row() + i > row + height - 1) {
                break;
            }

            // TODO also truncate by w
            const auto offset = column - f.column();
            if (offset >= 0) {
                s += lines.at(i).mid(offset).left(width);
            } else {
                s += lines.at(i); // TODO left padding by offset, truncate by width + offset
            }
        }
    }
    //qDebug() << "Result:" << x << y << w << h << s;
    return s;
}

QDate Rct2TicketPrivate::firstDayOfValidity() const
{
    const auto f = fieldText(3, 1, 48);
    const auto it = std::find_if(f.begin(), f.end(), [](QChar c) { return c.isDigit(); });
    if (it == f.end()) {
        return {};
    }

    const auto dtStr = f.midRef(std::distance(f.begin(), it));
    auto dt = QDate::fromString(dtStr.left(10).toString(), QStringLiteral("dd.MM.yyyy"));
    if (dt.isValid()) {
        return dt;
    }
    dt = QDate::fromString(dtStr.left(8).toString(), QStringLiteral("dd.MM.yy"));
    if (dt.isValid()) {
        if (dt.year() < 2000) {
            dt.setDate(dt.year() + 100, dt.month(), dt.day());
        }
        return dt;
    }
    dt = QDate::fromString(dtStr.left(4).toString(), QStringLiteral("yyyy"));
    return dt;
}

QDateTime Rct2TicketPrivate::parseTime(const QString &dateStr, const QString &timeStr) const
{
    const auto d = QDate::fromString(dateStr, QStringLiteral("dd.MM"));
    const auto t = QTime::fromString(timeStr, QStringLiteral("hh:mm"));
    return QDateTime({firstDayOfValidity().year(), d.month(), d.day()}, t);
}


// 6x "U_TLAY"
// 2x version (always "01")
// 4x record length, numbers as ASCII text
// 4x ticket layout type ("RCT2")
// 4x field count
// Nx fields (see Rct2TicketField)
Rct2Ticket::Rct2Ticket()
    : d(new Rct2TicketPrivate)
{
}

Rct2Ticket::Rct2Ticket(Uic9183Block block)
    : d(new Rct2TicketPrivate)
{
    d->block = block;
    qDebug() << QByteArray(block.data(), block.size());
}

Rct2Ticket::Rct2Ticket(const Rct2Ticket&) = default;
Rct2Ticket::~Rct2Ticket() = default;
Rct2Ticket& Rct2Ticket::operator=(const Rct2Ticket&) = default;

bool Rct2Ticket::isValid() const
{
    return !d->block.isNull() && d->block.size() > 34
        && std::strncmp(d->block.data() + 6, "01", 2) == 0
        && std::strncmp(d->block.data() + 12, "RCT2", 4) == 0;
}

QDate Rct2Ticket::firstDayOfValidity() const
{
    return d->firstDayOfValidity();
}

QDateTime Rct2Ticket::outboundDepartureTime() const
{
    return d->parseTime(d->fieldText(6, 1, 5), d->fieldText(6, 7, 5));
}

QDateTime Rct2Ticket::outboundArrivalTime() const
{
    return d->parseTime(d->fieldText(6, 52, 5), d->fieldText(6, 58, 5));
}

QString Rct2Ticket::outboundDepartureStation() const
{
    return d->fieldText(6, 13, 17).trimmed();
}

QString Rct2Ticket::outboundArrivalStation() const
{
    return d->fieldText(6, 34, 17).trimmed();
}

QString Rct2Ticket::outboundClass() const
{
    return d->fieldText(6, 66, 5).trimmed();
}


Uic9183Parser::Uic9183Parser()
    : d(new Uic9183ParserPrivate)
{
}

Uic9183Parser::Uic9183Parser(const Uic9183Parser&) = default;
Uic9183Parser::~Uic9183Parser() = default;
Uic9183Parser& Uic9183Parser::operator=(const Uic9183Parser&) = default;

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

// U_HEAD (version 1, size 53)
// 4x issuing carrier id
// 6x PNR
// 20x unique ticket key
// 12x issuing date/time as ddMMyyyyHHMM, as UTC
// 1x flags
// 2x ticket language
// 2x secondary ticket language

QString Uic9183Parser::pnr() const
{
    const auto b = d->findBlock("U_HEAD");
    if (b.isNull() || b.version() != 1 || b.size() != 53) {
        return {};
    }
    return QString::fromUtf8(b.data() + 16, 6);
}

Person Uic9183Parser::person() const
{
    // Deutsche Bahn vendor block
    const auto b = Vendor0080BLBlock(d->findBlock("0080BL"));
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
    return {};
}

QString Uic9183Parser::outboundDepartureStationId() const
{
    const auto b = Vendor0080BLBlock(d->findBlock("0080BL"));
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
    const auto b = Vendor0080BLBlock(d->findBlock("0080BL"));
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

Rct2Ticket Uic9183Parser::rct2Ticket() const
{
    return Rct2Ticket(d->findBlock("U_TLAY"));
}

QVariant Uic9183Parser::rct2TicketVariant() const
{
    const auto rct2 = rct2Ticket();
    if (rct2.isValid()) {
        return QVariant::fromValue(rct2);
    }
    return {};
}


#include "moc_uic9183parser.cpp"
