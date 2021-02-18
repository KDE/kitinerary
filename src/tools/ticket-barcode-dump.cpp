/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../lib/era/ssbticket.h"

#include <kitinerary_version.h>

#include <KItinerary/IataBcbpParser>
#include <KItinerary/Uic9183Parser>
#include <KItinerary/VdvTicket>
#include <KItinerary/VdvTicketParser>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QMetaProperty>

#include <iostream>

#include <cstring>

using namespace KItinerary;

void dumpSsbTicket(const QByteArray &data)
{
    SSBTicket ticket(data);

    const auto typePrefix = QByteArray("type" + QByteArray::number(ticket.ticketTypeCode()));
    for (auto i = 0; i < SSBTicket::staticMetaObject.propertyCount(); ++i) {
        const auto prop = SSBTicket::staticMetaObject.property(i);
        if (std::strncmp(prop.name(), "type", 4) == 0 && std::strncmp(prop.name(), typePrefix.constData(), 5) != 0) {
            continue;
        }

        const auto value = prop.readOnGadget(&ticket);
        switch (value.type()) {
            case QVariant::Int:
                std::cout << prop.name() << ": " << value.toInt() << std::endl;
                break;
            default:
                std::cout << prop.name() << ": " << qPrintable(value.toString()) << std::endl;
                break;
        }
    }

    if (ticket.ticketTypeCode() == SSBTicket::IRT_RES_BOA) {
        std::cout << std::endl;
        std::cout << "Issuing day: " << qPrintable(ticket.issueDate().toString(Qt::ISODate)) << std::endl;
        std::cout << "Departure day: " << qPrintable(ticket.type1DepartureDay().toString(Qt::ISODate)) << std::endl;
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
    parser.addPositionalArgument(QStringLiteral("input"), QStringLiteral("File to read data from, omit for using stdin."));
    parser.process(app);

    // TODO stdin support
    if (parser.positionalArguments().isEmpty()) {
        parser.showHelp(1);
    }

    QFile file(parser.positionalArguments().at(0));
    if (!file.open(QFile::ReadOnly)) {
        std::cerr << qPrintable(file.errorString()) << std::endl;
        return 1;
    }

    const auto data = file.readAll();

    if (IataBcbpParser::maybeIataBcbp(QString::fromLatin1(data))) {
        std::cout << "IATA Barcoded Boarding Pass" << std::endl;
        // TODO
    } else if (SSBTicket::maybeSSB(data)) {
        std::cout << "ERA SSB Ticket" << std::endl;
        dumpSsbTicket(data);
    } else if (Uic9183Parser::maybeUic9183(data)) {
        std::cout << "UIC 918.3 Container" << std::endl;
        // TODO
    } else if (VdvTicketParser::maybeVdvTicket(data)) {
        std::cout << "VDV Ticket" << std::endl;
        // TODO
    } else {
        std::cout << "Unknown content" << std::endl;
        return 1;
    }

    return 0;
}
