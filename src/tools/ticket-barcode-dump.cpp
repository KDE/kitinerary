/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../lib/asn1/berelement.h"
#include "../lib/era/dosipas1.h"
#include "../lib/era/dosipas2.h"
#include "../lib/era/elbticket.h"
#include "../lib/era/fcbticket.h"
#include "../lib/era/ssbv1ticket.h"
#include "../lib/era/ssbv2ticket.h"
#include "../lib/era/ssbv3ticket.h"
#include "../lib/iata/iatabcbp.h"
#include "../lib/uic9183/uic9183flex.h"
#include "../lib/uic9183/uic9183head.h"
#include "../lib/uic9183/uic9183header.h"
#include "../lib/uic9183/vendor0080vublockdata.h"
#include "../lib/vdv/vdvticketcontent.h"

#include <kitinerary_version.h>

#include <KItinerary/Uic9183Block>
#include <KItinerary/Uic9183Parser>
#include <KItinerary/Uic9183TicketLayout>
#include <KItinerary/Vendor0080Block>
#include <KItinerary/VdvTicket>
#include <KItinerary/VdvTicketParser>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QMetaProperty>
#include <QSequentialIterable>

#include <iostream>

#include <cstring>

using namespace KItinerary;

static void dumpRawData(const char *data, std::size_t size)
{
    bool isText = true;
    for (std::size_t i = 0; i < size && isText; ++i) {
        isText = (uint8_t)*(data + i) >= 20;
    }

    if (isText) {
        std::cout.write(data, size);
    } else {
        std::cout << "(hex) " << QByteArray(data, size).toHex().constData();
    }
}

template <typename T>
static void dumpGadget(const T *gadget, const char* indent = "")
{
    dumpGadget(gadget, &T::staticMetaObject, indent);
}

static void dumpGadget(const void *gadget, const QMetaObject *mo, const char* indent)
{
    if (!gadget || !mo) {
        return;
    }
    for (auto i = 0; i < mo->propertyCount(); ++i) {
        const auto prop = mo->property(i);
        if (!prop.isStored()) {
            continue;
        }
        const auto value = prop.readOnGadget(gadget);
        if (prop.isEnumType()) {
            std::cout << indent << prop.name() << ": " << prop.enumerator().valueToKey(value.toInt()) << std::endl;
        } else if (prop.typeId() == QMetaType::QByteArray) {
            std::cout << indent << prop.name() << ": ";
            const auto data = value.toByteArray();
            dumpRawData(data.constData(), data.size());
            std::cout << std::endl;
        } else {
            std::cout << indent << prop.name() << ": " << qPrintable(value.toString()) << std::endl;
        }
        if (const auto childMo = QMetaType(value.userType()).metaObject()) {
            QByteArray childIndent(indent);
            childIndent.push_back(' ');
            dumpGadget(value.constData(), childMo, childIndent.constData());
        } else if (value.canConvert<QVariantList>() && value.userType() != QMetaType::QString && value.userType() != QMetaType::QByteArray) {
            auto iterable = value.value<QSequentialIterable>();
            int idx = 0;
            QByteArray childIndent(indent);
            childIndent.append("  ");
            for (const QVariant &v : iterable) {
                if (QMetaType(v.userType()).metaObject()) {
                    std::cout << indent << " [" << idx++ << "]:" << std::endl;
                    dumpGadget(v.constData(), QMetaType(v.userType()).metaObject(), childIndent.constData());
                } else {
                    std::cout << indent << " [" << idx++ << "]: "  << qPrintable(v.toString()) << std::endl;
                }
            }
        }
    }
}

static void dumpSsbv3Ticket(const QByteArray &data)
{
    SSBv3Ticket ticket(data);

    const auto typePrefix = QByteArray("type" + QByteArray::number(ticket.ticketTypeCode()));
    for (auto i = 0; i < SSBv3Ticket::staticMetaObject.propertyCount(); ++i) {
        const auto prop = SSBv3Ticket::staticMetaObject.property(i);
        if (!prop.isStored()) {
            continue;
        }
        if (std::strncmp(prop.name(), "type", 4) == 0 && std::strncmp(prop.name(), typePrefix.constData(), 5) != 0) {
            continue;
        }

        const auto value = prop.readOnGadget(&ticket);
        std::cout << prop.name() << ": " << qPrintable(value.toString()) << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Issuing day: " << qPrintable(ticket.issueDate().toString(Qt::ISODate)) << std::endl;
    switch (ticket.ticketTypeCode()) {
        case SSBv3Ticket::IRT_RES_BOA:
            std::cout << "Departure day: " << qPrintable(ticket.type1DepartureDay().toString(Qt::ISODate)) << std::endl;
            break;
        case SSBv3Ticket::NRT:
            std::cout << "Valid from: " << qPrintable(ticket.type2ValidFrom().toString(Qt::ISODate)) << std::endl;
            std::cout << "Valid until: " << qPrintable(ticket.type2ValidUntil().toString(Qt::ISODate)) << std::endl;
            break;
        case SSBv3Ticket::GRT:
        case SSBv3Ticket::RPT:
            break;
    }
}

static void dumpSsbv2Ticket(const QByteArray &data)
{
    SSBv2Ticket ticket(data);
    dumpGadget(&ticket);
}

static void dumpSsbv1Ticket(const QByteArray &data)
{
    SSBv1Ticket ticket(data);
    dumpGadget(&ticket);
    std::cout << std::endl;
    std::cout << "First day of validitiy: " << qPrintable(ticket.firstDayOfValidity().toString(Qt::ISODate)) << std::endl;
    std::cout << "Departure time: " << qPrintable(ticket.departureTime().toString(Qt::ISODate)) << std::endl;
}

static void dumpElbTicketSegment(const ELBTicketSegment &segment)
{
    dumpGadget(&segment, "  ");
    std::cout << "  Departure date: "  << qPrintable(segment.departureDate().toString(Qt::ISODate)) << std::endl;
}

static void dumpElbTicket(const ELBTicket &ticket)
{
    const auto mo = &ELBTicket::staticMetaObject;
    for (auto i = 0; i < mo->propertyCount(); ++i) {
        const auto prop = mo->property(i);
        if (!prop.isStored() || QMetaType(prop.userType()).metaObject()) {
            continue;
        }
        const auto value = prop.readOnGadget(&ticket);
        std::cout << prop.name() << ": " << qPrintable(value.toString()) << std::endl;
    }
    std::cout << "Emission date: " << qPrintable(ticket.emissionDate().toString(Qt::ISODate)) << std::endl;
    std::cout << "Valid from: " << qPrintable(ticket.validFromDate().toString(Qt::ISODate)) << std::endl;
    std::cout << "Valid until: " << qPrintable(ticket.validUntilDate().toString(Qt::ISODate)) << std::endl;
    std::cout << std::endl << "Segment 1:" << std::endl;
    dumpElbTicketSegment(ticket.segment1());
    if (ticket.segment2().isValid()) {
        std::cout << std::endl << "Segment 2:" << std::endl;
        dumpElbTicketSegment(ticket.segment2());
    }
}

static void dumpUic9183(const QByteArray &data)
{
    Uic9183Parser parser;
    parser.parse(data);
    std::cout << " Header:" << std::endl;
    const auto header = parser.header();
    dumpGadget(&header, "  ");

    for (auto block = parser.firstBlock(); !block.isNull(); block = block.nextBlock()) {
        std::cout << " Block: ";
        std::cout.write(block.name(), 6);
        std::cout << ", size: " << block.size()  << ", version: " << block.version() << std::endl;

        if (block.isA<Uic9183Head>()) {
            Uic9183Head head(block);
            dumpGadget(&head, "  ");
        } else if (block.isA<Uic9183TicketLayout>()) {
            Uic9183TicketLayout tlay(block);
            std::cout << "   Layout standard: " << qPrintable(tlay.type()) << std::endl;
            for (auto field = tlay.firstField(); !field.isNull(); field = field.next()) {
                std::cout << "    [row: " << field.row() << " column: " << field.column()
                          << " height: " << field.height() << " width: " << field.width()
                          << " format: " << field.format() << "]: " << qPrintable(field.text())
                          << std::endl;
            }
        } else if (block.isA<Uic9183Flex>()) {
            Uic9183Flex flex(block);
            const auto fcbProp = Uic9183Flex::staticMetaObject.property(Uic9183Flex::staticMetaObject.indexOfProperty("fcb"));
            QVariant fcbData = fcbProp.readOnGadget(&flex);
            dumpGadget(fcbData.constData(), QMetaType(fcbData.typeId()).metaObject(), "  ");
        } else if (block.isA<Vendor0080BLBlock>()) {
            Vendor0080BLBlock vendor(block);
            dumpGadget(&vendor, "  ");
            for (int i = 0; i < vendor.orderBlockCount(); ++i) {
                const auto order = vendor.orderBlock(i);
                std::cout << "  Order block " << (i + 1) << ":" << std::endl;
                dumpGadget(&order, "   ");
            }
            std::cout << "  S-blocks:" << std::endl;
            for (auto sub = vendor.firstBlock(); !sub.isNull(); sub = sub.nextBlock()) {
                std::cout << "   ";
                std::cout.write(sub.id(), 3);
                std::cout << " (size: " << sub.size() << "): ";
                dumpRawData(sub.content(), sub.contentSize());
                std::cout << std::endl;
            }
        } else if (block.isA<Vendor0080VUBlock>()) {
            Vendor0080VUBlock vendor(block);
            dumpGadget(vendor.commonData(), "  ");
            for (int i = 0; i < (int)vendor.commonData()->numberOfTickets; ++i) {
                const auto ticket = vendor.ticketData(i);
                std::cout << "  Ticket " << (i + 1) << ":" << std::endl;
                dumpGadget(ticket, "   ");
                dumpGadget(&ticket->validityArea, "   ");
                std::cout << "    payload: (hex) " << QByteArray((const char*)&ticket->validityArea + sizeof(VdvTicketValidityAreaData), ticket->validityAreaDataSize - sizeof(VdvTicketValidityAreaData)).toHex().constData() << std::endl;
            }
        } else {
            std::cout << "  Content: ";
            dumpRawData(block.content(), block.contentSize());
            std::cout << std::endl;
        }
    }
}

template <typename T>
static void dumpDosipas(const T &ticket)
{
    dumpGadget(&ticket, "  ");
}

static void dumpVdv(const QByteArray &data)
{
    VdvTicketParser parser;
    if (!parser.parse(data)) {
        std::cerr << "failed to parse VDV ticket" << std::endl;
        return;
    }
    const auto ticket = parser.ticket();
    std::cout << " Header:" << std::endl;
    dumpGadget(ticket.header(), "  ");

    std::cout << " Product data:" << std::endl;
    for (auto block = ticket.productData().first(); block.isValid(); block = block.next()) {
        std::cout << "  Tag: 0x" << std::hex << block.type() << std::dec << " size: " << block.size() << std::endl;
        switch (block.type()) {
            case VdvTicketBasicData::Tag:
                dumpGadget(block.contentAt<VdvTicketBasicData>(), "    ");
                break;
            case VdvTicketTravelerData::Tag:
            {
                const auto traveler = block.contentAt<VdvTicketTravelerData>();
                dumpGadget(traveler, "    ");
                std::cout << "    name: " << qPrintable(QString::fromUtf8(traveler->name(), traveler->nameSize(block.contentSize()))) << std::endl;
                break;
            }
            case VdvTicketValidityAreaData::Tag:
            {
                const auto area = block.contentAt<VdvTicketValidityAreaData>();

                switch (area->type) {
                    case VdvTicketValidityAreaDataType31::Type:
                    {
                        const auto area31 = static_cast<const VdvTicketValidityAreaDataType31*>(area);
                        dumpGadget(area31, "    ");
                        std::cout << "    payload: (hex) " << QByteArray((const char*)block.contentData() + sizeof(VdvTicketValidityAreaDataType31), block.contentSize() - sizeof(VdvTicketValidityAreaDataType31)).toHex().constData() << std::endl;
                        break;
                    }
                    default:
                        dumpGadget(area, "    ");
                        std::cout << "    payload: (hex) " << QByteArray((const char*)block.contentData() + sizeof(VdvTicketValidityAreaData), block.contentSize() - sizeof(VdvTicketValidityAreaData)).toHex().constData() << std::endl;
                        break;
                }
                break;
            }
            default:
                std::cout << "   (hex) " << QByteArray((const char*)block.contentData(), block.contentSize()).toHex().constData() << std::endl;
        }
    }

    std::cout << " Transaction data:" << std::endl;
    dumpGadget(ticket.commonTransactionData(), "  ");
    std::cout << " Product-specific transaction data (" << ticket.productSpecificTransactionData().contentSize() << " bytes):" << std::endl;
    for (auto block = ticket.productSpecificTransactionData().first(); block.isValid(); block = block.next()) {
        std::cout << "  Tag: " << block.type() << " size: " << block.size() << std::endl;
        switch (block.type()) {
            default:
                std::cout << "   (hex) " << QByteArray((const char*)block.contentData(), block.contentSize()).toHex().constData() << std::endl;
        }
    }

    std::cout << " Issue data:" << std::endl;
    dumpGadget(ticket.issueData(), "  ");
    std::cout << " Trailer:" << std::endl;
    std::cout << "  identifier: ";
    std::cout.write(ticket.trailer()->identifier, 3);
    std::cout << std::endl;
    std::cout << "  version: " << ticket.trailer()->version << std::endl;
}

void dumpIataBcbp(const QString &data, const QDateTime &contextDate)
{
    IataBcbp ticket(data);
    if (!ticket.isValid()) {
        std::cout << "invalid format" << std::endl;
        return;
    }

    const auto ums = ticket.uniqueMandatorySection();
    dumpGadget(&ums);
    const auto ucs = ticket.uniqueConditionalSection();
    dumpGadget(&ucs);
    const auto issueDate = ucs.dateOfIssue(contextDate);
    std::cout << "Date of issue: " << qPrintable(issueDate.toString(Qt::ISODate)) << std::endl;

    for (auto i = 0; i < ums.numberOfLegs(); ++i) {
        std::cout << "Leg " << (i + 1) << std::endl;
        const auto rms = ticket.repeatedMandatorySection(i);
        dumpGadget(&rms, "  ");
        const auto rcs = ticket.repeatedConditionalSection(i);
        dumpGadget(&rcs, "  ");
        std::cout << "  Airline use section: " << qPrintable(ticket.airlineUseSection(i)) << std::endl;
        std::cout << "  Date of flight: " << qPrintable(rms.dateOfFlight(issueDate.isValid() ? QDateTime(issueDate, {}) : contextDate).toString(Qt::ISODate)) << std::endl;
    }

    if (ticket.hasSecuritySection()) {
        std::cout << "Security:" << std::endl;
        const auto sec = ticket.securitySection();
        dumpGadget(&sec, "  ");
    }
}

int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("ticket-barcode-dump"));
    QCoreApplication::setApplicationVersion(QStringLiteral(KITINERARY_VERSION_STRING));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Decode ticket barcode content."));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption contextDateOpt(QStringLiteral("context-date"), QStringLiteral("Context to resolve incomplete dates."), QStringLiteral("yyyy-MM-dd"));
    parser.addOption(contextDateOpt);
    parser.addPositionalArgument(QStringLiteral("input"), QStringLiteral("File to read data from, omit for using stdin."));
    parser.process(app);

    auto contextDate = QDate::currentDate();
    if (parser.isSet(contextDateOpt)) {
        contextDate = QDate::fromString(parser.value(contextDateOpt), Qt::ISODate);
    }

    QFile file;
    if (parser.positionalArguments().isEmpty()) {
        file.open(stdin, QFile::ReadOnly);
    } else {
        file.setFileName(parser.positionalArguments().at(0));
        if (!file.open(QFile::ReadOnly)) {
            std::cerr << qPrintable(file.errorString()) << std::endl;
            return 1;
        }
    }

    const auto data = file.readAll();

    if (IataBcbp::maybeIataBcbp(data)) {
        std::cout << "IATA Barcoded Boarding Pass" << std::endl;
        dumpIataBcbp(QString::fromUtf8(data), QDateTime(contextDate, {}));
    } else if (SSBv3Ticket::maybeSSB(data)) {
        std::cout << "ERA SSB Ticket" << std::endl;
        dumpSsbv3Ticket(data);
    } else if (SSBv2Ticket::maybeSSB(data)) {
        std::cout << "ERA SSB Ticket" << std::endl;
        dumpSsbv2Ticket(data);
    } else if (SSBv1Ticket::maybeSSB(data)) {
        std::cout << "ERA SSB Ticket" << std::endl;
        dumpSsbv1Ticket(data);
    } else if (Uic9183Parser::maybeUic9183(data)) {
        std::cout << "UIC 918.3 Container" << std::endl;
        dumpUic9183(data);
    } else if (VdvTicketParser::maybeVdvTicket(data)) {
        std::cout << "VDV Ticket" << std::endl;
        dumpVdv(data);
    } else if (auto ticket = ELBTicket::parse(data); ticket) {
        std::cout << "ERA ELB Ticket" << std::endl;
        dumpElbTicket(*ticket);
    } else if (auto ticket = Dosipas::v2::UicBarcodeHeader(data); ticket.isValid()) {
        std::cout << "DOSIPAS v2 Container" << std::endl;
        dumpDosipas(ticket);
    } else if (auto ticket = Dosipas::v1::UicBarcodeHeader(data); ticket.isValid()) {
        std::cout << "DOSIPAS v1 Container" << std::endl;
        dumpDosipas(ticket);
    } else {
        std::cout << "Unknown content" << std::endl;
        return 1;
    }

    return 0;
}
